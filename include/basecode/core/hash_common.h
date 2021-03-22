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

#include <basecode/core/bits.h>
#include <basecode/core/types.h>
#include <basecode/core/format.h>

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

    u0 write(u64* data, u32 bit, b8 flag);

    force_inline u32 prime_capacity(u32 idx) {
        return s_prime_capacities[idx];
    }

    force_inline b8 read(const u64* data, u32 bit) {
        const auto shifted_bit = bit >> 6U;
        return (data[shifted_bit] & (u64(1) << (bit % 64))) != 0;
    }

    force_inline u32 range_reduction(u64 hash, u32 size) {
        return hash % size;
    }

    force_inline u0 print_flags(const u64* flags, u32 size) {
        const auto num_words = std::max<u32>(next_power_of_two(size) / 64, 1);
        for (u32 i = 0; i < num_words; ++i) {
            format::print("{:0B}", flags[i]);
        }
        format::print("\n");
    }

    b8 find_free_bucket(const u64* hashes, u32 size, u32& bucket_idx);

    b8 find_free_bucket2(const u64* flags, u32 size, u32& bucket_idx);

    force_inline u32 flag_words_for_capacity(u32 capacity) {
        return std::max<u32>(
            std::max<u32>(next_power_of_two(capacity), 64) / 64,
            1);
    }

    force_inline s32 find_nearest_prime_capacity(u32 capacity) {
        for (u32 i = 0; i < sizeof(s_prime_capacities); ++i) {
            if (s_prime_capacities[i] >= capacity)
                return i;
        }
        return -1;
    }

    force_inline b8 requires_rehash(u32 size, u32 capacity, f32 load_factor) {
        if (capacity == 0)
            return true;
        const f32 lf = f32(size) / f32(capacity);
        return lf > load_factor;
    }
}
