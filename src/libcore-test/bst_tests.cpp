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
#include <basecode/core/set.h>
#include <basecode/core/bst.h>

using namespace basecode;

TEST_CASE("basecode::bst basics") {
    std::mt19937                       rg{std::random_device{}()};
    std::uniform_int_distribution<u32> pick(0, 4096);

    bst_t<u32> tree{};
    bst::init(tree);
    defer(bst::free(tree));

    {
        set_t<u32> set{};
        set::init(set);

        for (u32 i = 1; i < 64; ++i)
            set::insert(set, pick(rg));

        auto values = set::values(set);
        for (auto v : values) {
            bst::insert(tree, *v);
        }

        if (bintree::empty(tree))
            REQUIRE(!bintree::empty(tree));
        if (bintree::size(tree) != set.size)
            REQUIRE(bintree::size(tree) == set.size);

        for (auto v : values) {
            if (!bintree::find(tree, *v)) {
                REQUIRE(false);
            }
        }

        array::free(values);
        set::free(set);
    }

    bintree::print_whole_tree(tree, "before balance"_ss);
    bst::balance(tree);
    bintree::print_whole_tree(tree, "after balance "_ss);

    format::print("bst cursor: ");
    bin_tree_cursor_t<bst_t<u32>> cursor{};
    bintree::cursor::init(cursor, &tree);
    s32 i;
    u32* v;
    for (v = bintree::cursor::first(cursor, &tree), i = 0;
            v;
            ++i, v = bintree::cursor::next(cursor)) {
        if (i > 0) format::print(",");
        format::print("{}", *v);
    }
    format::print("\n");
}
