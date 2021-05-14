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

namespace basecode::bitset {
    u32 min(bitset_t& set);

    u32 max(bitset_t& set);

    u0 trim(bitset_t& set);

    u0 free(bitset_t& set);

    u0 reset(bitset_t& set);

    u0 clear(bitset_t& set);

    b8 empty(bitset_t& set);

    u32 count(bitset_t& set);

    b8 read(bitset_t& set, u32 bit);

    u32 size_in_bits(bitset_t& set);

    u32 size_in_words(bitset_t& set);

    u32 size_in_bytes(bitset_t& set);

    b8 next_set_bit(bitset_t& set, u32& bit);

    u0 write(bitset_t& set, u32 bit, b8 flag);

    b8 next_clear_bit(bitset_t& set, u32& bit);

    u0 union_of(bitset_t& lhs, const bitset_t& rhs);

    b8 any_set(const bitset_t& set, u32 start_bit = 0);

    u0 difference_of(bitset_t& lhs, const bitset_t& rhs);

    b8 disjoint(const bitset_t& lhs, const bitset_t& rhs);

    u0 intersection_of(bitset_t& lhs, const bitset_t& rhs);

    b8 contains_all(const bitset_t& lhs, const bitset_t& rhs);

    u32 union_count(const bitset_t& lhs, const bitset_t& rhs);

    bitset_t make(alloc_t* alloc = context::top()->alloc.main);

    b8 intersection_of(const bitset_t& lhs, const bitset_t& rhs);

    u0 symmetric_difference_of(bitset_t& lhs, const bitset_t& rhs);

    u32 difference_count(const bitset_t& lhs, const bitset_t& rhs);

    u32 intersection_count(const bitset_t& lhs, const bitset_t& rhs);

    u0 init(bitset_t& set, alloc_t* alloc = context::top()->alloc.main);

    u0 resize(bitset_t& set, u32 new_capacity, b8 pad_with_zeros = true);

    u32 symmetric_difference_count(const bitset_t& lhs, const bitset_t& rhs);
}
