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

#include <climits>
#include <basecode/core/types.h>

namespace basecode {
    u8 to_hex(u8 x);

    u64 bitmask(u64 n);

    u8 from_hex(u8 ch);

    u8 unybble(u8 value);

    u8 lnybble(u8 value);

    u64 rotl(u64 n, u8 c);

    u64 rotr(u64 n, u8 c);

    u64 power(u64 x, u64 n);

    b8 is_power_of_two(s64 x);

    template <typename T>
    T twos_complement(T value) {
        return ~value + 1;
    }

    u32 next_power_of_two(u32 n);

    u64 next_power_of_two(u64 n);

    b8 is_platform_little_endian();

    u16 endian_swap_word(u16 value);

    u32 previous_power_of_two(u32 n);

    u64 previous_power_of_two(u64 n);

    u32 endian_swap_dword(u32 value);

    u64 endian_swap_qword(u64 value);

    template <typename T, unsigned B>
    inline T sign_extend(const T x) {
        struct {T x:B;} s;
        return s.x = x;
    }

    u8 lnybble(u8 original, u8 value);

    u8 unybble(u8 original, u8 value);

    inline u64 align(u64 size, u64 align) {
        return size + (-size & (align - 1));
    }

    template <typename T>
    inline b8 is_sign_bit_set(T value) {
        return (value & (T(1) << ((sizeof(T) * CHAR_BIT) - 1))) != 0;
    }
}
