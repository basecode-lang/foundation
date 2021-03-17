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

#pragma once

#include <basecode/core/array.h>
#include <basecode/core/stack.h>
#include <basecode/core/format.h>
#include <basecode/core/graphviz/gv.h>
#include <basecode/core/memory/system/slab.h>

#define BST_BRANCH(n, d)   ((d) == 1 ? (n)->rhs : (n)->lhs)

namespace basecode {
    template <typename T>
    concept Binary_Tree = requires(const T& t) {
        typename                T::Node_Type;
        typename                T::Value_Type;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.node_slab}           -> same_as<alloc_t*>;
        {t.value_slab}          -> same_as<alloc_t*>;
        {t.root}                -> same_as<typename T::Node_Type>;
        {t.size}                -> same_as<u32>;
    };

    template <typename T>
    struct bst_t final {
        struct node_t;

        using Node_Type         = node_t*;
        using Value_Type        = T*;
        static constexpr u32 Value_Type_Size    = sizeof(T);
        static constexpr u32 Value_Type_Align   = alignof(T);

        struct node_t final {
            node_t*             lhs;
            node_t*             rhs;
            node_t*             parent;
            Value_Type          value;
        };
        static_assert(sizeof(node_t) <= 32, "node_t is now larger than 32 bytes!");

        static constexpr u32 Node_Type_Size     = sizeof(node_t);
        static constexpr u32 Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(bst_t<u32>) <= 40, "bst_t<u32> is now larger than 40 bytes!");

    namespace bst {
        inline graphviz::edge_t* make_edge(graphviz::graph_t& g,
                                           u32 first,
                                           u32 second,
                                           str::slice_t label = {}) {
            auto e = graphviz::graph::make_edge(g);
            e->first  = first;
            e->second = second;
            if (!slice::empty(label))
                graphviz::edge::label(*e, label);
            graphviz::edge::arrow_size(*e, .5f);
            graphviz::edge::dir(*e, graphviz::dir_type_t::both);
            graphviz::edge::arrow_tail(*e, graphviz::arrow_type_t::dot);
            graphviz::edge::arrow_head(*e, graphviz::arrow_type_t::normal);
            return e;
        }

        inline u0 make_nil_node(graphviz::graph_t& g, u32 id, str::slice_t label) {
            auto nil = graphviz::graph::make_node(g);
            graphviz::node::shape(*nil, graphviz::shape_t::point);
            make_edge(g, id, nil->id, label);
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        u32 dump_node(T& tree, graphviz::graph_t& g, Node_Type node, u32 parent_id = {}) {
            auto n = graphviz::graph::make_node(g);
            u32 id = n->id;
            graphviz::node::label(*n,
                                  format::format("{}{}",
                                                 tree.root == node ? "ROOT\n"_ss : ""_ss,
                                                 *node->value));

            if (tree.root == node) {
                graphviz::node::shape(*n, graphviz::shape_t::doublecircle);
                graphviz::node::style(*n, graphviz::node_style_t::filled);
                graphviz::node::fill_color(*n, graphviz::color_t::aqua);
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
            path::set(p, format::format("C:/temp/{}.dot", name));

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

            u32 value = *node->value;
            format::print("{}", value);
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

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type>
        u0 compress(T& tree, Node_Type root, u32 count) {
            if (!root)
                return;
            while (count--) {
                auto red    = root->lhs;
                auto black  = red->lhs;
                root->lhs   = black;
                red->lhs    = black->rhs;
                black->rhs  = red;
                root        = black;
            }
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type>
        u0 tree_to_vine(T& tree) {
            Node_Type q;
            Node_Type p;

            q = (Node_Type) &tree.root;
            p = tree.root;

            while (p != nullptr) {
                if (!p->rhs) {
                    q = p;
                    p = p->lhs;
                } else {
                    auto r = p->rhs;
                    p->rhs = r->lhs;
                    r->lhs = p;
                    p = r;
                    q->lhs = r;
                }
            }
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type>
        u0 vine_to_tree(T& tree) {
            auto leaves = tree.size + 1;
            for (;;) {
                auto next = leaves & (leaves - 1);
                if (!next) break;
                leaves = next;
            }
            leaves = tree.size + 1 - leaves;
            compress(tree, Node_Type(&tree.root), leaves);
            auto vine = tree.size - leaves;
            auto height = 1 + (leaves > 0);
            while (vine > 1) {
                compress(tree, Node_Type(&tree.root), vine / 2);
                vine /= 2;
                height++;
            }
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type>
        u0 update_parents(T& tree) {
            if (!tree.root)
                return;

            Node_Type p;
            tree.root->parent = nullptr;

            for (p = tree.root;; p = p->rhs) {
                for (; p->lhs; p = p->lhs)
                    p->lhs->parent = p;

                for (; !p->rhs; p = p->parent) {
                    for (;;) {
                        if (!p->parent)
                            return;
                        if (p == p->parent->lhs)
                            break;
                        p = p->parent;
                    }
                }
                p->rhs->parent = p;
            }
        }

        u0 free(Binary_Tree auto& tree) {
            memory::system::free(tree.node_slab);
            memory::system::free(tree.value_slab);
            tree.size = {};
            tree.root = {};
        }

        u0 reset(Binary_Tree auto& tree) {
            memory::slab::reset(tree.node_slab);
            memory::slab::reset(tree.value_slab);
            tree.size = {};
            tree.root = {};
        }

        u0 balance(Binary_Tree auto& tree) {
            if (tree.size == 0) return;
            tree_to_vine(tree);
            vine_to_tree(tree);
            update_parents(tree);
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 remove(T& tree, const Value_Type& value) {
            Node_Type p     {};
            Node_Type q     {};
            s32       dir   {};

            if (!tree.root)
                return false;

            p = tree.root;
            for (;;) {
                auto cmp = value <=> *p->value;
                if (cmp == 0)
                    break;
                dir = cmp > 0;
                p   = BST_BRANCH(p, dir);
                if (!p)
                    return false;
            }

            q = p->parent;
            if (!q) {
                q   = Node_Type(&tree.root);
                dir = 0;
            }

            if (!p->rhs) {
                auto qb = BST_BRANCH(q, dir);
                qb = p->lhs;
                if (qb)
                    qb->parent = p->parent;
            } else {
                auto r = p->rhs;
                if (!r->lhs) {
                    r->lhs = p->lhs;
                    BST_BRANCH(q, dir) = r;
                    r->parent = p->parent;
                    if (r->lhs) {
                        r->lhs->parent = r;
                    }
                } else {
                    auto s = r->lhs;
                    while (s->lhs)
                        s = s->lhs;
                    r = s->parent;
                    r->lhs = s->rhs;
                    s->lhs = p->lhs;
                    s->rhs = p->rhs;
                    BST_BRANCH(q, dir) = s;
                    if (s->lhs)
                        s->lhs->parent = s;
                    s->rhs->parent = s;
                    s->parent = p->parent;
                    if (r->lhs)
                        r->lhs->parent = r;
                }
            }

            memory::free(tree.value_slab, p->value);
            p->value      = nullptr;

            p->parent     = nullptr;
            p->lhs        = p->rhs = nullptr;
            if (p == tree.root)
                tree.root = nullptr;
            memory::free(tree.node_slab, p);
            tree.size--;

            return true;
        }

        inline u32 size(const Binary_Tree auto& tree) {
            return tree.size;
        }

        inline b8 empty(const Binary_Tree auto& tree) {
            return tree.size == 0;
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        Node_Type insert(T& tree, const Value_Type& value) {
            Node_Type node;
            Node_Type p;
            Node_Type q{};
            s32       dir{};

            for (q = nullptr, p = tree.root;
                 p != nullptr;
                 q = p, p = BST_BRANCH(p,dir)) {
                auto cmp = value <=> *p->value;
                if (cmp == 0)
                    return p;
                dir = cmp > 0;
            }

            node = Node_Type(memory::alloc(tree.node_slab));
            node->parent  = nullptr;
            node->lhs     = node->rhs = nullptr;

            if (q) {
                node->parent = q;
                BST_BRANCH(q, dir) = node;
            } else {
                tree.root = node;
            }

            node->value = (Value_Type*) memory::alloc(tree.value_slab);
            *node->value = value;

            tree.size++;

            return node;
        }

        template <Binary_Tree T,
            typename Node_Type = typename T::Node_Type>
        u0 init(T& tree, alloc_t* alloc = context::top()->alloc) {
            tree.size  = {};
            tree.root  = {};
            tree.alloc = alloc;

            slab_config_t node_cfg{};
            node_cfg.backing   = tree.alloc;
            node_cfg.buf_size  = bst_t<T>::Node_Type_Size;
            node_cfg.buf_align = bst_t<T>::Node_Type_Align;
            tree.node_slab = memory::system::make(alloc_type_t::slab, &node_cfg);

            slab_config_t value_cfg{};
            value_cfg.backing   = tree.alloc;
            value_cfg.buf_size  = bst_t<T>::Value_Type_Size;
            value_cfg.buf_align = bst_t<T>::Value_Type_Align;
            tree.value_slab = memory::system::make(alloc_type_t::slab, &value_cfg);
        }

        template <Binary_Tree T,
            typename Node_Type = typename T::Node_Type,
            typename Value_Type = typename T::Value_Type>
        const Node_Type find(const T& tree, const Value_Type& value) {
            Node_Type p = tree.root;
            while (p != nullptr) {
                auto cmp = value <=> *p->value;
                if (cmp < 0)        p = p->lhs;
                else if (cmp > 0)   p = p->rhs;
                else return         p;
            }
            return nullptr;
        }
    }
}
