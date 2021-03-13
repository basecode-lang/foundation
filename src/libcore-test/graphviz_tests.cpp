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
#include <basecode/core/stopwatch.h>
#include <basecode/core/graphviz/gv.h>

using namespace basecode;

TEST_CASE("basecode::graphviz basics") {
    graphviz::graph_t g{};
    graphviz::graph::init(g, graphviz::graph_type_t::directed, "test"_ss);
    defer(graphviz::graph::free(g));

    auto root = graphviz::graph::make_node(g, "root"_ss);
    auto lhs  = graphviz::graph::make_node(g, "lhs"_ss);
    auto rhs  = graphviz::graph::make_node(g, "rhs"_ss);

    auto edge1 = graphviz::graph::make_edge(g, "1"_ss);
    edge1->first  = root->id;
    edge1->second = lhs->id;
    graphviz::edge::arrow_head(*edge1, graphviz::arrow_type_t::box);
    graphviz::edge::arrow_tail(*edge1, graphviz::arrow_type_t::normal);

    auto edge2 = graphviz::graph::make_edge(g, "2"_ss);
    edge2->first  = root->id;
    edge2->second = rhs->id;
    graphviz::edge::arrow_head(*edge2, graphviz::arrow_type_t::box);
    graphviz::edge::arrow_tail(*edge2, graphviz::arrow_type_t::normal);

    buf_t buf{};
    buf.mode = buf_mode_t::alloc;
    buf::init(buf);
    defer(buf::free(buf));

    REQUIRE(OK(graphviz::dot::serialize(g, buf)));
}
