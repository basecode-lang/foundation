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
    struct hashed_entry_t final {
        u64             hash;
        string::slice_t slice;
    };

    struct interned_t final {
        array::array_t<hashed_entry_t>  entries;
    };

    struct pool_t final {
        u8*                             buf{};
        u8*                             cursor{};
        array::array_t<u64>             hashes;
        array::array_t<interned_t>      interned;
        memory::allocator_t*            allocator;

        explicit pool_t(memory::allocator_t* allocator);
    };

    u0 free(pool_t& pool);

    u0 init(pool_t& pool, u32 buf_size = (64 * 1024) - 16);

    string::slice_t intern(pool_t& pool, string::slice_t value);

    pool_t make(memory::allocator_t* allocator = context::current()->allocator);
}

