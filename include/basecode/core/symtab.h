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

#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/stack.h>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/memory.h>
#include <basecode/core/assoc_array.h>

namespace basecode {
    constexpr u8 empty = 0b00;
    constexpr u8 used  = 0b01;
    constexpr u8 leaf  = 0b10;

    struct symtab_node_t final {
        u64                             type:   2;
        u64                             sym:    8;
        u64                             next:   18;
        u64                             child:  18;
        u64                             value:  18;
    };

    template <typename V> struct symtab_t final {
        alloc_t*                        alloc;
        array_t<symtab_node_t>          nodes;
        array_t<V>                      values;
        u32                             size;
    };
    static_assert(sizeof(symtab_t<s32>) <= 64, "symtab_t<V> is now larger than 64 bytes!");

    struct find_level_result_t final {
        symtab_node_t*                  start_node;
        symtab_node_t*                  match_node;
        symtab_node_t*                  avail_node;
        u32                             node_id;
    };

    namespace symtab {
        template <typename V> u0 free(symtab_t<V>& symtab);
        template <typename V> u0 clear(symtab_t<V>& symtab);
        template <typename V> u0 reset(symtab_t<V>& symtab);
        template <typename V> u0 reserve(symtab_t<V>& symtab, u32 capacity);
        template <typename V> symtab_node_t* get_node(symtab_t<V>& symtab, u32 id);
        template <typename V> symtab_t<V> make(alloc_t* alloc = context::top()->alloc);
        template <typename V> const symtab_node_t* get_node(const symtab_t<V>& symtab, u32 id);
        template <typename V> u0 init(symtab_t<V>& symtab, alloc_t* alloc = context::top()->alloc);
        template <typename V> b8 find_level_node(const symtab_t<V>& symtab, find_level_result_t& r, u8 sym);
        template <typename V> const symtab_node_t* find_node(const symtab_t<V>& symtab, str::slice_t prefix);
        template <typename V> b8 insert_key(symtab_t<V>& symtab, str::slice_t key, symtab_node_t** leaf_node);
        template <typename V> u32 append_node(symtab_t<V>& symtab, u8 sym = 0, u32 next = 0, u32 child = 0, u8 type = empty);
        template <typename V> b8 has_children(const symtab_t<V>& symtab, const symtab_node_t* node, const symtab_node_t* end_node = {});
        template <typename V, typename B=std::remove_pointer_t<V>> u0 find_prefix(const symtab_t<V>& symtab, assoc_array_t<B*>& pairs, str::slice_t prefix = {});
        template <typename V, typename B=std::remove_pointer_t<V>> u0 walk(const symtab_t<V>& symtab, const symtab_node_t* node, str_t& key, assoc_array_t<B*>& pairs);

        template <typename V> u0 free(symtab_t<V>& symtab) {
            clear(symtab);
        }

        template <typename V> u0 reset(symtab_t<V>& symtab) {
            symtab.size = {};
            for (auto& node : symtab.nodes) {
                node.sym   = {};
                node.type  = empty;
            }
            array::reset(symtab.values);
        }

        template <typename V> u0 clear(symtab_t<V>& symtab) {
            array::free(symtab.nodes);
            array::free(symtab.values);
            symtab.size = {};
        }

        template <typename V> symtab_t<V> make(alloc_t* alloc) {
            symtab_t<V> symtab{};
            init(symtab, alloc);
            return symtab;
        }

        template <typename V> u0 format_nodes(symtab_t<V>& symtab) {
            u32 n = 1;
            format::print("symtab: size = {}, nodes.size = {}, values.size = {}\n", symtab.size, symtab.nodes.size, symtab.values.size);
            for (const auto& node : symtab.nodes) {
                s8 c = (s8) node.sym;
                format::print("{:04}: sym: {} type: {} next: {:>4} child: {:>4} value: {:>4}\n", n++, isprint(c) ? c : '.', node.type, node.next, node.child, node.value);
            }
        }

        template <typename V> u0 init(symtab_t<V>& symtab, alloc_t* alloc) {
            symtab.size  = {};
            symtab.alloc = alloc;
            array::init(symtab.nodes, symtab.alloc);
            array::init(symtab.values, symtab.alloc);
            append_node(symtab, 0, 0, 0, empty);
        }

        template <typename V> u0 reserve(symtab_t<V>& symtab, u32 capacity) {
            array::reserve(symtab.nodes, capacity);
            array::reserve(symtab.values, capacity);
        }

