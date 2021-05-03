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

#include <catch.hpp>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::variant basics") {
    variant_t one(s32(1));
    variant_t two(u8(2));
    variant_t three(f32(3.14));

    auto one_val = *one;
    auto two_val = *two;
    auto three_val = *three;

    REQUIRE(one_val == s32(1));
    REQUIRE(two_val == u8(2));
    REQUIRE(three_val == f32(3.14));

    format::print("one = {}, two = {}, three = {}\n", one, two, three);
}
