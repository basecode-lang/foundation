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

#include "slab_system.h"

namespace basecode::memory::slab {
    struct slab_t final {
        u0*             page;
        u0*             free_list;
        slab_t*         prev;
        slab_t*         next;
        u32             buf_count;
    };

    constexpr auto slab_size = sizeof(slab_t) + alignof(slab_t);

    static u0 remove(alloc_t* alloc, slab_t* slab) {
        auto subclass = &alloc->subclass.slab;
        if (slab->next)             slab->next->prev = slab->prev;
        if (slab->prev)             slab->prev->next = slab->next;
        if (subclass->head == slab) subclass->head = slab->next;
        if (subclass->tail == slab) subclass->tail = slab->prev;
    }

    static u0 move_back(alloc_t* alloc, slab_t* slab) {
        auto subclass = &alloc->subclass.slab;
        if (subclass->tail == slab) return;
        remove(alloc, slab);
        slab->next = {};
        slab->prev = (slab_t*) subclass->tail;
        ((slab_t*) subclass->tail)->next = slab;
        subclass->tail = slab;
    }

    static u0 move_front(alloc_t* alloc, slab_t* slab) {
        auto subclass = &alloc->subclass.slab;
        if (subclass->head == slab) return;
        remove(alloc, slab);
        slab->prev = {};
        if (subclass->head) {
            ((slab_t*) subclass->head)->prev = slab;
            slab->next = (slab_t*) subclass->head;
            if (!subclass->tail)
                subclass->tail = subclass->head;
        } else {
            subclass->tail = slab;
        }
        subclass->head = slab;
    }

    static u0 grow(alloc_t* alloc) {
        auto subclass = &alloc->subclass.slab;
        const auto page_size = system::os_page_size() - subclass->buf_align - sizeof(alloc_header_t);

        u32 alloc_size{};
        auto mem = (u8*) memory::alloc(alloc->backing, page_size, 8, &alloc_size);
        alloc->total_allocated += alloc_size;
        subclass->count++;

        u32 align_adjust{};
        auto slab = (slab_t*) system::align_forward(mem + page_size - slab_size, alignof(slab_t), align_adjust);
        slab->page = mem;
        slab->buf_count = {};
        slab->free_list = mem;
        slab->next = slab->prev = {};

        auto effective_size = subclass->buf_align * ((subclass->buf_size - 1) / subclass->buf_align + 1);
        u8* last_buffer = mem + (effective_size * (subclass->buf_max_count - 1));

        u32 i{};
        for (auto p = mem; p < last_buffer; p += effective_size) {
            *((u0**) p) = p + effective_size;
            ++i;
        }

        move_front(alloc, slab);
    }

    static u0 release(alloc_t* alloc) {
        auto subclass = &alloc->subclass.slab;
        slab_t* curr = (slab_t*) subclass->head;
        u32 freed_size{};
        while (curr) {
            memory::free(alloc->backing, curr->page, &freed_size);
            alloc->total_allocated -= freed_size;
            curr = curr->next;
        }
        assert(alloc->total_allocated == 0);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto subclass = &alloc->subclass.slab;
        auto slab_config = (slab_config_t*) config;
        alloc->backing = slab_config->backing;
        subclass->buf_size = slab_config->buf_size;
        subclass->buf_align = slab_config->buf_align;
        subclass->buf_max_count = (system::os_page_size() - slab_size - slab_config->buf_align) / slab_config->buf_size;
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto subclass = &alloc->subclass.slab;
        const auto page_size = system::os_page_size() - subclass->buf_align - sizeof(alloc_header_t);

        auto curr = (slab_t*) subclass->head;
        while (curr) {
            if (mem >= curr->page
            &&  mem <= ((u8*) curr->page + page_size - slab_size)) {
                break;
            }
            curr = curr->next;
        }

        // XXX: handle not finding the buffer

        u32 align_adjust{};
        auto slab = (slab_t*) system::align_forward((u8*) curr->page + page_size - slab_size, alignof(slab_t), align_adjust);
        *((u0**) mem) = slab->free_list;
        slab->free_list = mem;
        --slab->buf_count;

        if (slab->buf_count == 0) {
            remove(alloc, slab);
            memory::free(alloc->backing, slab->page, &freed_size);
            alloc->total_allocated -= freed_size;
        } else {
            freed_size = subclass->buf_size;
        }

        // XXX: why did i write the condition this way?
        //      wouldn't slab->buf_count < subclass->buf_max_count make more sense?
        //
        //      *i think* the reason i did it this was way to try and maximize an almost
        //      full page instead of constantly shuffling pages in the linked list.  maybe?
        if (slab->buf_count == subclass->buf_max_count - 1)
            move_front(alloc, slab);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto subclass = &alloc->subclass.slab;
        if (!subclass->head
        ||   subclass->count == subclass->buf_max_count) {
            grow(alloc);
        }

        auto head_slab = (slab_t*) subclass->head;
        u0* buf = head_slab->free_list;
        head_slab->free_list = *((u0**) buf);
        ++head_slab->buf_count;

        if (head_slab->buf_count == subclass->buf_max_count)
            move_back(alloc, head_slab);

        allocated_size = subclass->buf_size;
        return buf;
    }

    alloc_system_t g_alloc_system{
        .init       = init,
        .type       = alloc_type_t::slab,
        .free       = free,
        .alloc      = alloc,
        .release    = release,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::slab);
        // XXX: need to figure out how to do this
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }
}
