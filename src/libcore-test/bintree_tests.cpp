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
#include <basecode/core/bintree.h>

using namespace basecode;

TEST_CASE("basecode::bintree basics") {
    bintree_t<alloc_t*> tree{};
    bintree::init(tree);
    defer(bintree::free(tree));

    bintree_node_t* root_node{};
    bintree::append_node(tree, &root_node);
    bintree::insert_value(tree, root_node, memory::system::default_alloc());

    REQUIRE(root_node == bintree::root(tree));
    REQUIRE(!bintree::empty(tree));
    REQUIRE(bintree::size(tree) == 1);
}
