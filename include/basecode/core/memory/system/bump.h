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
    enum class bump_type_t : u8 {
        existing,
        allocator
    };

    struct bump_config_t : alloc_config_t {
        bump_config_t() : alloc_config_t(alloc_type_t::bump) {}

        bump_type_t             type;
    };

    namespace memory::bump {
        u0 reset(alloc_t* alloc);

        alloc_system_t* system();

        FORCE_INLINE u0* buf(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::bump);
            auto subclass = &a->subclass.bump;
            return subclass->buf;
        }

        FORCE_INLINE u16 offset(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::bump);
            auto subclass = &a->subclass.bump;
            return subclass->offset;
        }

        u0 buf(alloc_t* alloc, u0* buf, u32 size);

        FORCE_INLINE u16 end_offset(alloc_t* alloc) {
            auto a = unwrap(alloc);
            BC_ASSERT(a && a->system->type == alloc_type_t::bump);
            auto subclass = &a->subclass.bump;
            return subclass->end_offset;
        }
    }
}

