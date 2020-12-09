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

namespace basecode {
    struct bintree_node_t final {
        u64                         free:    1;
        u64                         left:   21;
        u64                         right:  21;
        u64                         value:  21;
    };

    using bintree_node_array_t      = array_t<bintree_node_t>;
    using bintree_node_stack_t      = stack_t<u32>;

    template <typename T>
    concept Binary_Tree = requires(const T& t) {
        typename                    T::Value_Type;
        {t.alloc}                   -> same_as<alloc_t*>;
        {t.nodes}                   -> same_as<bintree_node_array_t>;
        {t.values}                  -> same_as<array_t<typename T::Value_Type>>;
        {t.free_nodes}              -> same_as<bintree_node_stack_t>;
        {t.free_values}             -> same_as<bintree_node_stack_t>;
    };

    template <typename T>
    struct bintree_t final {
        using Value_Type            = T;

        alloc_t*                    alloc;
        bintree_node_array_t        nodes;
        array_t<T>                  values;
        bintree_node_stack_t        free_nodes;
        bintree_node_stack_t        free_values;
    };

    namespace bintree {
        template <typename T> requires Binary_Tree<T>
        inline decltype(auto) get_value(T& tree, u32 id);

        template <typename T> requires Binary_Tree<T>
        inline decltype(auto) get_value(const T& tree, u32 id);

        inline const bintree_node_t* left(const Binary_Tree auto& tree,
                                          const bintree_node_t* node);

        inline const bintree_node_t* right(const Binary_Tree auto& tree,
                                           const bintree_node_t* node);

        inline bintree_node_t* get_node(Binary_Tree auto& tree, u32 id);

        u0 remove_node(Binary_Tree auto& tree, bintree_node_t* parent_node);

        inline const bintree_node_t* get_node(const Binary_Tree auto& tree, u32 id);

        inline bintree_node_t* left(Binary_Tree auto& tree, const bintree_node_t* node);

        inline bintree_node_t* right(Binary_Tree auto& tree, const bintree_node_t* node);

        u0 free(Binary_Tree auto& tree) {
            array::free(tree.nodes);
            array::free(tree.values);
            stack::free(tree.free_nodes);
            stack::free(tree.free_values);
        }

        u0 clear(Binary_Tree auto& tree) {
            free(tree);
        }

        u0 reset(Binary_Tree auto& tree) {
            array::reset(tree.nodes);
            array::reset(tree.values);
            stack::reset(tree.free_nodes);
            stack::reset(tree.free_values);
        }

        u32 append_node(Binary_Tree auto& tree,
                        bintree_node_t** node,
                        u32 left = 0,
                        u32 right = 0,
                        u32 value = 0) {
            if (!stack::empty(tree.free_nodes)) {
                auto id = stack::pop(tree.free_nodes);
                *node = get_node(tree, id);
                (*node)->free = {};
                return id;
            }
            auto new_node = &array::append(tree.nodes);
            new_node->free  = {};
            new_node->left  = left;
            new_node->right = right;
            new_node->value = value;
            *node = new_node;
            return tree.nodes.size;
        }

        inline u32 size(const Binary_Tree auto& tree) {
            return tree.nodes.size;
        }

        inline b8 empty(const Binary_Tree auto& tree) {
            return array::empty(tree.nodes);
        }

        template <typename T> requires Binary_Tree<T>
        inline decltype(auto) get_value(T& tree, u32 id) {
            if constexpr (std::is_pointer_v<typename T::Value_Type>) {
                return id == 0 || id > tree.values.size ? nullptr : tree.values[id - 1];
            } else {
                return id == 0 || id > tree.values.size ? nullptr : &tree.values[id - 1];
            }
        }

        inline u32 capacity(const Binary_Tree auto& tree) {
            return tree.nodes.capacity;
        }

        inline bintree_node_t* root(Binary_Tree auto& tree) {
            return tree.nodes.size == 0 ? nullptr : &tree.nodes[0];
        }

        u0 reserve(Binary_Tree auto& tree, u32 new_capacity) {
            array::reserve(tree.nodes, new_capacity);
            array::reserve(tree.values, new_capacity);
        }

        template <typename T> requires Binary_Tree<T>
        inline decltype(auto) get_value(const T& tree, u32 id) {
            if constexpr (std::is_pointer_v<typename T::Value_Type>) {
                return id == 0 || id > tree.values.size ? nullptr : tree.values[id - 1];
            } else {
                return id == 0 || id > tree.values.size ? nullptr : &tree.values[id - 1];
            }
        }

        template <typename T> requires Binary_Tree<T>
        decltype(auto) emplace_value(T& tree, bintree_node_t* node) {
            if (!stack::empty(tree.free_values)) {
                node->value = stack::pop(tree.free_values);
                return get_value(tree, node->value);
            }
            if constexpr (std::is_pointer_v<T>) {
                auto value = array::append(tree.values);
                node->value = tree.values.size;
                return value;
            } else {
                auto value = &array::append(tree.values);
                node->value = tree.values.size;
                return value;
            }
        }

        inline const bintree_node_t* left(const Binary_Tree auto& tree,
                                          const bintree_node_t* node) {
            return get_node(tree, node->left);
        }

        inline const bintree_node_t* right(const Binary_Tree auto& tree,
                                           const bintree_node_t* node) {
            return get_node(tree, node->right);
        }

        inline bintree_node_t* get_node(Binary_Tree auto& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        inline const bintree_node_t* root(const Binary_Tree auto& tree) {
            return tree.nodes.size == 0 ? nullptr : &tree.nodes[0];
        }

        u0 remove_node(Binary_Tree auto& tree, bintree_node_t* parent_node) {
            if (parent_node->free) return;
            if (parent_node->left) {
                stack::push(tree.free_nodes, (u32) parent_node->left);
                remove_node(tree, left(tree, parent_node));
            }
            if (parent_node->right) {
                stack::push(tree.free_nodes, (u32) parent_node->right);
                remove_node(tree, right(tree, parent_node));
            }
            if (parent_node->value) {
                stack::push(tree.free_values, (u32) parent_node->value);
            }
            parent_node->free  = true;
            parent_node->value = parent_node->left = parent_node->right = {};
        }

        u0 init(Binary_Tree auto& tree, alloc_t* alloc = context::top()->alloc) {
            tree.alloc = alloc;
            array::init(tree.nodes, tree.alloc);
            array::init(tree.values, tree.alloc);
            stack::init(tree.free_nodes, tree.alloc);
            stack::init(tree.free_values, tree.alloc);
        }

        inline const bintree_node_t* get_node(const Binary_Tree auto& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        inline bintree_node_t* left(Binary_Tree auto& tree, const bintree_node_t* node) {
            return get_node(tree, node->left);
        }

        inline bintree_node_t* right(Binary_Tree auto& tree, const bintree_node_t* node) {
            return get_node(tree, node->right);
        }

        template <typename T> requires Binary_Tree<T>
        u32 insert_value(T& tree, bintree_node_t* node, const typename T::Value_Type& value) {
            if (!stack::empty(tree.free_values)) {
                node->value = stack::pop(tree.free_values);
                tree.values[node->value - 1] = value;
                return node->value;
            }
            array::append(tree.values, value);
            node->value = tree.values.size;
            return node->value;
        }
    }
}
