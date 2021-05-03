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

namespace basecode::rbt {
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
        Node_Type f     {};
        s32       dir   {};

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
            NODE_BRANCH(q, dir) = p->lhs;
            if (NODE_BRANCH(q, dir))
                NODE_BRANCH(q, dir)->parent = p->parent;
            f = q;
        } else {
            rbt_color_t t;
            auto r = p->rhs;

            if (!r->lhs) {
                r->lhs = p->lhs;
                NODE_BRANCH(q, dir) = r;
                r->parent = p->parent;
                if (r->rhs)
                    r->lhs->parent = r;

                t = p->color;
                p->color = r->color;
                r->color = t;

                f   = r;
                dir = 1;
            } else {
                auto s = r->lhs;
                while (s->lhs)
                    s = s->lhs;
                r = s->parent;
                r->lhs   = s->rhs;
                s->lhs   = p->lhs;
                s->rhs   = p->rhs;
                NODE_BRANCH(q, dir) = s;
                if (s->lhs)
                    s->lhs->parent = s;
                s->rhs->parent     = s;
                s->parent          = p->parent;
                if (r->lhs)
                    r->lhs->parent = r;

                t = p->color;
                p->color = s->color;
                s->color = t;

                f   = r;
                dir = 0;
            }
        }

        if (p->color == rbt_color_t::black) {
            for (;;) {
                Node_Type t;

                auto x = NODE_BRANCH(f, dir);
                if (x && x->color == rbt_color_t::red) {
                    x->color = rbt_color_t::black;
                    break;
                }

                if (f == Node_Type(&tree.root))
                    break;

                auto g = f->parent;
                if (!g)
                    g = Node_Type(&tree.root);

                if (dir == 0) {
                    auto w = f->rhs;

                    if (w->color == rbt_color_t::red) {
                        w->color = rbt_color_t::black;
                        f->color = rbt_color_t::red;

                        f->rhs = w->lhs;
                        w->lhs = f;
                        NODE_BRANCH(g, g->lhs != f) = w;

                        w->parent = f->parent;
                        f->parent = w;

                        g = w;
                        w = f->rhs;

                        w->parent = f;
                    }

                    if ((!w->lhs || w->lhs->color == rbt_color_t::black)
                    &&  (!w->rhs || w->rhs->color == rbt_color_t::black)) {
                        w->color = rbt_color_t::red;
                    } else {
                        if (!w->rhs
                        ||  w->rhs->color == rbt_color_t::black) {
                            auto y = w->lhs;
                            y->color = rbt_color_t::black;
                            w->color = rbt_color_t::red;
                            w->lhs   = y->rhs;
                            y->rhs   = w;
                            if (w->lhs)
                                w->lhs->parent = w;
                            w = f->rhs = y;
                            w->rhs->parent = w;
                        }

                        w->color      = f->color;
                        f->color      = rbt_color_t::black;
                        w->rhs->color = rbt_color_t::black;

                        f->rhs = w->lhs;
                        w->lhs = f;
                        NODE_BRANCH(g, g->lhs != f) = w;

                        w->parent = f->parent;
                        f->parent = w;
                        if (f->rhs)
                            f->rhs->parent = f;
                        break;
                    }
                } else {
                    auto w = f->lhs;

                    if (w->color == rbt_color_t::red) {
                        w->color = rbt_color_t::black;
                        f->color = rbt_color_t::red;

                        f->lhs = w->rhs;
                        w->rhs = f;
                        NODE_BRANCH(g, g->lhs != f) = w;

                        w->parent = f->parent;
                        f->parent = w;

                        g = w;
                        w = f->lhs;

                        w->parent = f;
                    }

                    if ((!w->lhs || w->lhs->color == rbt_color_t::black)
                    &&  (!w->rhs || w->rhs->color == rbt_color_t::black)) {
                        w->color = rbt_color_t::red;
                    } else {
                        if (!w->lhs
                        || w->lhs->color == rbt_color_t::black) {
                            auto y = w->rhs;
                            y->color = rbt_color_t::black;
                            w->color = rbt_color_t::red;
                            w->rhs   = y->lhs;
                            y->lhs   = w;
                            if (w->rhs)
                                w->rhs->parent = w;
                            w = f->lhs = y;
                            w->lhs->parent = w;
                        }

                        w->color      = f->color;
                        f->color      = rbt_color_t::black;
                        w->lhs->color = rbt_color_t::black;

                        f->lhs = w->rhs;
                        w->rhs = f;
                        NODE_BRANCH(g, g->lhs != f) = w;

                        w->parent = f->parent;
                        f->parent = w;
                        if (f->lhs)
                            f->lhs->parent = f;
                        break;
                    }
                }

                t = f;
                f = f->parent;
                if (!f)
                    f = Node_Type(&tree.root);
                dir = f->lhs != t;
            }
        }

        memory::free(tree.value_slab, p->value);
        p->value  = nullptr;
        p->parent = nullptr;
        p->lhs    = p->rhs = nullptr;
        p->color  = rbt_color_t::none;
        memory::free(tree.node_slab, p);
        --tree.size;
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
        n->color  = rbt_color_t::red;

        if (q) {
            n->parent = q;
            NODE_BRANCH(q, dir) = n;
        } else {
            tree.root = n;
        }

        n->value = (Value_Type*) memory::alloc(tree.value_slab);
        *n->value = value;

        ++tree.size;

        q = n;
        for (;;) {
            auto f = q->parent;

            if (!f || f->color == rbt_color_t::black)
                break;

            auto g = f->parent;
            if (!g)
                break;

            if (g->lhs == f) {
                auto y = g->rhs;
                if (y && y->color == rbt_color_t::red) {
                    f->color = y->color = rbt_color_t::black;
                    g->color = rbt_color_t::red;
                    q = g;
                } else {
                    auto h = g->parent;
                    if (!h)
                        h = Node_Type(&tree.root);

                    if (f->rhs == q) {
                        f->rhs    = q->lhs;
                        q->lhs    = f;
                        g->lhs    = q;
                        f->parent = q;
                        if (f->rhs)
                            f->rhs->parent = f;
                        f = q;
                    }

                    g->color = rbt_color_t::red;
                    f->color = rbt_color_t::black;

                    g->lhs = f->rhs;
                    f->rhs = g;
                    NODE_BRANCH(h, h->lhs != g) = f;

                    f->parent = g->parent;
                    g->parent = f;
                    if (g->lhs)
                        g->lhs->parent = g;
                    break;
                }
            } else {
                auto y = g->lhs;
                if (y && y->color == rbt_color_t::red) {
                    f->color = y->color = rbt_color_t::black;
                    g->color = rbt_color_t::red;
                    q = g;
                } else {
                    auto h = g->parent;
                    if (!h)
                        h = Node_Type(&tree.root);

                    if (f->lhs == q) {
                        f->lhs    = q->rhs;
                        q->rhs    = f;
                        g->rhs    = q;
                        f->parent = q;
                        if (f->lhs)
                            f->lhs->parent = f;
                        f = q;
                    }

                    g->color = rbt_color_t::red;
                    f->color = rbt_color_t::black;

                    g->rhs   = f->lhs;
                    f->lhs   = g;
                    NODE_BRANCH(h, h->lhs != g) = f;

                    f->parent = g->parent;
                    g->parent = f;
                    if (g->rhs)
                        g->rhs->parent = g;
                    break;
                }
            }
        }

        tree.root->color = rbt_color_t::black;

        return n;
    }

    template <Binary_Tree T,
              typename Node_Type = typename T::Node_Type>
    u0 init(T& tree, alloc_t* alloc = context::top()->alloc.main) {
        tree.size  = {};
        tree.root  = {};
        tree.alloc = alloc;

        slab_config_t node_cfg{};
        node_cfg.name          = "rbt::node_slab";
        node_cfg.buf_size      = rbt_t<T>::Node_Type_Size;
        node_cfg.buf_align     = rbt_t<T>::Node_Type_Align;
        node_cfg.num_pages     = DEFAULT_NUM_PAGES;
        node_cfg.backing.alloc = tree.alloc;
        tree.node_slab = memory::system::make(&node_cfg);

        slab_config_t value_cfg{};
        value_cfg.name          = "rbt::value_slab";
        value_cfg.buf_size      = rbt_t<T>::Value_Type_Size;
        value_cfg.buf_align     = rbt_t<T>::Value_Type_Align;
        value_cfg.num_pages     = DEFAULT_NUM_PAGES;
        value_cfg.backing.alloc = tree.alloc;
        tree.value_slab = memory::system::make(&value_cfg);
    }
}
