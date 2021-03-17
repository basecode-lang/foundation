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

#include <random>
#include <catch2/catch.hpp>
#include <basecode/core/bst.h>

using namespace basecode;

TEST_CASE("basecode::bst basics") {
    std::mt19937                       rg{std::random_device{}()};
    std::uniform_int_distribution<u32> pick(0, 4096);

    bst_t<u32> tree{};
    bst::init(tree);
    defer(bst::free(tree));

    array_t<u32> values{};
    array::init(values);

    for (u32 i = 1; i < 8; ++i)
        array::append(values, i);

    for (auto v : values)
        bst::insert(tree, v);

    for (auto v : values) {
        if (!bst::find(tree, v))
            REQUIRE(false);
    }

    if (bst::empty(tree))               REQUIRE(false);
    if (bst::size(tree) != values.size) REQUIRE(false);

    bst::print_whole_tree(tree, "before balance"_ss);
    bst::balance(tree);
    bst::print_whole_tree(tree, "after balance "_ss);
}
