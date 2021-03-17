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
#include <basecode/core/rbt.h>

using namespace basecode;

TEST_CASE("basecode::rbt basics") {
    std::mt19937                       rg{std::random_device{}()};
    std::uniform_int_distribution<u32> pick(0, 4096);

    rbt_t<u32> tree{};
    rbt::init(tree);
    defer(rbt::free(tree));

    set_t<u32> set{};
    set::init(set);

    for (u32 i = 1; i < 64; ++i)
        set::insert(set, pick(rg));

    set::for_each(set,
                  [](u32 idx, const auto& v, u0* user) -> u32 {
                      rbt_t<u32>& t = *((rbt_t<u32>*) user);
                      rbt::insert(t, v);
                      if (!rbt::find(t, v))
                          REQUIRE(false);
                      return 0;
                  },
                  &tree);

    if (rbt::empty(tree))               REQUIRE(!rbt::empty(tree));
    if (rbt::size(tree) != set.size)    REQUIRE(rbt::size(tree) == set.size);

    avl::print_whole_tree(tree, "red-black tree"_ss);
    avl::dump_dot(tree, "rbt"_ss);
}
