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
    graphviz::node::style(*root, graphviz::node_style_t::filled);
    graphviz::node::fill_color(*root, graphviz::color_t::aliceblue);

    auto lhs  = graphviz::graph::make_node(g, "lhs"_ss);
    graphviz::node::style(*lhs, graphviz::node_style_t::filled);
    graphviz::node::fill_color(*lhs, graphviz::color_t::pink);

    auto rhs  = graphviz::graph::make_node(g, "rhs"_ss);
    graphviz::node::style(*rhs, graphviz::node_style_t::filled);
    graphviz::node::fill_color(*rhs, graphviz::color_t::green);

    auto edge1 = graphviz::graph::make_edge(g);
    edge1->first  = root->id;
    edge1->second = lhs->id;
    graphviz::edge::label(*edge1, "lhs"_ss);
    graphviz::edge::dir(*edge1, graphviz::dir_type_t::both);
    graphviz::edge::arrow_tail(*edge1, graphviz::arrow_type_t::dot);
    graphviz::edge::arrow_head(*edge1, graphviz::arrow_type_t::normal);

    auto edge2 = graphviz::graph::make_edge(g);
    edge2->first  = root->id;
    edge2->second = rhs->id;
    graphviz::edge::label(*edge2, "rhs"_ss);
    graphviz::edge::dir(*edge2, graphviz::dir_type_t::both);
    graphviz::edge::arrow_tail(*edge2, graphviz::arrow_type_t::dot);
    graphviz::edge::arrow_head(*edge2, graphviz::arrow_type_t::normal);

    auto p = "C:/temp/test.dot"_path;

    buf_t buf{};
    buf.mode = buf_mode_t::alloc;
    buf::init(buf);
    defer(
        buf::free(buf);
        path::free(p);
        );

    REQUIRE(OK(graphviz::graph::serialize(g, buf)));
    REQUIRE(OK(buf::save(buf, p)));
}
