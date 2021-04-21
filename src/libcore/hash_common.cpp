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

#include <basecode/core/format.h>
#include <basecode/core/hash_common.h>

namespace basecode::hash_common {
    static u32 s_prime_capacities[] = {
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
        1395263,
        1674319,
        2009191,
        2411033,
        2893249,
        3471899,
        4166287,
        4999559,
        5999471,
        7199369,
        16601593,
        33712729,
        68460391,
        139022417,
        282312799,
        573292817,
        116418621l,
        2364114217,
        4294967291,
    };

    u32 prime_capacity(u32 idx) {
        return s_prime_capacities[idx];
    }

    b8 read_flag(const u64* data, u32 bit) {
        const auto shifted_bit = bit >> 6U;
        return (data[shifted_bit] & (u64(1) << (bit % 64))) != 0;
    }

    u32 range_reduction(u64 hash, u32 size) {
        return hash % size;
    }

    u32 flag_words_for_capacity(u32 capacity) {
        auto pow2 = next_power_of_two(capacity);
        return std::max<u32>(std::max<u32>(pow2, 64) / 64, 1);
    }

    u0 print_flags(const u64* flags, u32 size) {
        const auto num_words = std::max<u32>(next_power_of_two(size) / 64, 1);
        for (u32 i = 0; i < num_words; ++i) {
            format::print("{:0B}", flags[i]);
        }
        format::print("\n");
    }

    u0 write_flag(u64* data, u32 bit, b8 flag) {
        const auto shifted_bit = bit >> 6U;
        const auto mask        = u64(1) << (bit % 64);
        const auto new_mask    = u64(flag) << (bit % 64);
        auto word = data[shifted_bit];
        word &= ~mask;
        word |= new_mask;
        data[shifted_bit] = word;
    }

    s32 find_nearest_prime_capacity(u32 capacity) {
        for (u32 i = 0; i < sizeof(s_prime_capacities); ++i) {
            if (s_prime_capacities[i] >= capacity)
                return i;
        }
        return -1;
    }

    b8 requires_rehash(u32 size, u32 capacity, f32 load_factor) {
        if (capacity == 0)
            return true;
        const f32 lf = f32(size) / f32(capacity);
        return lf > load_factor;
    }

    b8 find_free_bucket(const u64* hashes, u32 size, u32& bucket_idx) {
        for (u32 i = bucket_idx; i < size; ++i) {
            if (!hashes[i]) {
                bucket_idx = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_idx; ++i) {
            if (!hashes[i]) {
                bucket_idx = i;
                return true;
            }
        }
        return false;
    }

    b8 find_free_bucket2(const u64* flags, u32 size, u32& bucket_idx) {
        for (u32 i = bucket_idx; i < size; ++i) {
            if (!read_flag(flags, i)) {
                bucket_idx = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_idx; ++i) {
            if (!read_flag(flags, i)) {
                bucket_idx = i;
                return true;
            }
        }
        return false;
    }
}
