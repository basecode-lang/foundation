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

namespace basecode {
    static inline u32 s_prime_capacities[] = {
        17,
        37,
        79,
        163,
        673,
        1361,
        2693,
        8081,
        16183,
        32369,
        64747,
        129497,
        259001,
        518017,
        1036039,
    };

    force_inline u32 prime_capacity(u32 idx) {
        return s_prime_capacities[idx];
    }

    force_inline u32 range_reduction(u64 hash, u32 size) {
        return hash % size;
    }

    force_inline u32 find_nearest_prime_capacity(u32 capacity) {
        for (auto p : s_prime_capacities) {
            if (p >= capacity)
                return p;
        }
        return 0;
    }

    b8 find_free_bucket(const u64* hashes, u32 size, u32& bucket_idx);

    force_inline b8 requires_rehash(u32 size, u32 capacity, f32 load_factor) {
        return capacity == 0 || f32(size) + 1 > f32(capacity - 1) * load_factor;
    }
}
