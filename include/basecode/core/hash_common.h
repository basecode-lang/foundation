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

#include <basecode/core/bits.h>

namespace basecode::hash_common {
    u32 prime_capacity(u32 idx);

    b8 read_flag(const u64* data, u32 bit);

    u32 range_reduction(u64 hash, u32 size);

    u32 flag_words_for_capacity(u32 capacity);

    u0 write_flag(u64* data, u32 bit, b8 flag);

    u0 print_flags(const u64* flags, u32 size);

    s32 find_nearest_prime_capacity(u32 capacity);

    b8 requires_rehash(u32 size, u32 capacity, f32 load_factor);

    b8 find_free_bucket(const u64* hashes, u32 size, u32& bucket_idx);

    b8 find_free_bucket2(const u64* flags, u32 size, u32& bucket_idx);
}
