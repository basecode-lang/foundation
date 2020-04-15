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

#include <basecode/core/types.h>
#include <basecode/core/array/array.h>
#include <basecode/core/hashtable/hashtable.h>
#include "ascii_string.h"

namespace basecode::intern {
    enum class status_t : u8 {
        ok,
        not_found,
        no_available_bucket
    };

    struct index_t final {
        id_t*                           ids;
        u64*                            hashes;
        string::slice_t*                slices;
    };

    struct result_t final {
        id_t                            id{};
        u64                             hash{};
        string::slice_t                 slice{};
        status_t                        status{};
    };

    struct pool_t final {
        id_t                            id;
        u8*                             head;
        u8*                             page;
        u32                             size;
        u8*                             index;
        u16                             offset;
        u32                             capacity;
        memory::allocator_t*            allocator;
        u16                             end_offset;
    };

    namespace pool {
        u0 init(
            pool_t& pool,
            memory::allocator_t* allocator = context::current()->allocator);

        u0 free(pool_t& pool);

        result_t get(pool_t& pool, id_t id);

        result_t intern(pool_t& pool, string::slice_t value);

        pool_t make(memory::allocator_t* allocator = context::current()->allocator);
    }
}

