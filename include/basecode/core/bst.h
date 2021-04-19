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

#include <basecode/core/bintree.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template <typename T>
    struct bst_t final {
        struct node_t;

        using Has_Color         = std::integral_constant<b8, false>;
        using Node_Type         = node_t*;
        using Value_Type        = T;
        static constexpr u32    Value_Type_Size    = sizeof(T);
        static constexpr u32    Value_Type_Align   = alignof(T);

        struct node_t final {
            node_t*             lhs;
            node_t*             rhs;
            node_t*             parent;
            Value_Type*         value;
        };
        static_assert(sizeof(node_t) <= 32, "bst<T>::node_t is now larger than 32 bytes!");

        static constexpr u32    Node_Type_Size     = sizeof(node_t);
        static constexpr u32    Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(bst_t<u32>) <= 40, "bst_t<u32> is now larger than 40 bytes!");

    namespace bst {
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
            auto q = (Node_Type) &tree.root;
            auto p = tree.root;

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
            s32 dir{};

            if (!tree.root)
                return false;

            auto p = tree.root;
            for (;;) {
                auto cmp = value <=> *p->value;
                if (cmp == 0)
                    break;
                dir = cmp > 0;
                p   = NODE_BRANCH(p, dir);
                if (!p)
                    return false;
            }

            auto q = p->parent;
            if (!q) {
                q   = Node_Type(&tree.root);
                dir = 0;
            }

            if (!p->rhs) {
                auto qb = NODE_BRANCH(q, dir);
                qb = p->lhs;
                if (qb)
                    qb->parent = p->parent;
            } else {
                auto r = p->rhs;
                if (!r->lhs) {
                    r->lhs = p->lhs;
                    NODE_BRANCH(q, dir) = r;
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
                    NODE_BRANCH(q, dir) = s;
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

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        Node_Type insert(T& tree, const Value_Type& value) {
            Node_Type n;
            Node_Type p;
            Node_Type q     {};
            s32       dir   {};

            for (q = nullptr, p = tree.root;
                 p != nullptr;
                 q = p, p = NODE_BRANCH(p,dir)) {
                auto cmp = value <=> *p->value;
                if (cmp == 0)
                    return p;
                dir = cmp > 0;
            }

            n = Node_Type(memory::alloc(tree.node_slab));
            n->parent = nullptr;
            n->lhs    = n->rhs = nullptr;

            if (q) {
                n->parent = q;
                NODE_BRANCH(q, dir) = n;
            } else {
                tree.root = n;
            }

            n->value = (Value_Type*) memory::alloc(tree.value_slab);
            *n->value = value;

            tree.size++;

            return n;
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
            node_cfg.num_pages = DEFAULT_NUM_PAGES;
            tree.node_slab = memory::system::make(alloc_type_t::slab, &node_cfg);

            slab_config_t value_cfg{};
            value_cfg.backing   = tree.alloc;
            value_cfg.buf_size  = bst_t<T>::Value_Type_Size;
            value_cfg.buf_align = bst_t<T>::Value_Type_Align;
            value_cfg.num_pages = DEFAULT_NUM_PAGES;
            tree.value_slab = memory::system::make(alloc_type_t::slab, &value_cfg);
        }
    }
}
