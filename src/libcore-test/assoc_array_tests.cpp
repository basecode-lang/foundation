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
#include <basecode/core/stopwatch.h>
#include <basecode/core/assoc_array.h>

using namespace basecode;

TEST_CASE("basecode::assoc_array basics") {
    assoc_array_t<u32> pairs{};
    assoc_array::init(pairs);
    defer(assoc_array::free(pairs));

    str_t temp{};
    str::init(temp);
    str::reserve(temp, 32);
    defer(str::free(temp));

    stopwatch_t time{};
    stopwatch::start(time);

    for (u32 i = 0; i < 100; ++i) {
        str::append(temp, "key_");
        str::random(temp, 4);
        assoc_array::append(pairs, temp, i);
        str::reset(temp);
    }

    stopwatch::stop(time);
    stopwatch::print_elapsed("assoc_array: append"_ss, 40, stopwatch::elapsed(time));

//    for (u32 i = 0; i < 100; ++i) {
//        if (i > 0) format::print(",");
//        auto pair = pairs[i];
//        format::print("{}={}", pair.key, *pair.value);
//    }
//    format::print("\n");

    REQUIRE(pairs.size == 100);
    REQUIRE(pairs.keys.size == (100 * 9));
}

TEST_CASE("basecode::assoc_array find by key") {
    assoc_array_t<u32> pairs{};
    assoc_array::init(pairs);
    defer(assoc_array::free(pairs));

    assoc_array::append(pairs, "test1"_ss, u32(10));
    assoc_array::append(pairs, "test2"_ss, u32(20));
    assoc_array::append(pairs, "test3"_ss, u32(30));
    assoc_array::append(pairs, "test4"_ss, u32(40));

    stopwatch_t time{};
    stopwatch::start(time);

    auto value = assoc_array::find(pairs, "test4"_ss);

    stopwatch::stop(time);
    stopwatch::print_elapsed("assoc_array: find"_ss, 40, stopwatch::elapsed(time));

    REQUIRE(value);
    REQUIRE(*value == 40);
}
