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

#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/graphviz/gv.h>
#include <basecode/core/assoc_array.h>
#include <basecode/core/stable_array.h>

namespace basecode {
    enum class symtab_node_type_t : u8 {
        empty,
        used,
        leaf,
    };

    template <typename T>
    concept Symbol_Table = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Value_Array;
        typename                T::Pair_Array;
        typename                T::Node_Type;
        typename                T::Node_Array;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.nodes}               -> same_as<typename T::Node_Array>;
        {t.values}              -> same_as<typename T::Value_Array>;
        {t.size}                -> same_as<u32>;
    };

    template <typename V>
    struct symtab_t final {
        using Value_Type        = V;
        using Value_Array       = stable_array_t<Value_Type>;
        using Pair_Array        = assoc_array_t<std::remove_pointer_t<Value_Type>*>;

        struct symtab_node_t final {
            symtab_node_t*      next;
            symtab_node_t*      child;
            Value_Type*         value;
            u32                 id;
            s8                  sym;
            symtab_node_type_t  type;
        };

        using Node_Type         = symtab_node_t;
        using Node_Array        = stable_array_t<Node_Type>;

        alloc_t*                alloc;
        Node_Array              nodes;
        Value_Array             values;
        u32                     size;
    };
    static_assert(sizeof(symtab_t<s32>) <= 80, "symtab_t<V> is now larger than 80 bytes!");

    namespace symtab {
        template <Symbol_Table T>
        u0 free(T& table);

        template <Symbol_Table T>
        u0 clear(T& table);

        template <Symbol_Table T>
        u0 reset(T& table);

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        Node_Type* make_node(T& table);

        template <Symbol_Table T>
        u0 reserve(T& table, u32 capacity);

        template <Symbol_Table T,
                  typename Pair_Array = typename T::Pair_Array>
        u0 find_prefix(const T& table,
                       Pair_Array& pairs,
                       str::slice_t prefix = {});

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        b8 has_children(const T& table,
                        const Node_Type* node,
                        const Node_Type* end_node = {});

        template <Symbol_Table T>
        u0 init(T& table, alloc_t* alloc = context::top()->alloc.main);

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        std::pair<Node_Type*, b8> find_level_node(const T& table,
                                                  s8 sym,
                                                  Node_Type* node);

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        const Node_Type* find_node(const T& table, str::slice_t prefix);

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        Node_Type* insert_key(T& table, str::slice_t key, Node_Type* root);

        template <Symbol_Table T,
                  b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>,
                  typename Node_Type = typename T::Node_Type,
                  typename Pair_Array = typename T::Pair_Array,
                  typename Base_Value_Type = std::remove_pointer_t<typename T::Value_Type>>
        u0 walk(const T& table, const Node_Type* node, str_t& key, Pair_Array& pairs);

        inline str::slice_t node_type_name(symtab_node_type_t type) {
            switch (type) {
                case symtab_node_type_t::empty:     return "empty"_ss;
                case symtab_node_type_t::used:      return "used"_ss;;
                case symtab_node_type_t::leaf:      return "leaf"_ss;
            }
        }

        template <Symbol_Table T>
        u0 free(T& table) {
            stable_array::free(table.nodes);
            stable_array::free(table.values);
            table.size = {};
        }

        template <Symbol_Table T>
        u0 reset(T& table) {
            table.size = {};
            stable_array::reset(table.nodes);
            stable_array::reset(table.values);
            make_node(table);
        }

        template <Symbol_Table T,
                  b8 Is_Pointer,
                  typename Node_Type,
                  typename Pair_Array,
                  typename Base_Value_Type>
        u0 walk(const T& table,
                const Node_Type* node,
                str_t& key,
                Pair_Array& pairs) {
            if (!node)
                return;
            while (true) {
                str::append(key, node->sym);
                if (node->type == symtab_node_type_t::leaf) {
                    if constexpr (Is_Pointer) {
                        assoc_array::append(pairs, key, *node->value);
                    } else {
                        assoc_array::append(pairs, key, node->value);
                    }
                }
                if (node->child)
                    walk(table, node->child, key, pairs);
                str::erase(key, key.length - 1, 1);
                if (!node->next)
                    break;
                node = node->next;
            }
        }

        template <Symbol_Table T, typename Node_Type>
        Node_Type* make_node(T& table) {
            Node_Type* node = &stable_array::append(table.nodes);
            std::memset(node, 0, sizeof(Node_Type));
            node->id = table.nodes.size;
            return node;
        }

        template <Symbol_Table T>
        u0 format_nodes(const T& table) {
            u32 n = 1;
            format::print("symtab: size = {}, nodes.size = {}, values.size = {}\n",
                          table.size,
                          table.nodes.size,
                          table.values.size);
            for (auto node : table.nodes) {
                s8 c = (s8) node->sym;
                format::print("{:04}: sym: {} ({:02x}) type: {} next: 0x{:016x} child: 0x{:016x} value: 0x{:016x}\n",
                              n++,
                              isprint(c) ? c : '.',
                              (u32) c,
                              node->type,
                              (u64) node->next,
                              (u64) node->child,
                              (u64) node->value);
            }
        }

        template <Symbol_Table T>
        u0 init(T& table, alloc_t* alloc) {
            table.size  = {};
            table.alloc = alloc;
            stable_array::init(table.nodes, table.alloc);
            stable_array::init(table.values, table.alloc);
            make_node(table);
        }

        template <Symbol_Table T>
        u0 reserve(T& table, u32 capacity) {
            stable_array::reserve(table.nodes, capacity);
            stable_array::reserve(table.values, capacity);
        }

        template <Symbol_Table T, typename Node_Type>
        Node_Type* insert_key(T& table,
                              str::slice_t key,
                              Node_Type* root) {
            auto curr = root;
            const auto short_key_len = key.length - 1;
            for (u32 i = 0; i < key.length; ++i) {
                auto [this_level, found] = find_level_node(table,
                                                           key[i],
                                                           curr);
                if (!found) {
                    if (this_level->type == symtab_node_type_t::empty) {
                        this_level->sym  = key[i];
                        this_level->type = symtab_node_type_t::used;
                    } else {
                        auto new_next = make_node(table);
                        new_next->sym  = key[i];
                        new_next->type = symtab_node_type_t::used;
                        this_level = this_level->next = new_next;
                    }
                    if (!this_level->child) {
                        if (i < short_key_len)
                            this_level->child = make_node(table);
                    }
                }
                if (i < short_key_len && this_level->child) {
                    curr = this_level->child;
                } else {
                    if (this_level->type == symtab_node_type_t::leaf
                    &&  !this_level->child) {
                        this_level->child = make_node(table);
                        this_level = this_level->child;
                    }
                    curr = this_level;
                }
            }
            return curr;
        }

        template <Symbol_Table T, typename Node_Type = typename T::Node_Type>
        b8 remove(T& table, str::slice_t prefix) {
            array_t<Node_Type*> prefix_nodes{};
            array::init(prefix_nodes, table.alloc);
            defer(array::free(prefix_nodes));
            Node_Type* node = &table.nodes[0];
            for (u32 i = 0; i < prefix.length; ++i) {
                while (node) {
                    if (i < prefix.length - 1)
                        array::append(prefix_nodes, node);
                    if (node->sym == prefix[i]) {
                        if (node->child && i < prefix.length - 1)
                            node = node->child;
                        break;
                    }
                    if (!node->next)
                        return false;
                    node = node->next;
                }
            }
            if (node && node->type == symtab_node_type_t::leaf) {
                stable_array::erase(table.values, node->value);
                node->value = {};
                node->type  = symtab_node_type_t::used;
                --table.size;
                b8 parent_cleared{};
                if (!has_children(table, node)) {
                    node->type = symtab_node_type_t::empty;
                    node->sym  = {};
                    parent_cleared = true;
                }
                for (s32 i = s32(prefix_nodes.size) - 1; i >= 0; --i) {
                    auto prefix_node = prefix_nodes[i];
                    if (parent_cleared || !has_children(table, prefix_node, node)) {
                        prefix_node->type = symtab_node_type_t::empty;
                        prefix_node->sym  = {};
                        parent_cleared    = {};
                    }
                }
                return true;
            }
            return false;
        }

        template <Symbol_Table T, typename Node_Type>
        b8 has_children(const T& table,
                        const Node_Type* node,
                        const Node_Type* end_node) {
            while (true) {
                if (node->type == symtab_node_type_t::leaf)
                    return true;
                if (node->child) {
                    if (has_children(table, node->child, end_node))
                        return true;
                }
                if (!node->next)
                    return false;
                node = node->next;
                if (end_node && node->next == end_node)
                    return false;
            }
        }

        template <Symbol_Table T>
        b8 create_dot_file(T& table, const path_t& path) {
            using namespace basecode::graphviz;

            buf_t buf{};
            buf.mode = buf_mode_t::alloc;
            buf::init(buf, table.alloc);

            graph_t g{};
            graph::init(g,
                        graph_type_t::directed,
                        "symtab"_ss,
                        table.alloc);
            graph::node_sep(g, 1);
            graph::rank(g, rank_type_t::same);
            graph::label(g, "symtab_t"_ss);
            defer(
                buf::free(buf);
                graph::free(g));

            u32 idx{};
            u32 node_ids[table.nodes.size];
            for (auto node : table.nodes) {
                auto trie_node  = graph::make_node(g);
                u32  id_field   = node::make_field(*trie_node);
                u32  sym_field  = node::make_field(*trie_node);
                u32  type_field = node::make_field(*trie_node);
                node::set_field_label(*trie_node,
                                      id_field,
                                      format::format("id: {}", node->id));
                node::set_field_label(*trie_node,
                                      sym_field,
                                      format::format("sym: {}",
                                                     isprint(node->sym) ? node->sym : '.'));
                node::set_field_label(*trie_node,
                                      type_field,
                                      format::format("type: {}",
                                                     symtab::node_type_name(node->type)));
                node::shape(*trie_node, shape_t::record);
                node::style(*trie_node, node_style_t::filled);
                node::fill_color(*trie_node, color_t::aliceblue);
                node_ids[idx] = trie_node->id;
                ++idx;
            }

            for (auto node : table.nodes) {
                if (node->next) {
                    auto edge = graph::make_edge(g);
                    edge->first  = graph::node_ref(&g,
                                                   node_ids[node->id - 1],
                                                   3,
                                                   compass_point_t::e);
                    edge->second = graph::node_ref(&g,
                                                   node_ids[node->next->id - 1],
                                                   1,
                                                   compass_point_t::w);
                    edge::label(*edge, "next"_ss);
                    edge::dir(*edge, dir_type_t::both);
                    edge::arrow_tail(*edge, arrow_type_t::dot);
                    edge::arrow_head(*edge, arrow_type_t::normal);
                }

                if (node->child) {
                    auto edge = graph::make_edge(g);
                    edge->first  = graph::node_ref(&g,
                                                   node_ids[node->id - 1],
                                                   2,
                                                   compass_point_t::s);
                    edge->second = graph::node_ref(&g,
                                                   node_ids[node->child->id - 1],
                                                   2,
                                                   compass_point_t::n);
                    edge::label(*edge, "child"_ss);
                    edge::dir(*edge, dir_type_t::both);
                    edge::arrow_tail(*edge, arrow_type_t::dot);
                    edge::arrow_head(*edge, arrow_type_t::normal);
                }
            }

            {
                auto status = graphviz::graph::serialize(g, buf);
                if (!OK(status))
                    return false;
            }
            {
                auto status = buf::save(buf, path);
                if (!OK(status))
                    return false;
            }

            return true;
        }

        template <Symbol_Table T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 set(T& table, str::slice_t key, Value_Type& value) {
            const Node_Type* level = find_node(table, key);
            if (!level || level->type != symtab_node_type_t::leaf)
                return false;
            *level->value = value;
            return true;
        }

        template <Symbol_Table T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 insert(T& table, str::slice_t key, auto& value) {
            auto leaf_node = insert_key(table,
                                        key,
                                        &table.nodes[0]);
            if (!leaf_node)
                return false;
            ++table.size;
            leaf_node->type  = symtab_node_type_t::leaf;
            leaf_node->value = &stable_array::append(table.values);
            *(leaf_node->value) = Value_Type(value);
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
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 emplace(T& table, str::slice_t key, Value_Type** value) {
            if (!value)
                return false;
            auto leaf_node = insert_key(table,
                                        key,
                                        &table.nodes[0]);
            if (!leaf_node)
                return false;
            ++table.size;
            leaf_node->type  = symtab_node_type_t::leaf;
            leaf_node->value = &stable_array::append(table.values);
            *value = leaf_node->value;
            return true;
        }

        template <Symbol_Table T, typename Node_Type>
        std::pair<Node_Type*, b8> find_level_node(const T& table,
                                                  s8 sym,
                                                  Node_Type* node) {
            while (node) {
                if (node->sym == sym)
                    return std::make_pair(node, true);
                if (!node->next)
                    return std::make_pair(node, false);
                node = node->next;
            }
            return std::make_pair(nullptr, false);
        }

        template <Symbol_Table T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 find(const T& table, str::slice_t key, Value_Type& value) {
            const Node_Type* level = &table.nodes[0];
            for (u32 i = 0; i < key.length; ++i) {
                auto [this_level, found] = find_level_node(table,
                                                           key[i],
                                                           level);
                if (!found)
                    return false;
                if (this_level->type == symtab_node_type_t::leaf
                &&  i == key.length - 1) {
                    level = this_level;
                    break;
                }
                level = this_level->child;
            }
            if (!level || level->type != symtab_node_type_t::leaf)
                return false;
            value = *level->value;
            return true;
        }

        template <Symbol_Table T,
                  typename Node_Type = typename T::Node_Type,
                  typename Value_Type = typename T::Value_Type>
        b8 insert(T& table, str::slice_t key, const auto& value) {
            auto leaf_node = insert_key(table,
                                        key,
                                        &table.nodes[0]);
            if (!leaf_node)
                return false;
            ++table.size;
            leaf_node->type = symtab_node_type_t::leaf;
            leaf_node->value = &stable_array::append(table.values,
                                                     Value_Type(value));
            return true;
        }

        template <Symbol_Table T, typename Node_Type>
        const Node_Type* find_node(const T& table, str::slice_t prefix) {
            auto node = &table.nodes[0];
            for (u32 i = 0; i < prefix.length; ++i) {
                auto [this_level, found] = find_level_node(table,
                                                           prefix[i],
                                                           node);
                if (!found)
                    break;
                if (i < prefix.length - 1)
                    node = this_level->child;
            }
            return node;
        }

        template <Symbol_Table T, typename Pair_Array>
        u0 find_prefix(const T& table, Pair_Array& pairs, str::slice_t prefix) {
            str_t key{};
            str::init(key, table.alloc);
            str::reserve(key, 64);
            defer(str::free(key));
            if (!slice::empty(prefix)) {
                auto node = find_node(table, prefix);
                if (node) {
                    str::append(key, prefix);
                    walk(table, node, key, pairs);
                }
            } else {
                walk(table, &table.nodes[0], key, pairs);
            }
        }
    }
}

