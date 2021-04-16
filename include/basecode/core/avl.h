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

#include <basecode/core/bintree.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template <typename T>
    struct avl_t final {
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
            s8                  balance;
        };
        static_assert(sizeof(node_t) <= 40, "avl<T>::node_t is now larger than 40 bytes!");

        static constexpr u32    Node_Type_Size     = sizeof(node_t);
        static constexpr u32    Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(avl_t<u32>) <= 40, "avl_t<u32> is now larger than 40 bytes!");

    namespace avl {
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
                    if (r->lhs)
                        r->lhs->parent = r;
                    r->balance = p->balance;
                    q   = r;
                    dir = 1;
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
                    s->parent      = p->parent;
                    if (r->lhs)
                        r->lhs->parent = r;
                    s->balance = p->balance;
                    q   = r;
                    dir = 0;
                }
            }
            memory::free(tree.value_slab, p->value);
            p->lhs     = p->rhs = nullptr;
            p->parent  = nullptr;
            p->value   = nullptr;
            p->balance = 0;
            memory::free(tree.node_slab, p);

            while (q != Node_Type(&tree.root)) {
                auto y = q;

                if (y->parent)
                    q = y->parent;
                else
                    q = Node_Type(&tree.root);

                if (dir == 0) {
                    dir = q->lhs != y;
                    y->balance++;
                    if (y->balance == 1)
                        break;
                    else if (y->balance == 2) {
                        auto x = y->rhs;
                        if (x->balance == -1) {
                            Node_Type w;

                            assert (x->balance == -1);
                            w = x->lhs;
                            x->lhs = w->lhs;
                            w->rhs = x;
                            y->rhs = w->lhs;
                            w->lhs = y;
                            if (w->balance == 1)
                                x->balance = 0, y->balance = -1;
                            else if (w->balance == 0)
                                x->balance = y->balance = 0;
                            else
                                x->balance = 1, y->balance = 0;
                            w->balance = 0;
                            w->parent  = y->parent;
                            x->parent  = y->parent = w;
                            if (x->lhs)
                                x->lhs->parent = x;
                            if (y->lhs)
                                y->rhs->pavl_parent = y;
                            NODE_BRANCH(q, dir) = w;
                        } else {
                            y->rhs    = x->lhs;
                            x->lhs    = y;
                            x->parent = y->parent;
                            y->parent = x;
                            if (y->rhs)
                                y->rhs->parent = y;
                            NODE_BRANCH(q, dir) = x;
                            if (x->balance == 0) {
                                x->balance = -1;
                                y->balance = 1;
                                break;
                            } else {
                                x->balance = y->balance = 0;
                                y = x;
                            }
                        }
                    }
                } else {
                    dir = q->lhs != y;
                    y->balance--;
                    if (y->balance == -1)
                        break;
                    else if (y->balance == -2) {
                        auto x = y->lhs;
                        if (x->balance == 1) {
                            assert (x->balance == 1);
                            auto w = x->rhs;
                            x->rhs = w->lhs;
                            w->lhs = x;
                            y->lhs = w->rhs;
                            w->rhs = y;
                            if (w->balance == -1)
                                x->balance = 0, y->balance = 1;
                            else if (w->balance == 0)
                                x->balance = y->balance = 0;
                            else
                                x->balance = -1, y->balance = 0;
                            w->balance = 0;
                            w->parent  = y->parent;
                            x->parent  = y->parent = w;
                            if (x->rhs)
                                x->rhs->parent = x;
                            if (y->lhs)
                                y->lhs->parent = y;
                            NODE_BRANCH(q, dir) = w;
                        } else {
                            y->lhs    = x->rhs;
                            x->rhs    = y;
                            x->parent = y->pavl_parent;
                            y->parent = x;
                            if (y->lhs)
                                y->lhs->parent = y;
                            NODE_BRANCH(q, dir) = x;
                            if (x->balance == 0) {
                                x->balance = 1;
                                y->balance = -1;
                                break;
                            } else {
                                x->balance = y->balance = 0;
                                y = x;
                            }
                        }
                    }
                }
            }

            --tree.size;
            return true;
        }

        template <Binary_Tree T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        Node_Type insert(T& tree, const Value_Type& value) {
            Node_Type y = tree.root;
            Node_Type p;
            Node_Type q;
            Node_Type n;
            Node_Type w;
            s32       dir;

            for (q = {}, p = tree.root; p; q = p, p = NODE_BRANCH(p, dir)) {
                auto cmp = value <=> *p->value;
                if (cmp == 0)
                    return p;
                dir = cmp > 0;
                if (p->balance != 0)
                    y = p;
            }

            n = Node_Type(memory::alloc(tree.node_slab));
            n->lhs     = n->rhs = nullptr;
            n->parent  = q;
            n->balance = 0;

            n->value = (Value_Type*) memory::alloc(tree.value_slab);
            *n->value = value;

            if (q)
                NODE_BRANCH(q, dir) = n;
            else
                tree.root = n;

            ++tree.size;

            if (tree.root == n)
                return p;

            for (p = n; p != y; p = q) {
                q   = p->parent;
                dir = q->lhs != p;
                if (dir == 0)
                    q->balance--;
                else
                    q->balance++;
            }

            if (y->balance == -2) {
                auto x = y->lhs;
                if (x->balance == -1) {
                    w = x;
                    y->lhs     = x->rhs;
                    x->rhs     = y;
                    x->balance = y->balance = 0;
                    x->parent  = y->parent;
                    y->parent  = x;
                    if (y->lhs)
                        y->lhs->parent = y;
                } else {
                    assert (x->balance == 1);
                    w = x->rhs;
                    x->rhs = w->lhs;
                    w->lhs = x;
                    y->lhs = w->rhs;
                    w->rhs = y;
                    if (w->balance == -1)
                        x->balance = 0, y->balance = 1;
                    else if (w->balance == 0)
                        x->balance = y->balance = 0;
                    else
                        x->balance = -1, y->balance = 0;
                    w->balance = 0;
                    w->parent  = y->parent;
                    x->parent  = y->parent = w;
                    if (x->rhs)
                        x->rhs->parent = x;
                    if (y->lhs)
                        y->lhs->parent = y;
                }
            } else if (y->balance == 2) {
                auto x = y->rhs;
                if (x->balance == 1) {
                    w = x;
                    y->rhs     = x->lhs;
                    x->lhs     = y;
                    x->balance = y->balance = 0;
                    x->parent  = y->parent;
                    y->parent  = x;
                    if (y->rhs)
                        y->rhs->parent = y;
                } else {
                    assert (x->balance == -1);
                    w = x->lhs;
                    x->lhs = w->rhs;
                    w->rhs = x;
                    y->rhs = w->lhs;
                    w->lhs = y;
                    if (w->balance == 1)
                        x->balance = 0, y->balance = -1;
                    else if (w->balance == 0)
                        x->balance = y->balance = 0;
                    else
                        x->balance = 1, y->balance = 0;
                    w->balance = 0;
                    w->parent  = y->parent;
                    x->parent  = y->parent = w;
                    if (x->lhs)
                        x->lhs->parent = x;
                    if (y->rhs)
                        y->rhs->parent = y;
                }
            } else
                return n;

            if (w->parent)
                NODE_BRANCH(w->parent, y != w->parent->lhs) = w;
            else
                tree.root = w;

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
            node_cfg.buf_size  = avl_t<T>::Node_Type_Size;
            node_cfg.buf_align = avl_t<T>::Node_Type_Align;
            node_cfg.num_pages = DEFAULT_NUM_PAGES;
            tree.node_slab = memory::system::make(alloc_type_t::slab, &node_cfg);

            slab_config_t value_cfg{};
            value_cfg.backing   = tree.alloc;
            value_cfg.buf_size  = avl_t<T>::Value_Type_Size;
            value_cfg.buf_align = avl_t<T>::Value_Type_Align;
            value_cfg.num_pages = DEFAULT_NUM_PAGES;
            tree.value_slab = memory::system::make(alloc_type_t::slab, &value_cfg);
        }
    }
}
