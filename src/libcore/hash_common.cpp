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

#include <basecode/core/hash_common.h>

namespace basecode {
    u0 write(u64* data, u32 bit, b8 flag) {
        const auto shifted_bit = bit >> 6U;
        const auto mask        = u64(1) << (bit % 64);
        const auto new_mask    = u64(flag) << (bit % 64);
        auto word = data[shifted_bit];
        word &= ~mask;
        word |= new_mask;
        data[shifted_bit] = word;
    }

    b8 find_free_bucket2(const u64* flags, u32 size, u32& bucket_idx) {
        for (u32 i = bucket_idx; i < size; ++i) {
            if (!read(flags, i)) {
                bucket_idx = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_idx; ++i) {
            if (!read(flags, i)) {
                bucket_idx = i;
                return true;
            }
        }
        return false;
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
}
