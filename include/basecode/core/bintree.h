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

#pragma once

#include <basecode/core/buf.h>
#include <basecode/core/types.h>
#include <basecode/core/format.h>
#include <basecode/core/graphviz/gv.h>

#define NODE_BRANCH(n, d)       ((d) == 1 ? (n)->rhs : (n)->lhs)

namespace basecode {
    template <Binary_Tree Tree_Type, typename Node_Type>
    struct bin_tree_cursor_t final {
        Tree_Type*              tree;
        Node_Type               node;
    };

    namespace bintree {
        inline u0 make_nil_node(graphviz::graph_t& g,
                                u32 id, str::slice_t label);

        inline graphviz::edge_t* make_edge(graphviz::graph_t& g,
                                           u32 first,
                                           u32 second,
                                           str::slice_t label = {});

        inline u32 size(const Binary_Tree auto& tree) {
            return tree.size;
        }

        inline b8 empty(const Binary_Tree auto& tree) {
            return tree.size == 0;
        }

        template <Binary_Tree T,
                  typename Has_Color = typename T::Has_Color,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        u32 dump_node(T& tree,
                      graphviz::graph_t& g,
                      Node_Type node, u32 parent_id = {}) {
            auto n  = graphviz::graph::make_node(g);
            auto id = n->id;
            graphviz::node::label(*n,
                                  format::format("{}{}",
                                                 tree.root == node ? "ROOT\n"_ss : ""_ss,
                                                 *node->value));
            graphviz::node::style(*n, graphviz::node_style_t::filled);
            if constexpr (Has_Color::value) {
                if (u8(node->color) == color::red) {
                    graphviz::node::fill_color(*n, graphviz::color_t::red);
                    graphviz::node::font_color(*n, graphviz::color_t::black);
                } else {
                    graphviz::node::fill_color(*n, graphviz::color_t::black);
                    graphviz::node::font_color(*n, graphviz::color_t::white);
                }
            } else {
                if (tree.root == node) {
                    graphviz::node::shape(*n, graphviz::shape_t::doublecircle);
                    graphviz::node::fill_color(*n, graphviz::color_t::aqua);
                }
            }

            if (node->parent) {
                auto e = make_edge(g, id, parent_id);
                graphviz::edge::color(*e, graphviz::color_t::red);
                graphviz::edge::style(*e, graphviz::edge_style_t::dashed);
            }

            if (node->lhs) {
                auto lhs_id = dump_node(tree, g, node->lhs, id);
                make_edge(g, id, lhs_id, "L"_ss);
            } else {
                make_nil_node(g, id, "L"_ss);
            }

            if (node->rhs) {
                auto rhs_id = dump_node(tree, g, node->rhs, id);
                make_edge(g, id, rhs_id, "R"_ss);
            } else {
                make_nil_node(g, id, "R"_ss);
            }

            return id;
        }

