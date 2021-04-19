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

        FORCE_INLINE u0* tail(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return ++sc->tail;
        }

        FORCE_INLINE u0* head(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return ++sc->head;
        }

        FORCE_INLINE u32 count(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return sc->count;
        }

        FORCE_INLINE u16 page_size(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::page);
            auto sc = &a->subclass.page;
            return sc->page_size;
        }
    }
}

