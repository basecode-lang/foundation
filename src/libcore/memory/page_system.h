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

#include "memory.h"

namespace basecode {
    struct page_header_t final {
        u0*                     prev;
        u0*                     next;
    };

    struct page_config_t : alloc_config_t {
        alloc_t*                backing;
        u32                     page_size;
    };

    namespace memory::page {
        u0 reset(alloc_t* alloc);

        alloc_system_t* system();

        force_inline u0* tail(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto subclass = &a->subclass.page;
            return (u8*) subclass->tail + sizeof(page_header_t);
        }

        force_inline u0* head(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto subclass = &a->subclass.page;
            return (u8*) subclass->head + sizeof(page_header_t);
        }

        force_inline u16 size(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto subclass = &a->subclass.page;
            return subclass->size;
        }

        force_inline u32 count(alloc_t* alloc) {
            auto a = unwrap(alloc);
            assert(a && a->system->type == alloc_type_t::page);
            auto subclass = &a->subclass.page;
            return subclass->count;
        }
    }
}

