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

    template <typename T> struct bintree_t final {
        bintree_node_array_t        nodes;
        array_t<T>                  values;
        stack_t<u32>                free_nodes;
        stack_t<u32>                free_values;
    };

    namespace bintree {
        template <typename T> u0 free(bintree_t<T>& tree) {
            array::free(tree.nodes);
            array::free(tree.values);
            stack::free(tree.free_nodes);
            stack::free(tree.free_values);
        }

        template <typename T> u0 clear(bintree_t<T>& tree) {
            free(tree);
        }

        template <typename T> u0 reset(bintree_t<T>& tree) {
            array::reset(tree.nodes);
            array::reset(tree.values);
            stack::reset(tree.free_nodes);
            stack::reset(tree.free_values);
        }

        template <typename T> u32 size(const bintree_t<T>& tree) {
            return tree.nodes.size;
        }

        template <typename T> b8 empty(const bintree_t<T>& tree) {
            return array::empty(tree.nodes);
        }

        template <typename T> u32 capacity(const bintree_t<T>& tree) {
            return tree.nodes.capacity;
        }

        template <typename T> bintree_node_t* root(bintree_t<T>& tree) {
            return tree.nodes.size == 0 ? nullptr : &tree.nodes[0];
        }

        template <typename T> u0 reserve(bintree_t<T>& tree, u32 new_capacity) {
            array::reserve(tree.nodes, new_capacity);
            array::reserve(tree.values, new_capacity);
        }

        template <typename T> decltype(auto) get_value(bintree_t<T>& tree, u32 id) {
            if constexpr (std::is_pointer_v<T>) {
                return id == 0 || id > tree.values.size ? nullptr : tree.values[id - 1];
            } else {
                return id == 0 || id > tree.values.size ? nullptr : &tree.values[id - 1];
            }
        }

        template <typename T> const bintree_node_t* root(const bintree_t<T>& tree) {
            return tree.nodes.size == 0 ? nullptr : &tree.nodes[0];
        }

        template <typename T> bintree_node_t* get_node(bintree_t<T>& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        template <typename T> decltype(auto) get_value(const bintree_t<T>& tree, u32 id) {
            if constexpr (std::is_pointer_v<T>) {
                return id == 0 || id > tree.values.size ? nullptr : tree.values[id - 1];
            } else {
                return id == 0 || id > tree.values.size ? nullptr : &tree.values[id - 1];
            }
        }

        template <typename T> const bintree_node_t* get_node(const bintree_t<T>& tree, u32 id) {
            return id == 0 || id > tree.nodes.size ? nullptr : &tree.nodes[id - 1];
        }

        template <typename T> u0 init(bintree_t<T>& tree, alloc_t* alloc = context::top()->alloc) {
            array::init(tree.nodes, alloc);
            array::init(tree.values, alloc);
            stack::init(tree.free_nodes, alloc);
            stack::init(tree.free_values, alloc);
        }

        template <typename T> bintree_node_t* left(bintree_t<T>& tree, const bintree_node_t* node) {
            return get_node(tree, node->left);
        }

        template <typename T> bintree_node_t* right(bintree_t<T>& tree, const bintree_node_t* node) {
            return get_node(tree, node->right);
        }

        template <typename T> u0 remove_node(bintree_t<T>& tree, bintree_node_t* parent_node) {
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

        template <typename T> decltype(auto) emplace_value(bintree_t<T>& tree, bintree_node_t* node) {
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

        template <typename T> u32 insert_value(bintree_t<T>& tree, bintree_node_t* node, const T& value) {
            if (!stack::empty(tree.free_values)) {
                node->value = stack::pop(tree.free_values);
                tree.values[node->value - 1] = value;
                return node->value;
            }
            array::append(tree.values, value);
            node->value = tree.values.size;
            return node->value;
        }

        template <typename T> const bintree_node_t* left(const bintree_t<T>& tree, const bintree_node_t* node) {
            return get_node(tree, node->left);
        }

        template <typename T> const bintree_node_t* right(const bintree_t<T>& tree, const bintree_node_t* node) {
            return get_node(tree, node->right);
        }

        template <typename T> u32 append_node(bintree_t<T>& tree, bintree_node_t** node, u32 left = 0, u32 right = 0, u32 value = 0) {
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
    }
}
