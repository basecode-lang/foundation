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

#include <basecode/core/memory.h>
#include <basecode/core/integer.h>

#define KNUTH_POW2_THRESHOLD_LEN    (6)
#define KNUTH_POW2_THRESHOLD_ZEROS  (3)

namespace basecode::integer {
    [[maybe_unused]] static u32 s_bits_per_digit[] = {
           0,    0, 1024, 1624, 2048, 2378, 2648, 2875, 3072, 3247, 3402,
        3543, 3672, 3790, 3899, 4001, 4096, 4186, 4271, 4350, 4426, 4498,
        4567, 4633, 4696, 4756, 4814, 4870, 4923, 4975, 5025, 5074, 5120,
        5166, 5210, 5253, 5295
    };

    u0 free(integer_t& num) {
        memory::free(num.alloc, num.data);
        num.data   = {};
        num.size   = num.capacity = {};
        num.offset = {};
    }

    u0 reset(integer_t& num) {
        num.size = num.offset = {};
    }

    integer_t pow(s32 exponent) {
        return integer_t{};
    }

    u0 normalize(integer_t& num) {
        if (!num.size) {
            num.offset = 0;
            return;
        }

        auto index = num.offset;
        if (num.data[index])
            return;

        auto index_bound = index + num.size;
        do {
            index++;
        } while (index < index_bound && !num.data[index]);

        const auto num_zeros = index - num.offset;
        num.size -= num_zeros;
        num.offset = (!num.size ? 0 : num.offset + num_zeros);
    }

    status_t init(integer_t& num,
                  const s8* data,
                  u32 len,
                  u32 radix,
                  alloc_t* alloc) {
        num.alloc    = alloc;
        num.size     = {};
        num.offset   = {};
        num.capacity = {};

        return status_t::ok;
    }

    integer_t abs(const integer_t& value) {
        return integer_t();
    }

    integer_t gcd(const integer_t& value) {
        return integer_t();
    }

    u0 grow(integer_t& num, u32 new_capacity) {
        new_capacity = std::max(new_capacity, num.capacity);
        reserve(num, new_capacity * 2);
    }

    str_t to_string(integer_t& num, u32 radix) {
        str_t str{};
        str::init(str, num.alloc);
        return str;
    }

    u0 reserve(integer_t& num, u32 new_capacity) {
        if (new_capacity == 0) {
            free(num);
            return;
        }

        if (new_capacity == num.capacity)
            return;

        new_capacity = std::max(num.size, new_capacity);
        num.data = (u64*) memory::realloc(
            num.alloc,
            num.data,
            new_capacity * sizeof(u64),
            alignof(u64));
        const auto data          = num.data + num.size;
        const auto size_to_clear = new_capacity > num.capacity ?
                                   new_capacity - num.capacity : 0;
        std::memset(data, 0, size_to_clear * sizeof(u64));
        num.capacity = new_capacity;
    }

    status_t init(integer_t& num, s64 value, alloc_t* alloc) {
        num.alloc    = alloc;
        num.size     = {};
        num.offset   = {};
        num.capacity = {};

        return status_t::ok;
    }
}
