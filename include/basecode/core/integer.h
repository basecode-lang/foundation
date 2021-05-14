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

#include <basecode/core/context.h>

namespace basecode::integer {
    u0 free(integer_t& num);

    u0 reset(integer_t& num);

    integer_t pow(s32 exponent);

    u0 normalize(integer_t& num);

    integer_t abs(const integer_t& value);

    integer_t gcd(const integer_t& value);

    str_t to_string(integer_t& num, u32 radix);

    u0 reserve(integer_t& num, u32 new_capacity);

    u0 grow(integer_t& num, u32 new_capacity = 2);

    inline integer_t operator-(const integer_t& lhs) {
        return integer_t{};
    }

    inline integer_t operator~(const integer_t& lhs) {
        return integer_t{};
    }

    inline integer_t operator<<(const integer_t& lhs, u32 n) {
        return integer_t{};
    }

    inline integer_t operator>>(const integer_t& lhs, u32 n) {
        return integer_t{};
    }

    status_t init(integer_t& num,
                  s64 value,
                  alloc_t* alloc = context::top()->alloc.main);

    status_t init(integer_t& num,
                  const s8* data,
                  u32 len,
                  u32 radix,
                  alloc_t* alloc = context::top()->alloc.main);

    status_t init(integer_t& num,
                  const String_Concept auto& value,
                  Radix_Concept auto radix,
                  alloc_t* alloc = context::top()->alloc.main) {
        return init(num,
                    (const s8*) value.data,
                    value.length,
                    radix,
                    alloc);
    }

    inline auto operator<=>(const integer_t& lhs, const integer_t& rhs) {
        return std::strong_ordering::equivalent;
    }

    inline integer_t operator&(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator|(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator^(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator/(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator%(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator*(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator+(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }

    inline integer_t operator-(const integer_t& lhs, const integer_t& rhs) {
        return integer_t{};
    }
}
