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

#include <basecode/core/bits.h>

namespace basecode {
    u8 to_hex(u8 x) {
        return x + (x > 9 ? ('A' - 10) : '0');
    }

    u64 bitmask(u64 n) {
        return u64(1) << n;
    }

    u8 from_hex(u8 ch) {
        if (ch <= '9' && ch >= '0')
            ch -= '0';
        else if (ch <= 'f' && ch >= 'a')
            ch -= 'a' - 10;
        else if (ch <= 'F' && ch >= 'A')
            ch -= 'A' - 10;
        else
            ch = 0;
        return ch;
    }

    u8 unybble(u8 value) {
        return u8((value & u32(0xf0)) >> u32(4));
    }

    u8 lnybble(u8 value) {
        return u8(value & u32(0x0f));
    }

    u64 rotl(u64 n, u8 c) {
        const u64 mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n << c) | (n >> (-c & mask));
    }

    u64 rotr(u64 n, u8 c) {
        const u64 mask = (CHAR_BIT * sizeof(n) - 1);
        c &= mask;
        return (n >> c) | (n << (-c & mask));
    }

    u64 power(u64 x, u64 n) {
        u64 pow = 1;
        while (n) {
            if (n & u32(1))
                pow *= x;
            n = n >> u32(1);
            x = x * x;
        }
        return pow;
    }

    b8 is_power_of_two(s64 x) {
        if (x <= 0)
            return false;
        return !(x & (x-1));
    }

    u32 next_power_of_two(u32 n) {
        n--;
        n |= n >> u32(1);
        n |= n >> u32(2);
        n |= n >> u32(4);
        n |= n >> u32(8);
        n |= n >> u32(16);
        n++;
        return n;
    }

    u64 next_power_of_two(u64 n) {
        n--;
        n |= n >> u32(1);
        n |= n >> u32(2);
        n |= n >> u32(4);
        n |= n >> u32(8);
        n |= n >> u32(16);
        n |= n >> u32(32);
        n++;
        return n;
    }

    b8 is_platform_little_endian() {
        s32 n = 1;
        return (*(s8*) & n) == 1;
    }

    u64 align(u64 size, u64 align) {
        if (align > 0) {
            auto result = size + align - 1;
            return result - result % align;
        }
        return size;
    }

    u16 endian_swap_word(u16 value) {
        return (value >> u16(8)) | (value << u16(8));
    }

    u32 previous_power_of_two(u32 n) {
        n |= n >> u32(1);
        n |= n >> u32(2);
        n |= n >> u32(4);
        n |= n >> u32(8);
        n |= n >> u32(16);
        return n - (n >> u32(1));
    }

    u64 previous_power_of_two(u64 n) {
        n |= n >> u32(1);
        n |= n >> u32(2);
        n |= n >> u32(4);
        n |= n >> u32(8);
        n |= n >> u32(16);
        n |= n >> u32(32);
        return n - (n >> u32(1));
    }

    u32 endian_swap_dword(u32 value) {
        return ((value >> 24) & 0xff)
               |  ((value << 8) & 0xff0000)
               |  ((value >> 8) & 0xff00)
               |  ((value << 24) & 0xff000000);
    }

    u64 endian_swap_qword(u64 value) {
        return ((value & 0x00000000000000ffu) << 56) |
               ((value & 0x000000000000ff00u) << 40) |
               ((value & 0x0000000000ff0000u) << 24) |
               ((value & 0x00000000ff000000u) << 8)  |
               ((value & 0x000000ff00000000u) >> 8)  |
               ((value & 0x0000ff0000000000u) >> 24) |
               ((value & 0x00ff000000000000u) >> 40) |
               ((value & 0xff00000000000000u) >> 56);
    }

    u8 lnybble(u8 original, u8 value) {
        u8 res = original;
        res &= u32(0xf0);
        res |= (value & u32(0x0f));
        return res;
    }

    u8 unybble(u8 original, u8 value) {
        u8 res = original;
        res &= u32(0x0f);
        res |= ((value << u32(4)) & u32(0xf0));
        return res;
    }

    u64 sign_extend(s64 value, u32 bits) {
        auto shift = sizeof(u64) * CHAR_BIT - bits;
        auto result = (value << shift) >> shift;
        return static_cast<u64>(result);
    }
}
