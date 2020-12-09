// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
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
#include <basecode/core/iterator.h>

namespace basecode {
    struct list_node_t final {
        u64                     free    : 1;
        u64                     prev    : 21;
        u64                     next    : 21;
        u64                     value   : 21;
    };

    template <typename T>
    concept Linked_List = requires(const T& t) {
        typename                T::Value_Type;
        {t.alloc}               -> same_as<alloc_t*>;
        {t.nodes}               -> same_as<array_t<list_node_t>>;
        {t.values}              -> same_as<array_t<typename T::Value_Type>>;
        {t.head}                -> same_as<u32>;
        {t.tail}                -> same_as<u32>;
        {t.free}                -> same_as<u32>;
        {t.size}                -> same_as<u32>;
    };

    template <typename T>
    struct list_t final {
        using Value_Type        = T;

        alloc_t*                alloc;
        array_t<list_node_t>    nodes;
        array_t<T>              values;
        u32                     head;
        u32                     tail;
        u32                     free;
        u32                     size;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const list_t* ref) {
                pos = ref->nodes.size + 1;
            }
            inline u0 next(const list_t* ref) {
                pos = (&ref->nodes[pos - 1])->next;
                pos = pos == 0 ? ref->nodes.size + 1 : pos;
            }
            inline u0 begin(const list_t* ref) {
                pos = ref->head;
            }
            inline list_node_t* get(list_t* ref) {
                return &ref->nodes[pos - 1];
            }
            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
            inline const list_node_t* get(const list_t* ref) {
                return &ref->nodes[pos - 1];
            }
        };
        DECL_ITERS(list_t, list_node_t*, iterator_state_t);
    };

    namespace list {
        u0 free(Linked_List auto& list) {
            array::free(list.nodes);
            array::free(list.values);
            list.head = list.tail = list.size = {};
        }

        u0 clear(Linked_List auto& list) {
            free(list);
        }

        u0 reset(Linked_List auto& list) {
            list.head = list.tail = list.size = {};
            array::reset(list.nodes);
            array::reset(list.values);
        }

        inline b8 empty(const Linked_List auto& list) {
            return list.size == 0;
        }

        inline list_node_t* head(Linked_List auto& list) {
            return get_node(list, list.head);
        }

        inline list_node_t* tail(Linked_List auto& list) {
            return get_node(list, list.tail);
        }

        b8 remove(Linked_List auto& list, list_node_t* node) {
            if (!node || list.size == 0) return false;
            auto head = get_node(list, list.head);
            auto tail = get_node(list, list.tail);
            auto prev = get_node(list, node->prev);
            auto next = get_node(list, node->next);
            if (next) {
                list.free  = node->prev;
                next->prev = node->prev;
            }
            if (prev) {
                list.free  = prev->next;
                prev->next = node->next;
            }
            if (head == node)
                list.head  = node->next;
            if (tail == node)
                list.tail  = node->prev;
            node->free     = true;
            node->next     = node->prev = {};
            --list.size;
            return true;
        }

        u0 reserve(Linked_List auto& list, u32 new_capacity) {
            array::reserve(list.nodes, new_capacity);
            array::reserve(list.values, new_capacity);
        }

        template <typename T> requires Linked_List<T>
        inline typename T::Value_Type& value(T& list, list_node_t* node) {
            return list.values[node->value - 1];
        }

        template <typename T> requires Linked_List<T>
        list_node_t* append(T& list, const typename T::Value_Type& value) {
            auto tail = get_node(list, list.tail);
            if (!tail) {
                list.head = list.tail = alloc_node(list, &tail);
            } else {
                list_node_t* new_tail{};
                auto new_id = alloc_node(list, &new_tail);
                if (list.tail == list.head) {
                    auto head = get_node(list, list.head);
                    head->next = new_id;
                    new_tail->prev = list.head;
                } else {
                    new_tail->prev = list.tail;
                    tail->next = new_id;
                }
                list.tail = new_id;
                tail = new_tail;
            }
            tail->free = false;
            if (tail->value) {
                list.free = 0;
                list.values[tail->value - 1] = value;
            } else {
                array::append(list.values, value);
                tail->value = list.values.size;
            }
            ++list.size;
            return tail;
        }

        inline list_node_t* get_node(Linked_List auto& list, u32 id) {
            return id == 0 || id > list.nodes.size ? nullptr : &list.nodes[id - 1];
        }

        inline list_node_t* prev(Linked_List auto& list, list_node_t* node) {
            return get_node(list, node->prev);
        }

        inline list_node_t* next(Linked_List auto& list, list_node_t* node) {
            return get_node(list, node->next);
        }

        u32 alloc_node(Linked_List auto& list, list_node_t** new_node = {}) {
            if (list.free) {
                for (u32 i = list.free - 1; i < list.nodes.size; ++i) {
                    auto& node = list.nodes[i];
                    if (node.free) {
                        if (new_node) *new_node = &node;
                        return i + 1;
                    }
                }
            }
            auto node = &array::append(list.nodes);
            node->free = true;
            node->prev = node->next = node->value = {};
            if (new_node) *new_node = node;
            return list.nodes.size;
        }

        u0 init(Linked_List auto& list, alloc_t* alloc = context::top()->alloc) {
            list.alloc = alloc;
            list.head  = list.tail = list.size = {};
            array::init(list.nodes, list.alloc);
            array::init(list.values, list.alloc);
        }
    }
}
