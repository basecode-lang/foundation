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

#include <random>
#include <catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/digraph.h>

using namespace basecode;

TEST_CASE("basecode::digraph basics") {
    digraph_t<u32> g{};
    digraph::init(g);
    defer(digraph::free(g));

    u32 one   = 1U;
    u32 two   = 2U;
    u32 three = 3U;
    u32 four  = 4U;
    u32 five  = 5U;
    u32 six   = 6U;
    u32 seven = 7U;
    u32 eight = 8U;

    auto n1 = digraph::make_node(g, one);
    auto n2 = digraph::make_node(g, two);
    auto n3 = digraph::make_node(g, three);
    auto n4 = digraph::make_node(g, four);
    auto n5 = digraph::make_node(g, five);
    auto n6 = digraph::make_node(g, six);
    auto n7 = digraph::make_node(g, seven);
    auto n8 = digraph::make_node(g, eight);

    //  N1 <---- N3 <------ N6 <---> N7
    //   |      * ^          ^        ^
    //   |     /  |          |        |
    //   |    /   |          |        |
    //   |   /    |          |        |
    //   *  /     |          |        |
    //  N2 <----- N4 <----> N5 <----- N8 <---+
    //                                 ^     |
    //                                 |     |
    //                                 +-----+
    digraph::make_edge(g, n1, n2);
    digraph::make_edge(g, n2, n3);
    digraph::make_edge(g, n3, n1);
    digraph::make_edge(g, n4, n2);
    digraph::make_edge(g, n4, n5);
    digraph::make_edge(g, n5, n4);
    digraph::make_edge(g, n5, n6);
    digraph::make_edge(g, n6, n3);
    digraph::make_edge(g, n6, n7);
    digraph::make_edge(g, n7, n6);
    digraph::make_edge(g, n8, n5);
    digraph::make_edge(g, n8, n7);
    digraph::make_edge(g, n8, n8);

    auto scc = digraph::strongly_connected(g);
    format::print("[");
    for (auto i = 0; i < scc.size; ++i) {
        if (i > 0) format::print(", ");
        auto& comp = scc[i];
        format::print("{{");
        for (auto j = 0; j < comp.size; ++j) {
            if (j > 0) format::print(",");
            format::print("{}", *(comp[j]->value));
        }
        format::print("}}");
        array::free(comp);
    }
    format::print("]\n");
    array::free(scc);
}
