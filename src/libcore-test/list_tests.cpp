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
#include <basecode/core/list.h>
#include <basecode/core/defer.h>
#include <basecode/core/slice.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>
#include "test.h"

using namespace basecode;

template <typename V>
static u0 format_nodes(list_t<V>& list) {
    u32 n = 1;
    format::print("list: size = {}, nodes.size = {}, values.size = {}\n",
                  list.size,
                  list.nodes.size,
                  list.values.size);
    for (const auto& node : list.nodes) {
        format::print("{:>4}: free: {} prev: {} next: {:>4} value: {:>4}\n",
                      n++,
                      node.free,
                      node.prev,
                      node.next,
                      node.value);
    }
}

TEST_CASE("basecode::list basics") {
    list_t<s32> numbers{};
    list::init(numbers);
    defer({
        list::free(numbers);
    });

    for (s32 i = 1; i <= 20; ++i) {
        list::append(numbers, i);
    }

    REQUIRE(!list::empty(numbers));
    REQUIRE(numbers.size == 20);

//    list::remove(numbers, &numbers.nodes[3]);
//    REQUIRE(numbers.size == 19);

    list::append(numbers, 21);
    REQUIRE(numbers.size == 21);

    s32 sum{};
    for (auto node : numbers) {
        sum += list::value(numbers, node);
    }
    format::print("sum: {}\n", sum);
    REQUIRE(sum == 231);
    //format_nodes(numbers);
}
