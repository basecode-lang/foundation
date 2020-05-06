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
#include <basecode/core/format.h>
#include <basecode/core/stable_array.h>

using namespace basecode;

TEST_CASE("basecode::stable_array insert") {
    auto numbers = stable_array::make<s32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(stable_array::free(numbers));

    REQUIRE(!stable_array::empty(numbers));
    REQUIRE(numbers.size == 9);

    stable_array::insert(numbers, 3, 30);
    REQUIRE(numbers[3] == 30);

    REQUIRE(numbers.size == 10);

    s32 k = 0;
    s32 j = 0;
    for (auto n : numbers) {
        if (j == 3) {
            REQUIRE(*n == 30);
        } else {
            REQUIRE(*n == k++);
        }
        ++j;
    }
}

TEST_CASE("basecode::stable_array with 64-bit values") {
    auto numbers = stable_array::make<s64>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(stable_array::free(numbers));

    REQUIRE(!stable_array::empty(numbers));
    REQUIRE(numbers.size == 9);

    stable_array::insert(numbers, s64(3), s64(30));
    REQUIRE(numbers[3] == 30);

    REQUIRE(numbers.size == 10);

    s64 k = 0;
    s64 j = 0;
    for (u32 i = 0; i < numbers.size; ++i) {
        auto n = numbers[i];
        if (j == 3) {
            REQUIRE(n == 30);
        } else {
            REQUIRE(n == k++);
        }
        ++j;
    }
}

TEST_CASE("basecode::stable_array erase") {
    auto numbers = stable_array::make<s32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(stable_array::free(numbers));

    REQUIRE(!stable_array::empty(numbers));
    REQUIRE(numbers.size == 9);

    stable_array::erase(numbers, 3);
    REQUIRE(numbers[3] == 4);
    for (u32 i = 0; i < numbers.size; ++i)
        format::print("{},", numbers[i]);
    format::print("\n");

    REQUIRE(numbers.size == 8);

    s32 k = 0;
    s32 j = 0;
    for (u32 i = 0; i < numbers.size; ++i) {
        auto n = numbers[i];
        if (j == 3) {
            REQUIRE(n == 4);
            k = n + 1;
        } else {
            REQUIRE(n == k++);
        }
        ++j;
    }
}

