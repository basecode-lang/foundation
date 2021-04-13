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

#include <basecode/core/integer.h>

namespace basecode {
    struct decimal_t final {
        alloc_t*                alloc;
        u64*                    data;
        u32                     size;
        u32                     capacity;
    };

    namespace decimal {
    }

    namespace hash {
        inline u64 hash64(const decimal_t& key) {
            u64 hash{};
            for (u32 i = 0; i < key.size; ++i)
                hash = 63 * hash + key.data[i];
            return hash;
        }
    }
}
