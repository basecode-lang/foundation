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
#include <basecode/core/array.h>
#include <basecode/core/format.h>

using namespace basecode;

TEST_CASE("basecode::array insert") {
    auto numbers = array::make<s32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(array::free(numbers));

    REQUIRE(!array::empty(numbers));
    REQUIRE(numbers.size == 9);

    array::insert(numbers, 3, 30);
    REQUIRE(numbers[3] == 30);

    REQUIRE(numbers.size == 10);

    s32 k = 0;
    s32 j = 0;
    for (auto n : numbers) {
        if (j == 3) {
            REQUIRE(n == 30);
        } else {
            REQUIRE(n == k++);
        }
        ++j;
    }
}

TEST_CASE("basecode::array erase") {
    auto numbers = array::make<s32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(array::free(numbers));

    REQUIRE(!array::empty(numbers));
    REQUIRE(numbers.size == 9);

    array::erase(numbers, 3);
    REQUIRE(numbers[3] == 4);
    for (auto n : numbers) format::print("{},", n);
    format::print("\n");

    REQUIRE(numbers.size == 8);

    s32 k = 0;
    s32 j = 0;
    for (auto n : numbers) {
        if (j == 3) {
            REQUIRE(n == 4);
            k = n + 1;
        } else {
            REQUIRE(n == k++);
        }
        ++j;
    }
}

TEST_CASE("array_t reserve space; fill") {
    array_t<s32> numbers{};
    array::init(numbers);
    array::reserve(numbers, 4096);
    defer(array::free(numbers));

    REQUIRE(array::empty(numbers));
    REQUIRE(numbers.size == 0);
    REQUIRE(numbers.capacity == 4096);

    for (s32 i = 0; i < 4096; i++)
        array::append(numbers, i);

    REQUIRE(!array::empty(numbers));
    REQUIRE(numbers.size == 4096);
    REQUIRE(numbers.capacity == 4096);

    std::sort(
        std::begin(numbers),
        std::end(numbers),
        [](auto lhs, auto rhs) {
            return lhs > rhs;
        });

    for (s32 i = 0; i < 4096; i++)
        REQUIRE(numbers[i] == 4095 - i);
}

TEST_CASE("basecode::array initializer list") {
    auto numbers = array::make<u32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(array::free(numbers));

    REQUIRE(!array::empty(numbers));
    REQUIRE(numbers.size == 9);

    s32 k = 0;
    for (auto n : numbers)
        REQUIRE(n == k++);
}
