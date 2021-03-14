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

namespace basecode {
    struct bst_node_t final {
        u32                     link[2];
        u32                     parent;
        u32                     value;
        u32                     id;
    };
    static_assert(sizeof(bst_node_t) <= 24, "bst_node_t is now larger than 24 bytes!");

    using bst_node_array_t      = array_t<bst_node_t>;
    using bst_node_stack_t      = stack_t<u32>;

    template <typename T>
    concept Binary_Tree = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Value_Array;
        typename                T::Value_Type_Ptr;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.nodes}               -> same_as<bst_node_array_t>;
        {t.values}              -> same_as<array_t<typename T::Value_Type>>;
        {t.free.nodes}          -> same_as<bst_node_stack_t>;
        {t.free.values}         -> same_as<bst_node_stack_t>;
        {t.size}                -> same_as<u32>;
        {t.root}                -> same_as<u32>;
    };

    template <typename T>
    struct bst_t final {
        using Value_Type        = T;
        using Value_Array       = array_t<Value_Type>;
        using Value_Type_Ptr    = std::remove_pointer_t<Value_Type>*;

        alloc_t*                alloc;
        bst_node_array_t        nodes;
        Value_Array             values;
        struct {
            bst_node_stack_t    nodes;
            bst_node_stack_t    values;
        }                       free;
        u32                     size;
        u32                     root;
    };
    static_assert(sizeof(bst_t<u32>) <= 116, "bst_t<u32> is now larger than 116 bytes!");

    namespace bst {
        inline bst_node_t* root(Binary_Tree auto& tree);

        template <typename T> requires Binary_Tree<T>
        b8 remove(T& tree, const typename T::Value_Type& value);

        inline const bst_node_t* root(const Binary_Tree auto& tree);

        inline bst_node_t* get_node(Binary_Tree auto& tree, u32 id);

        inline const bst_node_t* get_node(const Binary_Tree auto& tree, u32 id);

        inline bst_node_t* left(Binary_Tree auto& tree, const bst_node_t* node);

        inline bst_node_t* right(Binary_Tree auto& tree, const bst_node_t* node);

        template <typename T> requires Binary_Tree<T>
        inline bool get_value(T& tree, bst_node_t* node, typename T::Value_Type& value);

        inline const bst_node_t* left(const Binary_Tree auto& tree, const bst_node_t* node);

        inline const bst_node_t* right(const Binary_Tree auto& tree, const bst_node_t* node);

        template <typename T> requires Binary_Tree<T>
        inline bool get_value(const T& tree, const bst_node_t* node, typename T::Value_Type& value);

        inline u0 print_tree_struct(const bst_t<u32>& tree, const bst_node_t* node, u32 level) {
            if (level > 16) {
                format::print("[...]");
                return;
            }

            if (!node) {
                format::print("nil");
                return;
            }

            u32 value{};
            if (!bst::get_value(tree, node, value))
                return;

            format::print("{}", value);
            if (node->link[0] || node->link[1]) {
                format::print("(");
                print_tree_struct(tree, bst::get_node(tree, node->link[0]), level + 1);
                if (node->link[1]) {
                    format::print(",");
                    print_tree_struct(tree, bst::get_node(tree, node->link[1]), level + 1);
                }
                format::print(")");
            }
        }

        inline u0 print_whole_tree(const bst_t<u32>& tree, str::slice_t title) {
            format::print("{}: ", title);
            print_tree_struct(tree, bst::root(tree), 0);
            format::print("\n");
        }

        inline static u0 compress(Binary_Tree auto& tree, bst_node_t* node, u32 count) {
            if (tree.size == 0 || !node || count > tree.size) return;
            while (count--) {
                auto red   = get_node(tree, node->link[0]);
                auto black = get_node(tree, red->link[0]);
                node->link[0]  = black->id;
                red->link[0]   = black->link[1];
                black->link[1] = red->id;
                node = black;
            }
        }

        inline static u0 tree_to_vine(Binary_Tree auto& tree) {
            bst_node_t* p = root(tree);
            bst_node_t* q = p;

            while (p) {
                if (!p->link[1]) {
                    q = p;
                    p = get_node(tree, p->link[0]);
                } else {
                    auto r = get_node(tree, p->link[1]);
                    p->link[1] = r->link[0];
                    r->link[0] = p->id;
                    p = r;
                    q->link[0] = r->id;
                }
            }
        }

        inline static u0 vine_to_tree(Binary_Tree auto& tree) {
            auto leaves = tree.size + 1;
            for (;;) {
                auto next = leaves & (leaves - 1);
                if (!next) break;
                leaves = next;
            }
            leaves = tree.size + 1 - leaves;
            compress(tree, root(tree), leaves);
            auto vine = tree.size - leaves;
            auto height = 1 + (leaves > 0);
            while (vine > 1) {
                compress(tree, root(tree), vine / 2);
                vine /= 2;
                height++;
            }
        }

        inline static u0 update_parents(Binary_Tree auto& tree) {
            if (tree.size == 0)
                return;

            auto r = root(tree);
            r->parent = 0;

            bst_node_t* p;

            for (p = r;; p = get_node(tree, p->link[1])) {
                for (; p->link[0]; p = get_node(tree, p->link[0])) {
                    auto temp = get_node(tree, p->link[0]);
                    temp->parent = p->id;
                }

                for (; !p->link[1]; p = get_node(tree, p->parent)) {
                    for (;;) {
                        if (!p->parent)
                            return;
                        auto temp = get_node(tree, p->parent);
                        if (p->id == temp->link[0])
                            break;
                        p = get_node(tree, p->parent);
                    }
                }

                auto temp = get_node(tree, p->link[1]);
                temp->parent = p->id;
            }
        }

        u0 free(Binary_Tree auto& tree) {
            array::free(tree.nodes);
            array::free(tree.values);
            stack::free(tree.free.nodes);
            stack::free(tree.free.values);
            tree.size = {};
        }

        u0 reset(Binary_Tree auto& tree) {
            array::reset(tree.nodes);
            array::reset(tree.values);
            stack::reset(tree.free.nodes);
            stack::reset(tree.free.values);
            tree.size = {};
            tree.root = {};
        }

        u0 balance(Binary_Tree auto& tree) {
            if (tree.size == 0) return;
            tree_to_vine(tree);
            vine_to_tree(tree);
            update_parents(tree);
        }

        inline u32 size(const Binary_Tree auto& tree) {
            return tree.size;
        }

        inline b8 empty(const Binary_Tree auto& tree) {
            return tree.size == 0;
        }

        inline bst_node_t* root(Binary_Tree auto& tree) {
            return tree.root == 0 ? nullptr : &tree.nodes[tree.root - 1];
        }

        inline u32 capacity(const Binary_Tree auto& tree) {
            return tree.nodes.capacity;
        }

        u0 reserve(Binary_Tree auto& tree, u32 new_capacity) {
            array::reserve(tree.nodes, new_capacity);
            array::reserve(tree.values, new_capacity);
        }

        template <typename T> requires Binary_Tree<T>
        b8 remove(T& tree, const typename T::Value_Type& value) {
            using Value_Type = typename T::Value_Type;

            bst_node_t* p;
            bst_node_t* q;
            s32         dir;

            if (tree.size == 0)
                return false;

            p = root(tree);
            for (;;) {
                Value_Type v{};
                if (!get_value(tree, p, v))
                    return false;
                auto cmp = value <=> v;
                if (cmp == 0)
                    break;
                dir = cmp > 0;
                p   = get_node(tree, p->link[dir]);
                if (!p)
                    return false;
            }

            q = get_node(tree, p->parent);
            if (!q) {
                q   = root(tree);
                dir = 0;
            }

            if (!p->link[1]) {
                q->link[dir] = p->link[0];
                if (q->link[dir]) {
                    auto temp = get_node(tree, q->link[dir]);
                    temp->parent = p->parent;
                }
            } else {
                auto r = get_node(tree, p->link[1]);
                if (!r->link[0]) {
                    r->link[0]   = p->link[0];
                    q->link[dir] = r->id;
                    r->parent    = p->parent;
                    if (r->link[0]) {
                        auto temp    = get_node(tree, r->link[0]);
                        temp->parent = r->id;
                    }
                } else {
                    auto s = get_node(tree, r->link[0]);
                    while (s->link[0])
                        s = get_node(tree, s->link[0]);
                    r = get_node(tree, s->parent);
                    r->link[0] = s->link[1];
                    s->link[0] = p->link[0];
                    s->link[1] = p->link[1];
                    q->link[dir] = s->id;
                    if (s->link[0]) {
                        auto temp = get_node(tree, s->link[0]);
                        temp->parent = s->id;
                    }
                    {
                        auto temp = get_node(tree, s->link[1]);
                        temp->parent = s->id;
                    }
                    s->parent = p->parent;
                    if (r->link[0]) {
                        auto temp = get_node(tree, r->link[0]);
                        temp->parent = r->id;
                    }
                }
            }

            stack::push(tree.free.nodes, p->id);
            stack::push(tree.free.values, p->value);
            p->value  = 0;
            p->parent = 0;
            p->link[0] = p->link[1] = 0;
            if (p->id == tree.root)
                tree.root = 0;
            tree.size--;

            return true;
        }

        inline bst_node_t* get_node(Binary_Tree auto& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        inline const bst_node_t* root(const Binary_Tree auto& tree) {
            return tree.root == 0 ? nullptr : &tree.nodes[tree.root - 1];
        }

        template <typename T> requires Binary_Tree<T>
        bst_node_t* insert(T& tree, const typename T::Value_Type& value) {
            using Value_Type = typename T::Value_Type;

            bst_node_t* node;
            bst_node_t* p;
            bst_node_t* q;
            s32         dir{};

            for (q = nullptr, p = root(tree);
                 p != nullptr;
                 q = p, p = get_node(tree, p->link[dir])) {
                Value_Type v{};
                if (!get_value(tree, p, v))
                    return nullptr;
                auto cmp = value <=> v;
                if (cmp == 0)
                    return p;
                dir = cmp > 0;
            }

            if (!stack::empty(tree.free.nodes)) {
                node = get_node(tree, stack::pop(tree.free.nodes));
            } else {
                node = &array::append(tree.nodes);
                node->id = tree.nodes.size;
            }
            node->value   = 0;
            node->parent  = 0;
            node->link[0] = node->link[1] = 0;

            if (q) {
                node->parent = q->id;
                q->link[dir] = node->id;
            } else {
                tree.root = node->id;
            }

            if (!stack::empty(tree.free.values)) {
                node->value = stack::pop(tree.free.values);
                tree.values[node->value - 1] = value;
            } else {
                array::append(tree.values, value);
                node->value = tree.values.size;
            }

            tree.size++;

            return node;
        }

        u0 init(Binary_Tree auto& tree, alloc_t* alloc = context::top()->alloc) {
            tree.size  = {};
            tree.root  = {};
            tree.alloc = alloc;
            array::init(tree.nodes, tree.alloc);
            array::init(tree.values, tree.alloc);
            stack::init(tree.free.nodes, tree.alloc);
            stack::init(tree.free.values, tree.alloc);
        }

        inline const bst_node_t* get_node(const Binary_Tree auto& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        inline bst_node_t* left(Binary_Tree auto& tree, const bst_node_t* node) {
            return get_node(tree, node->link[0]);
        }

        inline bst_node_t* right(Binary_Tree auto& tree, const bst_node_t* node) {
            return get_node(tree, node->link[1]);
        }

        template <typename T> requires Binary_Tree<T>
        const bst_node_t* find(const T& tree, const typename T::Value_Type& value) {
            using Value_Type = typename T::Value_Type;

            const bst_node_t* p = root(tree);
            while (p != nullptr) {
                Value_Type v{};
                if (!get_value(tree, p, v))
                    break;
                auto cmp = value <=> v;
                if (cmp < 0)        p = left(tree, p);
                else if (cmp > 0)   p = right(tree, p);
                else return         p;
            }
            return nullptr;
        }

        template <typename T> requires Binary_Tree<T>
        inline bool get_value(T& tree, bst_node_t* node, typename T::Value_Type& value) {
            if (!node || node->value == 0 || node->value > tree.values.size)
                return false;
            value = tree.values[node->value - 1];
            return true;
        }

        inline const bst_node_t* left(const Binary_Tree auto& tree, const bst_node_t* node) {
            return get_node(tree, node->link[0]);
        }

        inline const bst_node_t* right(const Binary_Tree auto& tree, const bst_node_t* node) {
            return get_node(tree, node->link[1]);
        }

        template <typename T> requires Binary_Tree<T>
        inline bool get_value(const T& tree, const bst_node_t* node, typename T::Value_Type& value) {
            if (!node || node->value == 0 || node->value > tree.values.size)
                return false;
            value = tree.values[node->value - 1];
            return true;
        }
    }
}
