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

#include <basecode/core/buf.h>

#define FILE_READ_SLEB128(t, v, c)          SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);                      \
    if (!OK(leb::decode_signed(file.crsr, (v), (c))))                   \
        return status_t::read_error;)
#define FILE_READ_ULEB128(t, v, c)          SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);                      \
    if (!OK(leb::decode_unsigned(file.crsr, (v), (c))))                 \
        return status_t::read_error;)
#define FILE_READ_SLICE(s, c)               SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(s), str::slice_t>);           \
    FILE_READ_ULEB128(u32, (s).length, (c));                            \
    (s).data = FILE_PTR();                                              \
    FILE_SEEK_FWD((s).length);)

namespace basecode::leb {
    u0 init(leb128_t& leb);

    template <typename T,
        b8 Is_Signed = std::numeric_limits<T>::is_signed,
        std::enable_if_t<std::numeric_limits<T>::is_integer, b8> = true>
    leb128_t encode(T value, u32 pad = 0) {
        leb128_t leb{};
        init(leb);
        leb.is_signed = Is_Signed;
        if constexpr (Is_Signed) {
            b8 more;
            u32 count{};
            do {
                u8 byte = value & lower_seven_bits;
                value >>= 7;
                more = !((((value == 0) && ((byte & sign_bit) == 0))
                    ||   ((value == -1) && ((byte & sign_bit) != 0))));
                count++;
                if (more || count < pad)
                    byte |= continuation_bit;
                leb.data[leb.size++] = byte;
            } while (more);
            return leb;
        } else {
            u32 count{};
            do {
                u8 b = value & lower_seven_bits;
                value >>= 7;
                count++;
                if (value || count < pad)
                    b |= continuation_bit;
                leb.data[leb.size++] = b;
            } while (value);
            return leb;
        }
    }

    template <typename T,
        u32 Size_In_Bits = sizeof(T) * 8,
        std::enable_if_t<std::numeric_limits<T>::is_integer, b8> = true>
    T decode(const leb128_t& leb) {
        T   value   {};
        u32 shift   {};
        u8  byte;
        const u8* p = leb.data;
        if (leb.is_signed) {
            do {
                byte = *p;
                T slice = byte & lower_seven_bits;
                value |= slice << shift;
                shift += 7;
                ++p;
            } while (byte >= 128);
            if (shift < Size_In_Bits && (byte & sign_bit))
                value |= T(-1) << shift;
        } else {
            do {
                T slice = *p & lower_seven_bits;
                value += slice << shift;
                shift += 7;
            } while(*p++ >= 128);
        }
        return value;
    }

    template <typename T,
        u32 Size_In_Bits = sizeof(T) * 8,
        std::enable_if_t<std::numeric_limits<T>::is_integer, b8> = true>
    status_t decode_signed(buf_crsr_t& crsr, T& value, u32& count) {
        u32 shift{};
        u8  byte;
        value = {};
        count = {};
        do {
            buf::cursor::read(crsr, byte);
            T slice = byte & lower_seven_bits;
            value |= slice << shift;
            shift += 7;
            ++count;
        } while (byte >= 128);
        if (shift < Size_In_Bits && (byte & sign_bit))
            value |= T(-1) << shift;
        return status_t::ok;
    }

    template <typename T,
        std::enable_if_t<std::numeric_limits<T>::is_integer, b8> = true>
    status_t decode_unsigned(buf_crsr_t& crsr, T& value, u32& count) {
        u32 shift{};
        u8  byte;
        value = {};
        count = {};
        do {
            buf::cursor::read(crsr, byte);
            T slice = byte & lower_seven_bits;
            value += slice << shift;
            shift += 7;
            ++count;
        } while(byte >= 128);
        return status_t::ok;
    }
}
