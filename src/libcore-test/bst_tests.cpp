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

    set_t<u32> set{};
    set::init(set);

    for (u32 i = 1; i < 64; ++i)
        set::insert(set, pick(rg));

    set::for_each(set,
                  [](u32 idx, const auto& v, u0* user) -> u32 {
                      bst_t<u32>& t = *((bst_t<u32>*) user);
                      bst::insert(t, v);
                      if (!bst::find(t, v))
                          REQUIRE(false);
                      return 0;
                  },
                  &tree);

    if (bst::empty(tree))               REQUIRE(!bst::empty(tree));
    if (bst::size(tree) != set.size)    REQUIRE(bst::size(tree) == set.size);

    avl::print_whole_tree(tree, "before balance"_ss);
    bst::balance(tree);
    avl::print_whole_tree(tree, "after balance "_ss);

    bin_tree_cursor_t<bst_t<u32>> cursor{};
    avl::cursor::init(cursor, &tree);
    s32 i;
    u32* v;
    for (v = avl::cursor::first(cursor, &tree), i = 0;
            v;
            ++i, v = avl::cursor::next(cursor)) {
        if (i > 0) format::print(",");
        format::print("{}", *v);
    }
    format::print("\n");
}
