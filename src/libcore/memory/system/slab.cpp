// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//      F O U N D A T I O N   P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <algorithm>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    struct slab_t final {
        u0*             page;
        u0*             free_list;
        slab_t*         prev;
        slab_t*         next;
        u32             buf_count;
    };
}

namespace basecode::memory::slab {
    constexpr auto slab_size = sizeof(slab_t) + alignof(slab_t);

    static u0 remove(alloc_t* alloc, slab_t* slab) {
        auto sc                                      = &alloc->subclass.slab;
        if (slab->next)             slab->next->prev = slab->prev;
        if (slab->prev)             slab->prev->next = slab->next;
        if (sc->head == slab)       sc->head         = slab->next;
        if (sc->tail == slab)       sc->tail         = slab->prev;
    }

    static u0 move_back(alloc_t* alloc, slab_t* slab) {
        auto sc = &alloc->subclass.slab;
        if (sc->tail == slab) return;
        remove(alloc, slab);
        slab->next     = {};
        slab->prev     = sc->tail;
        sc->tail->next = slab;
        sc->tail       = slab;
    }

    static slab_t* move_front(alloc_t* alloc, slab_t* slab) {
        auto sc  = &alloc->subclass.slab;
        if (sc->head == slab) return slab;
        remove(alloc, slab);
        slab->prev = {};
        if (sc->head) {
            sc->head->prev = slab;
            slab->next     = sc->head;
            if (!sc->tail)
                sc->tail   = sc->head;
        } else {
            sc->tail = slab;
        }
        sc->head = slab;
        return sc->head;
    }

    static slab_t* grow(alloc_t* alloc, u32& alloc_size) {
        auto sc   = &alloc->subclass.slab;
        auto mem  = (u8*) memory::alloc(alloc->backing, sc->page_size, 8, &alloc_size);
        sc->count++;

        u32  align_adjust{};
        auto slab = (slab_t*) system::align_forward(mem + sc->page_size - slab_size, alignof(slab_t), align_adjust);
        slab->buf_count = {};
        slab->page      = mem;
        slab->free_list = mem;
        slab->next      = slab->prev = {};

        auto eff_size = sc->buf_align * ((sc->buf_size - 1) / sc->buf_align + 1);
        u8* last_buf = mem + (eff_size * (sc->buf_max_count - 1));
        for (auto p  = mem; p < last_buf; p += eff_size)
            *((u0**) p) = p + eff_size;

        return move_front(alloc, slab);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto sc  = &alloc->subclass.slab;
        auto cfg = (slab_config_t*) config;
        alloc->backing = cfg->backing;
        sc->count      = {};
        sc->head       = sc->tail = {};
        sc->num_pages  = std::max<u8>(cfg->num_pages, 1);
        sc->buf_align  = std::max<u8>(cfg->buf_align, alignof
            (u0*));
        sc->buf_size   = std::max<u32>(cfg->buf_size, sizeof(u0*));
        const auto page_overhead = sc->buf_align < 8 ? 8 :
                                   sc->buf_align + sizeof(alloc_header_t);
        sc->page_size     = (system::os_page_size() - page_overhead) *
                            sc->num_pages;
        sc->buf_max_count = (sc->page_size - slab_size) /
                            sc->buf_size;
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto sc = &alloc->subclass.slab;

        auto curr = sc->head;
        while (curr) {
            if (mem >= curr->page
            &&  mem <= ((u8*) curr->page + sc->page_size - slab_size)) {
                break;
            }
            curr = curr->next;
        }

        if (!curr || !curr->page)
            return;

        u32 align_adjust{};
        auto slab = (slab_t*) system::align_forward((u8*) curr->page + sc->page_size - slab_size, alignof(slab_t), align_adjust);
        *((u0**) mem) = slab->free_list;
        slab->free_list = mem;
        --slab->buf_count;

        if (slab->buf_count == 0) {
            remove(alloc, slab);
            memory::free(alloc->backing, slab->page, &freed_size);
            alloc->total_allocated -= freed_size;
        } else {
            freed_size = {};
        }

        if (slab->buf_count == sc->buf_max_count - 1)
            move_front(alloc, slab);
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        auto sc   = &alloc->subclass.slab;
        auto curr = sc->head;
        u32 temp_size{}, total_freed{};
        while (curr) {
            memory::free(alloc->backing, curr->page, &temp_size);
            total_freed += temp_size;
            curr = curr->next;
        }
        alloc->total_allocated -= total_freed;
        if (freed_size) *freed_size = total_freed;
        if (enforce) assert(alloc->total_allocated == 0);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        UNUSED(size);
        UNUSED(align);
        auto sc        = &alloc->subclass.slab;
        auto head_slab = sc->head;
        if (!head_slab || head_slab->buf_count == sc->buf_max_count) {
            head_slab = grow(alloc, alloc_size);
        } else {
            alloc_size = {};
        }
        u0* buf = head_slab->free_list;
        head_slab->free_list = *((u0**) buf);
        ++head_slab->buf_count;
        if (head_slab->buf_count == sc->buf_max_count)
            move_back(alloc, head_slab);
        alloc->total_allocated += alloc_size;
        return buf;
    }

    alloc_system_t g_alloc_system{
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = {},
        .type       = alloc_type_t::slab,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::slab);
        auto sc   = &a->subclass.slab;
        auto curr = sc->head;
        while (curr) {
            auto mem        = (u8*) curr->page;
            curr->buf_count = {};
            curr->free_list = mem;
            u32 eff_size    = sc->buf_align * ((sc->buf_size - 1) / sc->buf_align + 1);
            u8* last_buf    = mem + (eff_size * (sc->buf_max_count - 1));
            for (auto p  = mem; p < last_buf; p += eff_size)
                *((u0**) p) = p + eff_size;
            curr = curr->next;
        }
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }
}
