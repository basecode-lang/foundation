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
    static page_header_t* grow(alloc_t* alloc);

    static u0 remove(alloc_t* alloc, page_header_t* page);

    static page_header_t* move_back(alloc_t* alloc, page_header_t* page);

    [[maybe_unused]]
    static page_header_t* move_front(alloc_t* alloc, page_header_t* page);

    static u32 fini(alloc_t* alloc) {
        auto sc        = &alloc->subclass.page;
        auto curr_page = sc->head;
        page_header_t* to_free[sc->count];
        u32 total_freed{};
        u32 idx{};
        while (curr_page) {
            to_free[idx++] = curr_page;
            curr_page = curr_page->next;
        }
        for (page_header_t* page : to_free)
            total_freed += memory::internal::free(alloc->backing, page);
        return total_freed;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        UNUSED(mem);
        return 0;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        auto page = (page_header_t*) mem - 1;
        remove(alloc, page);
        return memory::internal::free(alloc->backing, mem);
    }

    static page_header_t* grow(alloc_t* alloc) {
        auto       sc = &alloc->subclass.page;
        const auto r  = memory::internal::alloc(alloc->backing,
                                                sc->page_size,
                                                alignof(page_header_t));
        std::memset(r.mem, 0, sizeof(page_header_t));
        sc->count++;
        return move_back(alloc, (page_header_t*) r.mem);
    }

    static u0 remove(alloc_t* alloc, page_header_t* page) {
        auto sc = &alloc->subclass.page;
        if (page->next)         page->next->prev = page->prev;
        if (page->prev)         page->prev->next = page->next;
        if (sc->head == page)   sc->head         = page->next;
        if (sc->tail == page)   sc->tail         = page->prev;
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
        UNUSED(size);
        UNUSED(align);
        auto sc = &alloc->subclass.page;
        mem_result_t r{};
        if (sc->cursor) {
            r.mem = (u8*) sc->cursor + sizeof(page_header_t);
            r.size     = 0;
            sc->cursor = sc->cursor->next;
        } else {
            auto page = grow(alloc);
            r.mem  = (u8*) page + sizeof(page_header_t);
            r.size = sc->page_size;
        }
        return r;
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
        sc->cursor = sc->head;
    }

    alloc_system_t* system() {
        return &g_system;
    }

    static page_header_t* move_back(alloc_t* alloc, page_header_t* page) {
        auto sc = &alloc->subclass.page;
        if (sc->tail == page)
            return sc->tail;
        remove(alloc, page);
        page->next = {};
        if (sc->tail) {
            sc->tail->next = page;
            page->prev = sc->tail;
            if (!sc->head)
                sc->head = sc->tail;
        } else {
            sc->head = page;
        }
        sc->tail = page;
        return sc->tail;
    }

    static page_header_t* move_front(alloc_t* alloc, page_header_t* page) {
        auto sc  = &alloc->subclass.page;
        if (sc->head == page)
            return page;
        remove(alloc, page);
        page->prev = {};
        if (sc->head) {
            sc->head->prev = page;
            page->next     = sc->head;
            if (!sc->tail)
                sc->tail   = sc->head;
        } else {
            sc->tail = page;
        }
        sc->head = page;
        return sc->head;
    }
}
