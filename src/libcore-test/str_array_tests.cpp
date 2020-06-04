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
#include <basecode/core/str_array.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::str_array basics") {
    str_array_t array{};
    str_array::init(array);
    defer(str_array::free(array));

    str_t temp{};
    str::init(temp);
    str::reserve(temp, 32);
    defer(str::free(temp));

    stopwatch_t time{};
    stopwatch::start(time);

    for (u32 i = 0; i < 100; ++i) {
        str::random(temp, 32);
        str_array::append(array, temp);
        str::reset(temp);
    }

    stopwatch::stop(time);
    stopwatch::print_elapsed("str_array: append"_ss, 40, time);
    //format::print("{}\n", array);

    REQUIRE(array.size == 100);
    REQUIRE(array.buf.size == (100 * 33));
}
