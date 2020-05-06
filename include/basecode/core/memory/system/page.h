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

#pragma once

#include <basecode/core/memory.h>

namespace basecode {
    struct page_header_t final {
        page_header_t*          prev;
        page_header_t*          next;
    };

    struct page_config_t : alloc_config_t {
        alloc_t*                backing;
        u8                      num_pages;
    };

    namespace memory::page {
        u0 reset(alloc_t* alloc);

        alloc_system_t* system();

        force_inline u0* tail(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return ++sc->tail;
        }

        force_inline u0* head(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return ++sc->head;
        }

        force_inline u32 count(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return sc->count;
        }

        force_inline u16 page_size(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return sc->page_size;
        }
    }
}