        template <typename V> b8 remove(symtab_t<V>& symtab, str::slice_t prefix) {
            symtab_node_t* node;
            array_t<u32> prefix_nodes{};
            array::init(prefix_nodes, symtab.alloc);
            array::reserve(prefix_nodes, prefix.length * 2);
            defer(array::free(prefix_nodes));
            u32 next_node_id = 1;
            for (u32 i = 0; i < prefix.length; ++i) {
                node = get_node(symtab, next_node_id);
                while (node) {
                    if (i < prefix.length - 1)
                        array::append(prefix_nodes, next_node_id);
                    if (node->sym == prefix[i]) {
                        next_node_id = node->child;
                        break;
                    }
                    if (!node->next)
                        return false;
                    next_node_id = node->next;
                    node = get_node(symtab, next_node_id);
                }
            }
            if (node && node->type == leaf) {
                node->value = {};
                node->type  = used;
                --symtab.size;
                b8 parent_cleared{};
                if (!has_children(symtab, node)) {
                    node->type      = empty;
                    node->sym       = {};
                    parent_cleared  = true;
                }
                for (s32 i = prefix_nodes.size - 1; i >= 0; --i) {
                    const auto prefix_node = get_node(symtab, prefix_nodes[i]);
                    if (!has_children(symtab, prefix_node, node) || parent_cleared) {
                        prefix_node->type   = empty;
                        prefix_node->sym    = {};
                        parent_cleared      = {};
                    }
                }
                return true;
            }
            return false;
        }

        template <typename V> symtab_node_t* get_node(symtab_t<V>& symtab, u32 id) {
            return id == 0 || id > symtab.nodes.size ? nullptr : &symtab.nodes[id - 1];
        }

        template <typename V> b8 insert(symtab_t<V>& symtab, str::slice_t key, V& value) {
            symtab_node_t* leaf_node{};
            if (!insert_key(symtab, key, &leaf_node))
                return false;
            ++symtab.size;
            array::append(symtab.values, value);
            leaf_node->type  = leaf;
            leaf_node->value = symtab.values.size;
            return true;
        }

        template <typename V> b8 emplace(symtab_t<V>& symtab, str::slice_t key, V** value) {
            if (!value) return false;
            symtab_node_t* leaf_node{};
            if (!insert_key(symtab, key, &leaf_node))
                return false;
            ++symtab.size;
            *value = &array::append(symtab.values);
            leaf_node->type  = leaf;
            leaf_node->value = symtab.values.size;
            return true;
        }

        template <typename V> b8 find(const symtab_t<V>& symtab, str::slice_t key, V& value) {
            u32 next_node_id = 1;
            const symtab_node_t* level{};
            for (u32 i = 0; i < key.length; ++i) {
                level = get_node(symtab, next_node_id);
                while (level) {
                    if (level->sym == key[i]) {
                        next_node_id = level->child;
                        if (level->type != leaf && !next_node_id)
                            return false;
                        break;
                    }
                    if (!level->next)
                        return false;
                    level = get_node(symtab, level->next);
                }
            }

            if (!level || level->type != leaf)
                return false;
            value = symtab.values[level->value - 1];
            return true;
        }

        template <typename V> const symtab_node_t* get_node(const symtab_t<V>& symtab, u32 id) {
            return id == 0 || id > symtab.nodes.size ? nullptr : &symtab.nodes[id - 1];
        }

        template <typename V> b8 insert(symtab_t<V>& symtab, str::slice_t key, const V& value) {
            symtab_node_t* leaf_node{};
            if (!insert_key(symtab, key, &leaf_node))
                return false;
            ++symtab.size;
            array::append(symtab.values, value);
            leaf_node->type  = leaf;
            leaf_node->value = symtab.values.size;
            return true;
        }

        template <typename V> u32 append_node(symtab_t<V>& symtab, u8 sym, u32 next, u32 child, u8 type) {
            auto& node = array::append(symtab.nodes);
            node.sym   = sym;
            node.type  = type;
            node.next  = next;
            node.child = child;
            return symtab.nodes.size;
        }

        template <typename V> b8 find_level_node(const symtab_t<V>& symtab, find_level_result_t& r, u8 sym) {
            const symtab_node_t* curr_node = r.start_node;
            auto curr_id = r.node_id;
            r.avail_node = r.match_node = {};
            while (curr_node) {
                if (curr_node->sym == sym) {
                    r.node_id    = curr_id;
                    r.match_node = const_cast<symtab_node_t*>(curr_node);
                    return true;
                }
                if (!curr_node->next)
                    break;
                curr_id   = curr_node->next;
                curr_node = get_node(symtab, curr_id);
            }
            r.node_id    = curr_id;
            r.avail_node = const_cast<symtab_node_t*>(curr_node);
            return false;
        }

