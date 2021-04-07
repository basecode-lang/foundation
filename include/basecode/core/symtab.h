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

#define GET_NODE(t, id)         (&((t).nodes[(id) - 1]))

namespace basecode {
    constexpr u8 empty          = 0b00;
    constexpr u8 used           = 0b01;
    constexpr u8 leaf           = 0b10;

    struct symtab_node_t final {
        u64                     type:   2;
        u64                     sym:    8;
        u64                     next:   18;
        u64                     child:  18;
        u64                     value:  18;
    };

    template <typename T>
    concept Symbol_Table = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Pair_Array;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.nodes}               -> same_as<array_t<symtab_node_t>>;
        {t.values}              -> same_as<array_t<typename T::Value_Type>>;
        {t.size}                -> same_as<u32>;
    };


    template <typename V>
    struct symtab_t final {
        using Value_Type        = V;
        using Pair_Array        = assoc_array_t<std::remove_pointer_t<V>*>;

        alloc_t*                alloc;
        array_t<symtab_node_t>  nodes;
        array_t<V>              values;
        u32                     size;
    };
    static_assert(sizeof(symtab_t<s32>) <= 64, "symtab_t<V> is now larger than 64 bytes!");

    struct find_level_result_t final {
        symtab_node_t*          start_node;
        symtab_node_t*          match_node;
        symtab_node_t*          avail_node;
        u32                     node_id;
    } __attribute__((aligned(32)));

    namespace symtab {
        template <Symbol_Table T>
        u0 free(T& table);

        template <Symbol_Table T>
        u0 clear(T& table);

        template <Symbol_Table T>
        u0 reset(T& table);

        template <Symbol_Table T>
        b8 has_children(const T& table,
                        const symtab_node_t* node,
                        const symtab_node_t* end_node = {});

        template <Symbol_Table T>
        inline u32 append_node(T& table,
                               u8 sym = 0,
                               u32 next = 0,
                               u32 child = 0,
                               u8 type = empty);

        template <Symbol_Table T>
        u0 reserve(T& table, u32 capacity);

        template <Symbol_Table T>
        u0 init(T& table, alloc_t* alloc = context::top()->alloc);

        template <Symbol_Table T>
        b8 find_level_node(const T& table, find_level_result_t& r, u8 sym);

        template <Symbol_Table T>
        const symtab_node_t* find_node(const T& table, str::slice_t prefix);

        template <Symbol_Table T>
        b8 insert_key(T& table, str::slice_t key, symtab_node_t** leaf_node);

        template <Symbol_Table T,
                  typename Pair_Array = typename T::Pair_Array>
        u0 find_prefix(const T& table, Pair_Array& pairs, str::slice_t prefix = {});

        template <Symbol_Table T,
                  typename Pair_Array = typename T::Pair_Array,
                  b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>,
                  typename Bare_Value_Type = std::remove_pointer_t<typename T::Value_Type>>
        u0 walk(const T& table, const symtab_node_t* node, str_t& key, Pair_Array& pairs);

        template <Symbol_Table T>
        u0 free(T& table) {
            clear(table);
        }

        template <Symbol_Table T>
        u0 reset(T& table) {
            table.size = {};
            array::reset(table.nodes);
            array::reset(table.values);
            append_node(table, 0, 0, 0, empty);
        }

        template <Symbol_Table T>
        u0 clear(T& table) {
            array::free(table.nodes);
            array::free(table.values);
            table.size = {};
        }

        template <Symbol_Table T>
        b8 has_children(const T& table,
                        const symtab_node_t* node,
                        const symtab_node_t* end_node) {
            while (true) {
                if (node->type == leaf) return true;
                if (node->child) {
                    auto child_node = GET_NODE(table, node->child);
                    if (has_children(table, child_node, end_node))
                        return true;
                }
                if (!node->next)
                    return false;
                node = GET_NODE(table, node->next);
                if (end_node && node == end_node)
                    return false;
            }
        }

        template <Symbol_Table T>
        u0 format_nodes(const T& table) {
            u32 n = 1;
            format::print("symtab: size = {}, nodes.size = {}, values.size = {}\n",
                          table.size,
                          table.nodes.size,
                          table.values.size);
            for (const auto& node : table.nodes) {
                s8 c = (s8) node.sym;
                format::print("{:04}: sym: {} ({:02x}) type: {} next: {:>4} child: {:>4} value: {:>4}\n",
                              n++,
                              isprint(c) ? c : '.',
                              (u32) c,
                              node.type,
                              node.next,
                              node.child,
                              node.value);
            }
        }

        template <Symbol_Table T>
        u0 init(T& table, alloc_t* alloc) {
            table.size  = {};
            table.alloc = alloc;
            array::init(table.nodes, table.alloc);
            array::init(table.values, table.alloc);
            append_node(table, 0, 0, 0, empty);
        }

        template <Symbol_Table T>
        u0 reserve(T& table, u32 capacity) {
            array::reserve(table.nodes, capacity);
            array::reserve(table.values, capacity);
        }

        template <Symbol_Table T>
        b8 remove(T& table, str::slice_t prefix) {
            symtab_node_t* node;
            u32 prefix_nodes[table.nodes.size];
            u32 prefix_count{};
            u32 next_node_id = 1;
            for (u32 i = 0; i < prefix.length; ++i) {
                node = GET_NODE(table, next_node_id);
                while (node) {
                    if (i < prefix.length - 1)
                        prefix_nodes[prefix_count++] = next_node_id;
                    if (node->sym == prefix[i]) {
                        next_node_id = node->child;
                        break;
                    }
                    if (!node->next)
                        return false;
                    next_node_id = node->next;
                    node = GET_NODE(table, next_node_id);
                }
            }
            if (node && node->type == leaf) {
                node->value = {};
                node->type  = used;
                --table.size;
                b8 parent_cleared{};
                if (!has_children(table, node)) {
                    node->type = empty;
                    node->sym  = {};
                    parent_cleared = true;
                }
                for (s32 i = prefix_count - 1; i >= 0; --i) {
                    const auto prefix_node = GET_NODE(table, prefix_nodes[i]);
                    if (!has_children(table, prefix_node, node) || parent_cleared) {
                        prefix_node->type = empty;
                        prefix_node->sym  = {};
                        parent_cleared = {};
                    }
                }
                return true;
            }
            return false;
        }

        template <Symbol_Table T,
                  typename Value_Type = typename T::Value_Type>
        b8 set(T& table, str::slice_t key, Value_Type& value) {
            u32 next_node_id = 1;
            const symtab_node_t* level{};
            for (u32 i = 0; i < key.length; ++i) {
                level = GET_NODE(table, next_node_id);
                while (level) {
                    if (level->sym == key[i]) {
                        next_node_id = level->child;
                        if (level->type != leaf && !next_node_id)
                            return false;
                        break;
                    }
                    if (!level->next)
                        return false;
                    level = GET_NODE(table, level->next);
                }
            }
            if (!level || level->type != leaf)
                return false;
            table.values[level->value - 1] = value;
            return true;
        }

        template <Symbol_Table T,
                  typename Value_Type = typename T::Value_Type>
        b8 insert(T& table, str::slice_t key, Value_Type& value) {
            symtab_node_t* leaf_node{};
            if (!insert_key(table, key, &leaf_node))
                return false;
            ++table.size;
            array::append(table.values, value);
            leaf_node->type  = leaf;
            leaf_node->value = table.values.size;
            return true;
        }

        template <Symbol_Table T,
                  typename Pair_Array = typename T::Pair_Array>
        u0 format_pairs(const T& table, str::slice_t prefix = {}) {
            Pair_Array pairs{};
            assoc_array::init(pairs);
            find_prefix(table, pairs, prefix);
            defer(assoc_array::free(pairs));
            for (u32 i = 0; i < pairs.size; ++i) {
                auto pair = pairs[i];
                format::print("{:<20}: {}\n", pair.key, *pair.value);
            }
        }

        template <Symbol_Table T,
                  typename Value_Type = typename T::Value_Type>
        b8 emplace(T& table, str::slice_t key, Value_Type** value) {
            if (!value)
                return false;
            symtab_node_t* leaf_node{};
            if (!insert_key(table, key, &leaf_node))
                return false;
            ++table.size;
            *value = &array::append(table.values);
            leaf_node->type  = leaf;
            leaf_node->value = table.values.size;
            return true;
        }

        template <Symbol_Table T,
                  typename Value_Type = typename T::Value_Type>
        b8 find(const T& table, str::slice_t key, Value_Type& value) {
            u32 next_node_id = 1;
            const symtab_node_t* level{};
            for (u32 i = 0; i < key.length; ++i) {
                level = GET_NODE(table, next_node_id);
                while (level) {
                    if (level->sym == key[i]) {
                        next_node_id = level->child;
                        if (level->type != leaf && !next_node_id)
                            return false;
                        break;
                    }
                    if (!level->next)
                        return false;
                    level = GET_NODE(table, level->next);
                }
            }
            if (!level || level->type != leaf)
                return false;
            value = table.values[level->value - 1];
            return true;
        }

        template <Symbol_Table T,
                  typename Value_Type = typename T::Value_Type>
        b8 insert(T& table, str::slice_t key, const Value_Type& value) {
            symtab_node_t* leaf_node{};
            if (!insert_key(table, key, &leaf_node))
                return false;
            ++table.size;
            array::append(table.values, value);
            leaf_node->type  = leaf;
            leaf_node->value = table.values.size;
            return true;
        }

        template <Symbol_Table T>
        b8 find_level_node(const T& table, find_level_result_t& r, u8 sym) {
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
                curr_node = GET_NODE(table, curr_id);
            }
            r.node_id    = curr_id;
            r.avail_node = const_cast<symtab_node_t*>(curr_node);
            return false;
        }

        template <Symbol_Table T>
        const symtab_node_t* find_node(const T& table, str::slice_t prefix) {
            u32 next_node_id = 1;
            for (u32 i = 0; i < prefix.length; ++i) {
                auto node = GET_NODE(table, next_node_id);
                while (node) {
                    if (node->sym == prefix[i]) {
                        if (node->child)
                            next_node_id = node->child;
                        break;
                    }
                    if (!node->next)
                        return nullptr;
                    node = GET_NODE(table, node->next);
                }
            }
            return GET_NODE(table, next_node_id);
        }

        template <Symbol_Table T>
        b8 insert_key(T& table, str::slice_t key, symtab_node_t** leaf_node) {
            if (!leaf_node || key.length == 0)
                return false;
            u32 next_node_id = 1;
            find_level_result_t r{};
            symtab_node_t* node = GET_NODE(table, next_node_id);
            for (u32 i = 0; i < key.length - 1; ++i) {
                const auto sym = key[i];
                r.node_id    = next_node_id;
                r.start_node = node;
                if (!find_level_node(table, r, sym)) {
                    node = r.avail_node;
                    if (node->type == empty) {
                        node->sym  = key[i];
                        node->type = used;
                    } else {
                        next_node_id = node->next = append_node(table, sym, 0, 0, used);
                        node         = GET_NODE(table, next_node_id);
                    }
                } else {
                    node         = r.match_node;
                    next_node_id = r.node_id;
                }
                if (!node->child) {
                    auto child_node = append_node(table);
                    node = GET_NODE(table, next_node_id);
                    node->child    = child_node;
                }
                next_node_id = node->child;
                node         = GET_NODE(table, next_node_id);
            }
            {
                const auto last_sym = key[key.length - 1];
                r.node_id    = next_node_id;
                r.start_node = node;
                if (!find_level_node(table, r, last_sym)) {
                    node = r.avail_node;
                    if (node->type == empty) {
                        node->type = used;
                        node->sym  = last_sym;
                    } else {
                        node->next = append_node(table, last_sym, 0, 0, used);
                        node       = GET_NODE(table, node->next);
                    }
                } else {
                    node = r.match_node;
                }
            }
            *leaf_node = node;
            return node->type != leaf;
        }

        template <Symbol_Table T, typename Pair_Array>
        u0 find_prefix(const T& table, Pair_Array& pairs, str::slice_t prefix) {
            str_t key{};
            str::init(key, table.alloc);
            str::reserve(key, 32);
            defer(str::free(key));
            if (!slice::empty(prefix)) {
                auto node = find_node(table, prefix);
                if (node) {
                    str::append(key, prefix);
                    walk(table, node, key, pairs);
                }
            } else {
                walk(table, GET_NODE(table, 1), key, pairs);
            }
        }

        template <Symbol_Table T>
        inline u32 append_node(T& table, u8 sym, u32 next, u32 child, u8 type) {
            auto& node = array::append(table.nodes);
            node.sym   = sym;
            node.type  = type;
            node.next  = next;
            node.child = child;
            return table.nodes.size;
        }

        template <Symbol_Table T,
                  typename Pair_Array,
                   b8 Is_Pointer,
                  typename Bare_Value_Type>
        u0 walk(const T& table, const symtab_node_t* node, str_t& key, Pair_Array& pairs) {
            while (true) {
                str::append(key, node->sym);
                if (node->type == leaf) {
                    if constexpr (Is_Pointer) {
                        assoc_array::append(pairs,
                                            key,
                                            (Bare_Value_Type*) table.values[node->value - 1]);
                    } else {
                        assoc_array::append(pairs,
                                            key,
                                            (Bare_Value_Type*) &table.values[node->value - 1]);
                    }
                }
                if (node->child) {
                    auto child_node = GET_NODE(table, node->child);
                    walk(table, child_node, key, pairs);
                }
                str::erase(key, key.length - 1, 1);
                if (!node->next)
                    break;
                node = GET_NODE(table, node->next);
            }
        }
    }
}

