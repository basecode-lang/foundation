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

#include <cassert>
#include <basecode/core/memory/system/page.h>

namespace basecode::memory::page {
    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (page_config_t*) config;
        auto sc  = &alloc->subclass.page;
        sc->count      = {};
        alloc->backing = cfg->backing;
        sc->cursor     = sc->tail = sc->head = {};
        auto page_size = memory::system::os_page_size();
        sc->page_size  = (page_size * std::max<u8>(1, cfg->num_pages)) - (sizeof(page_header_t) + sizeof(alloc_header_t));
        assert(alloc->backing);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto sc   = &alloc->subclass.page;
        auto page = (page_header_t*) mem - 1;
        if (page->next) page->next->prev = page->prev;
        if (page->prev) page->prev->next = page->next;
        if (sc->head == mem) sc->head    = page->next;
        if (sc->tail == mem) sc->tail    = page->prev;
        alloc->backing->system->free(alloc->backing, mem, freed_size);
        alloc->total_allocated -= freed_size;
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        auto backing   = alloc->backing->system;
        auto sc        = &alloc->subclass.page;
        auto curr_page = sc->head;
        u32 temp_freed{}, total_freed{};
        while (curr_page) {
            auto prev_page = curr_page->prev;
            backing->free(alloc->backing, curr_page, temp_freed);
            alloc->total_allocated -= temp_freed;
            total_freed += temp_freed;
            curr_page = prev_page;
        }
        if (freed_size) *freed_size = total_freed;
        if (enforce) assert(alloc->total_allocated == 0);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        UNUSED(size); UNUSED(align);
        auto sc      = &alloc->subclass.page;
        auto backing = alloc->backing;
        page_header_t* page{};
        if (sc->cursor) {
            page = sc->cursor;
            sc->cursor = sc->cursor->next;
        }
        if (!page) {
            page = (page_header_t*) backing->system->alloc(backing, sc->page_size, alignof(page_header_t), alloc_size);
            sc->count++;
            alloc->total_allocated += alloc_size;
            if (!sc->tail) sc->tail = page;
            if (sc->head) {
                page->prev     = sc->head;
                sc->head->next = page;
            } else {
                page->prev = page->next = {};
            }
            sc->head = page;
        }
        return ++page;
    }

    alloc_system_t g_system{
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = {},
        .type       = alloc_type_t::page,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::page);
        auto sc = &a->subclass.page;
        sc->cursor = sc->tail;
    }

    alloc_system_t* system() {
        return &g_system;
    }
}
