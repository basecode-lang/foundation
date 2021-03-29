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
#include <basecode/core/avl.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

template <Binary_Tree T>
static u0 print_avl_cursor(T& tree) {
    format::print("avl cursor: ");
    bin_tree_cursor_t<T> cursor{};
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

TEST_CASE("basecode::avl basics", "[avl]") {
    std::mt19937                       rg{std::random_device{}()};
    std::uniform_int_distribution<u32> pick(1, 4096);

    avl_t<u32> tree{};
    avl::init(tree);
    defer(avl::free(tree));

    set_t<u32> set{};
    set::init(set);

    for (u32 i = 0; i < 64; ++i)
        set::insert(set, pick(rg));

    TIME_BLOCK(
        "avl: insert 64 random u32 integers from set"_ss,
        for (auto v : set) {
            avl::insert(tree, v);
        });

    if (bintree::empty(tree))
        REQUIRE(!bintree::empty(tree));

    if (bintree::size(tree) != set.size) {
        u32 values[set.size];
        u32 x = {};
        for (auto v : set)
            values[x++] = v;
        std::sort(
            &values[0],
            &values[set.size - 1],
            [&](const auto lhs, const auto rhs) {
                return lhs < rhs;
            });
        print_avl_cursor(tree);
        format::print("avl values: ");
        for (u32 i = 0; i < set.size; ++i) {
            if (i > 0) format::print(",");
            format::print("{}", values[i]);
        }
        REQUIRE(bintree::size(tree) == set.size);
    }

    u32 found{};
    TIME_BLOCK(
        "avl: find integers from set in avl tree"_ss,
        for (auto v : set) {
            if (bintree::find(tree, v))
                ++found;
        });
    REQUIRE(found == set.size);

    set::free(set);

//    bintree::print_whole_tree(tree, "avl tree"_ss);
//    bintree::dump_dot(tree, "avl"_ss);
    print_avl_cursor(tree);
}