        template <typename V> const symtab_node_t* find_node(const symtab_t<V>& symtab, str::slice_t prefix) {
            u32 next_node_id = 1;
            for (u32 i = 0; i < prefix.length; ++i) {
                auto node = get_node(symtab, next_node_id);
                while (node) {
                    if (node->sym == prefix[i]) {
                        next_node_id = node->child;
                        break;
                    }
                    if (!node->next)
                        return nullptr;
                    node = get_node(symtab, node->next);
                }
            }
            return get_node(symtab, next_node_id);
        }

        template <typename V> b8 insert_key(symtab_t<V>& symtab, str::slice_t key, symtab_node_t** leaf_node) {
            if (!leaf_node || key.length == 0)
                return false;
            u32 next_node_id = 1;
            symtab_node_t* node = get_node(symtab, next_node_id);
            for (u32 i = 0; i < key.length - 1; ++i) {
                const auto sym = key[i];
                find_level_result_t r{};
                r.node_id    = next_node_id;
                r.start_node = node;
                if (!find_level_node(symtab, r, sym)) {
                    node = r.avail_node;
                    if (node->type == empty) {
                        node->sym  = key[i];
                        node->type = used;
                    } else {
                        next_node_id = node->next = append_node(symtab, sym, 0, 0, used);
                        node         = get_node(symtab, next_node_id);
                    }
                } else {
                    node             = r.match_node;
                    next_node_id     = r.node_id;
                }
                if (!node->child) {
                    auto child_node = append_node(symtab);
                    node = get_node(symtab, next_node_id);
                    node->child    = child_node;
                }
                next_node_id       = node->child;
                node               = get_node(symtab, next_node_id);
            }
            {
                const auto last_sym = key[key.length - 1];
                find_level_result_t r{};
                r.node_id    = next_node_id;
                r.start_node = node;
                if (!find_level_node(symtab, r, last_sym)) {
                    node = r.avail_node;
                    if (node->type == empty) {
                        node->type = used;
                        node->sym  = last_sym;
                    } else {
                        node->next = append_node(symtab, last_sym, 0, 0, used);
                        node       = get_node(symtab, node->next);
                    }
                } else {
                    node = r.match_node;
                }
            }
            *leaf_node = node;
            return node->type != leaf;
        }

        template <typename V, typename B> u0 find_prefix(const symtab_t<V>& symtab, assoc_array_t<B*>& pairs, str::slice_t prefix) {
            str_t key{};
            str::init(key, symtab.alloc);
            str::reserve(key, 32);
            defer(str::free(key));
            if (!slice::empty(prefix)) {
                auto node = find_node(symtab, prefix);
                if (node) {
                    str::append(key, prefix);
                    walk(symtab, node, key, pairs);
                }
            } else {
                walk(symtab, get_node(symtab, 1), key, pairs);
            }
        }

        template <typename V> b8 has_children(const symtab_t<V>& symtab, const symtab_node_t* node, const symtab_node_t* end_node) {
            while (true) {
                if (node->type == leaf) return true;
                if (node->child) {
                    auto child_node = get_node(symtab, node->child);
                    if (has_children(symtab, child_node, end_node))
                        return true;
                }
                if (!node->next)
                    return false;
                node = get_node(symtab, node->next);
                if (end_node && node == end_node)
                    return false;
            }
        }

        template <typename V, typename B=std::remove_pointer_t<V>> u0 format_pairs(symtab_t<V>& symtab, str::slice_t prefix = {}) {
            assoc_array_t<B*> pairs{};
            assoc_array::init(pairs);
            find_prefix(symtab, pairs, prefix);
            defer(assoc_array::free(pairs));
            for (u32 i = 0; i < pairs.size; ++i) {
                auto pair = pairs[i];
                format::print("{:<20}: {}\n", pair.key, *pair.value);
            }
        }

        template <typename V, typename B> u0 walk(const symtab_t<V>& symtab, const symtab_node_t* node, str_t& key, assoc_array_t<B*>& pairs) {
            while (true) {
                str::append(key, node->sym);
                if (node->type == leaf) {
                    if constexpr (std::is_pointer_v<V>) {
                        assoc_array::append(pairs, key, (B*) symtab.values[node->value - 1]);
                    } else {
                        assoc_array::append(pairs, key, (B*) &symtab.values[node->value - 1]);
                    }
                }
                if (node->child) {
                    auto child_node = get_node(symtab, node->child);
                    walk(symtab, child_node, key, pairs);
                }
                str::erase(key, key.length - 1, 1);
                if (!node->next)
                    break;
                node = get_node(symtab, node->next);
            }
        }
    }
}

