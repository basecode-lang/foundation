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

#include <basecode/core/memory/meta.h>

namespace basecode::memory::meta {
    struct system_t final {
        bintree_t<alloc_t*>     tree;
        b8                      init{};
    };

    thread_local system_t       t_meta_system{};

    const bintree_t<alloc_t*>& tree() {
        return t_meta_system.tree;
    }

    static bintree_node_t* find_alloc_node(bintree_node_t* parent_node, alloc_t* alloc) {
        auto curr_node = parent_node;
        while (curr_node) {
            auto value = bintree::get_value(t_meta_system.tree, curr_node->value);
            if (value == alloc) break;
            bintree_node_t* alloc_node{};
            if (curr_node->left) {
                alloc_node = find_alloc_node(bintree::left(t_meta_system.tree, curr_node), alloc);
            }
            if (!alloc_node && curr_node->right) {
                alloc_node = find_alloc_node(bintree::right(t_meta_system.tree, curr_node), alloc);
            }
            curr_node  = alloc_node;
        }
        return curr_node;
    }

    static bintree_node_t* find_left_leaf(bintree_node_t* parent_node) {
        auto curr_node = parent_node;
        while (curr_node && curr_node->left)
            curr_node = bintree::left(t_meta_system.tree, curr_node);
        return curr_node;
    }

    static bintree_node_t* find_right_leaf(bintree_node_t* parent_node) {
        auto curr_node = parent_node;
        while (curr_node && curr_node->right)
            curr_node = bintree::right(t_meta_system.tree, curr_node);
        return curr_node;
    }

    u0 fini() {
        bintree::free(t_meta_system.tree);
        t_meta_system.init = {};
    }

    u0 init(alloc_t* alloc) {
        bintree::init(t_meta_system.tree, alloc);
        bintree_node_t* root{};
        bintree::append_node(t_meta_system.tree, &root);
        t_meta_system.init = true;
        track(alloc);
    }

    u0 track(alloc_t* alloc) {
        if (!t_meta_system.init) return;
        const auto tree_size = bintree::size(t_meta_system.tree);
        const auto tree_cap  = bintree::capacity(t_meta_system.tree);
        if (tree_size + 4 > tree_cap)
            bintree::reserve(t_meta_system.tree, tree_size * 2 + 8);
        bintree_node_t* new_node{};
        bintree_node_t* alloc_node;
        const b8 is_child = alloc->backing != nullptr;
        auto root_node = bintree::root(t_meta_system.tree);
        if (is_child) {
            auto parent_node = find_alloc_node(root_node, alloc->backing);
            if (find_alloc_node(parent_node, alloc))
                return;
            alloc_node = find_right_leaf(parent_node);
            if (!alloc_node->right && !alloc_node->left) {
                alloc_node->right = bintree::append_node(t_meta_system.tree, &new_node);
                alloc_node = new_node;
                alloc_node->left  = bintree::append_node(t_meta_system.tree, &new_node);
                alloc_node = new_node;
            }
        } else {
            alloc_node = find_alloc_node(root_node, alloc);
        }
        alloc_node = find_left_leaf(alloc_node ? alloc_node : root_node);
        if (alloc_node->value) {
            alloc_node->left = bintree::append_node(t_meta_system.tree, &new_node);
            alloc_node = new_node;
        }
        bintree::insert_value(t_meta_system.tree, alloc_node, alloc);
    }

    u0 untrack(alloc_t* alloc) {
        if (!t_meta_system.init) return;
        auto root_node = bintree::root(t_meta_system.tree);
        auto alloc_node = find_alloc_node(root_node, alloc);
        if (alloc_node) {
            bintree::remove_node(t_meta_system.tree, alloc_node);
        }
    }
}

