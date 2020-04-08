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

#include <foundation/types.h>
#include <foundation/array/array.h>
#include "ascii_string.h"

namespace basecode::intern {
    struct hash_t final {
        u8 data[32]{};
    };

    struct pool_t final {
        u8*                             buf{};
        u8*                             cursor{};
        array::array_t<hash_t>          hashes;
        array::array_t<string::slice_t> interned;
        memory::allocator_t*            allocator;

        explicit pool_t(memory::allocator_t* allocator);
    };

    u0 free(pool_t& intern);

    u0 init(pool_t& intern, u32 buf_size = (64 * 1024) - 16);

    string::slice_t intern(pool_t& intern, string::slice_t value);

    pool_t make(memory::allocator_t* allocator = context::current()->allocator);
}

