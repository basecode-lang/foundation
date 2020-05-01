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

#include <catch2/catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/bitset.h>

using namespace basecode;

TEST_CASE("basecode::set find_first_high bit") {
    bitset_t bs{};
    bitset::init(bs);
    bitset::resize(bs, 128);
    defer(bitset::free(bs));

    bitset::write(bs, 57, true);

    u32 bit_number = 57;
    REQUIRE(bitset::next_set_bit(bs, bit_number));
    REQUIRE(!bitset::next_set_bit(bs, ++bit_number));

    bitset::write(bs, 0, true);
    bitset::write(bs, 1, true);
    bitset::write(bs, 2, true);
    bit_number = 0;
    REQUIRE(bitset::next_set_bit(bs, bit_number));
    bitset::write(bs, 0, false);
    REQUIRE(bitset::next_set_bit(bs, ++bit_number));
    bitset::write(bs, 1, false);
    REQUIRE(bitset::next_set_bit(bs, ++bit_number));
}

TEST_CASE("basecode::set find_first_low bit") {
    bitset_t bs{};
    bitset::init(bs);
    bitset::resize(bs, 128);
    defer(bitset::free(bs));

    for (u32 bit = 0; bit < 128; ++bit)
        bitset::write(bs, bit, true);
    bitset::write(bs, 10, false);

    u32 bit_number = 5;
    REQUIRE(bitset::next_clear_bit(bs, bit_number));
    REQUIRE(bit_number == 10);

    for (u32 bit = 0; bit < 128; bit += 2)
        bitset::write(bs, bit, false);

    bit_number = 0;
    REQUIRE(bitset::next_clear_bit(bs, bit_number));
    REQUIRE(bit_number == 0);
    REQUIRE(bitset::next_clear_bit(bs, ++bit_number));
    REQUIRE(bit_number == 2);
}
