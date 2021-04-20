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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/assert.h>
#include <basecode/core/memory/system/page.h>

namespace basecode::memory::page {
    static u32 fini(alloc_t* alloc) {
        auto sc        = &alloc->subclass.page;
        auto curr_page = sc->head;
        u32 total_freed{};
        while (curr_page) {
            auto prev_page = curr_page->prev;
            total_freed += memory::internal::free(alloc->backing, curr_page);
            curr_page = prev_page;
        }
        return total_freed;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        UNUSED(mem);
        return 0;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        auto sc   = &alloc->subclass.page;
        auto page = (page_header_t*) mem - 1;
        if (page->next) page->next->prev = page->prev;
        if (page->prev) page->prev->next = page->next;
        if (sc->head == mem) sc->head    = page->next;
        if (sc->tail == mem) sc->tail    = page->prev;
        return memory::internal::free(alloc->backing, mem);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (page_config_t*) config;
        auto sc  = &alloc->subclass.page;
        sc->count      = {};
        sc->tail       = {};
        sc->head       = {};
        sc->cursor     = {};
        sc->num_pages  = cfg->num_pages;
        alloc->backing = cfg->backing.alloc;
        auto page_size = memory::system::os_alloc_granularity();
        sc->page_size  = (page_size * sc->num_pages) - sizeof(page_header_t);
        BC_ASSERT_NOT_NULL(alloc->backing);
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        UNUSED(size); UNUSED(align);
        auto sc      = &alloc->subclass.page;
        auto backing = alloc->backing;
        page_header_t* page{};
        if (sc->cursor) {
            page = sc->cursor;
            sc->cursor = sc->cursor->next;
        }
        mem_result_t r{};
        if (!page) {
            r = memory::internal::alloc(backing,
                                        sc->page_size,
                                        alignof(page_header_t));
            page = (page_header_t*) r.mem;
            sc->count++;
            if (!sc->tail) {
                sc->tail = page;
            }
            if (sc->head) {
                page->prev     = sc->head;
                sc->head->next = page;
            } else {
                page->prev = page->next = {};
            }
            sc->head = page;
        }
        return mem_result_t{++page, r.size};
    }

    alloc_system_t g_system{
        .size       = size,
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = {},
        .type       = alloc_type_t::page,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        BC_ASSERT_MSG(a && a->system->type == alloc_type_t::page,
                      "expected a non-null page allocator");
        auto sc = &a->subclass.page;
        sc->cursor = sc->tail;
    }

    alloc_system_t* system() {
        return &g_system;
    }
}
