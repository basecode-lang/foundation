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

#include <catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/graphviz/gv.h>

using namespace basecode;

TEST_CASE("basecode::graphviz basics") {
    using namespace graphviz;

    graph_t g{};
    graph::init(g, graph_type_t::directed, "test"_ss);
    defer(graph::free(g));

    auto root = graph::make_node(g);
    node::label(*root, "root"_ss);
    node::style(*root, node_style_t::filled);
    node::fill_color(*root, color_t::aliceblue);

    auto lhs  = graph::make_node(g);
    node::label(*lhs, "lhs"_ss);
    node::style(*lhs, node_style_t::filled);
    node::fill_color(*lhs, color_t::pink);

    auto rhs  = graph::make_node(g);
    node::label(*rhs, "rhs"_ss);
    node::style(*rhs, node_style_t::filled);
    node::fill_color(*rhs, color_t::green);

    auto edge1 = graphviz::graph::make_edge(g);
    edge1->first  = graph::node_ref(&g, root->id);
    edge1->second = graph::node_ref(&g, lhs->id);
    edge::label(*edge1, "lhs"_ss);
    edge::dir(*edge1, dir_type_t::both);
    edge::arrow_tail(*edge1, arrow_type_t::dot);
    edge::arrow_head(*edge1, arrow_type_t::normal);

    auto edge2 = graph::make_edge(g);
    edge2->first  = graph::node_ref(&g, root->id);
    edge2->second = graph::node_ref(&g, rhs->id);
    edge::label(*edge2, "rhs"_ss);
    edge::dir(*edge2, dir_type_t::both);
    edge::arrow_tail(*edge2, arrow_type_t::dot);
    edge::arrow_head(*edge2, arrow_type_t::normal);

    auto p = "test.dot"_path;

    buf_t buf{};
    buf.mode = buf_mode_t::alloc;
    buf::init(buf);
    defer(
        buf::free(buf);
        path::free(p);
        );

    REQUIRE(OK(graph::serialize(g, buf)));
    REQUIRE(OK(buf::save(buf, p)));
}
