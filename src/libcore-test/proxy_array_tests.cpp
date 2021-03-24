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
#include <basecode/core/proxy_array.h>

using namespace basecode;

TEST_CASE("basecode::proxy_array basics") {
    array_t<s32> numbers{};
    array::init(numbers);
    defer(array::free(numbers));

    for (s32 i = 0; i < 4096; ++i) {
        array::append(numbers, i);
    }

    proxy_array_t<s32> proxy1{};
    proxy_array::init(proxy1, numbers, 0, 1024);

    proxy_array_t<s32> proxy2{};
    proxy_array::init(proxy2, numbers, 1024, 1024);

    u32 i{};
    for (auto n : proxy1) {
        if (n != numbers[i])
            REQUIRE(n == numbers[i]);
        ++i;
    }

    i = 1024;
    for (auto n : proxy2) {
        if (n != numbers[i])
            REQUIRE(n == numbers[i]);
        ++i;
    }
}