        inline u0 make_nil_node(graphviz::graph_t& g,
                                u32 id, str::slice_t label) {
            auto nil = graphviz::graph::make_node(g);
            graphviz::node::shape(*nil, graphviz::shape_t::point);
            make_edge(g, id, nil->id, label);
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        u0 dump_dot(T& tree, const String_Concept auto& name) {
            if (!tree.root)
                return;

            graphviz::graph_t g{};
            graphviz::graph::init(g,
                                  graphviz::graph_type_t::directed,
                                  name,
                                  {},
                                  tree.alloc);
            defer(graphviz::graph::free(g));
            graphviz::graph::font_size(g, 8.0f);

            dump_node(tree, g, tree.root);

            path_t p{};
            path::init(p, tree.alloc);
            path::set(p, format::format("{}.dot", name));

            buf_t buf{};
            buf.mode = buf_mode_t::alloc;
            buf::init(buf);

            defer(
                buf::free(buf);
                path::free(p);
                 );

            graphviz::graph::serialize(g, buf);
            buf::save(buf, p);
        }

        inline graphviz::edge_t* make_edge(graphviz::graph_t& g,
                                           u32 first,
                                           u32 second,
                                           str::slice_t label) {
            auto e = graphviz::graph::make_edge(g);
            e->first  = graphviz::graph::node_ref(&g, first);
            e->second = graphviz::graph::node_ref(&g, second);
            if (!slice::empty(label))
                graphviz::edge::label(*e, label);
            graphviz::edge::arrow_size(*e, .5f);
            graphviz::edge::dir(*e, graphviz::dir_type_t::both);
            graphviz::edge::arrow_tail(*e, graphviz::arrow_type_t::dot);
            graphviz::edge::arrow_head(*e, graphviz::arrow_type_t::normal);
            return e;
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        const Node_Type find(const T& tree, const Value_Type& value) {
            auto p = tree.root;
            while (p != nullptr) {
                auto cmp = value <=> *p->value;
                if (cmp < 0)        p = p->lhs;
                else if (cmp > 0)   p = p->rhs;
                else return         p;
            }
            return nullptr;
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::_Node_Type>
        u0 print_tree_struct(const T& tree, const Node_Type node, u32 level) {
            if (level > 16) {
                format::print("[...]");
                return;
            }

            if (!node) {
                format::print("nil");
                return;
            }

            format::print("{}", *node->value);
            if (node->lhs || node->rhs) {
                format::print("(");
                print_tree_struct(tree, node->lhs, level + 1);
                if (node->rhs) {
                    format::print(",");
                    print_tree_struct(tree, node->rhs, level + 1);
                }
                format::print(")");
            }
        }

        u0 print_whole_tree(const Binary_Tree auto& tree, str::slice_t title) {
            format::print("{}: ", title);
            print_tree_struct(tree, tree.root, 0);
            format::print("\n");
        }

        namespace cursor {
            // insert

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* last(bin_tree_cursor_t<Tree_Type>& cursor,
                             Tree_Type* tree) {
                cursor.tree = tree;
                cursor.node = tree->root;
                if (cursor.node) {
                    while (cursor.node->rhs)
                        cursor.node = cursor.node->rhs;
                    return cursor.node->value;
                }
                return nullptr;
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* first(bin_tree_cursor_t<Tree_Type>& cursor,
                              Tree_Type* tree) {
                cursor.tree = tree;
                cursor.node = tree->root;
                if (cursor.node) {
                    while (cursor.node->lhs)
                        cursor.node = cursor.node->lhs;
                    return cursor.node->value;
                }
                return nullptr;
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            u0 replace(bin_tree_cursor_t<Tree_Type>& cursor,
                       const Value_Type& value) {
                *cursor.node->value = value;
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* find(bin_tree_cursor_t<Tree_Type>& cursor,
                             Tree_Type* tree,
                             const Value_Type& value) {
                s32 dir;
                cursor.tree = tree;
                for (auto p = tree->root; p; p = NODE_BRANCH(p, dir)) {
                    auto cmp = value <=> *p->value;
                    if (cmp == 0) {
                        cursor.node = p;
                        return p->value;
                    }
                    dir = cmp > 0;
                }
                cursor.node = nullptr;
                return nullptr;
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* next(bin_tree_cursor_t<Tree_Type>& cursor) {
                if (!cursor.node)
                    return first(cursor, cursor.tree);
                else if (!cursor.node->rhs) {
                    for (auto p = cursor.node, q = p->parent;;
                         p = q, q = q->parent) {
                        if (!q || p == q->lhs) {
                            cursor.node = q;
                            return cursor.node ? cursor.node->value : nullptr;
                        }
                    }
                } else {
                    cursor.node = cursor.node->rhs;
                    while (cursor.node->lhs)
                        cursor.node = cursor.node->lhs;
                    return cursor.node->value;
                }
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* prev(bin_tree_cursor_t<Tree_Type>& cursor) {
                if (!cursor.node)
                    return last(cursor, cursor.tree);
                else if (!cursor.node->lhs) {
                    for (auto p = cursor.node, q = p->parent;;
                         p = q, q = q->parent) {
                        if (!q || p == q->rhs) {
                            cursor.node = q;
                            return cursor.node ? cursor.node->value : nullptr;
                        }
                    }
                } else {
                    cursor.node = cursor.node->lhs;
                    while (cursor.node->rhs)
                        cursor.node = cursor.node->rhs;
                    return cursor.node->value;
                }
            }

            template <Binary_Tree Tree_Type,
                      typename Value_Type = typename Tree_Type::Value_Type>
            Value_Type* curr(bin_tree_cursor_t<Tree_Type>& cursor) {
                return cursor.node ? cursor.node->value : nullptr;
            }

            template <Binary_Tree Tree_Type>
            u0 init(bin_tree_cursor_t<Tree_Type>& cursor, Tree_Type* tree) {
                cursor.tree = tree;
                cursor.node = nullptr;
            }
        }
    }
}
