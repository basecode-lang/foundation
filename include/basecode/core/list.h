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

namespace basecode::list {
    template <Linked_List T>
    u32 alloc_node(T& list, list_node_t** new_node = {});

    inline list_node_t* get_node(Linked_List auto& list, u32 id);

    template <Linked_List T>
    u0 free(T& list) {
        array::free(list.nodes);
        array::free(list.values);
        list.head = list.tail = list.size = {};
    }

    template <Linked_List T>
    u0 clear(T& list) {
        free(list);
    }

    template <Linked_List T>
    u0 reset(T& list) {
        list.head = list.tail = list.size = {};
        array::reset(list.nodes);
        array::reset(list.values);
    }

    template <Linked_List T>
    inline b8 empty(const T& list) {
        return list.size == 0;
    }

    template <Linked_List T>
    inline list_node_t* head(T& list) {
        return get_node(list, list.head);
    }

    template <Linked_List T>
    inline list_node_t* tail(T& list) {
        return get_node(list, list.tail);
    }

    template <Linked_List T>
    b8 remove(T& list, list_node_t* node) {
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

    template <Linked_List T>
    u0 reserve(T& list, u32 new_capacity) {
        array::reserve(list.nodes, new_capacity);
        array::reserve(list.values, new_capacity);
    }

    template <Linked_List T,
              typename Value_Type = std::remove_reference_t<typename T::Value_Type>>
    inline Value_Type& value(T& list, list_node_t* node) {
        return list.values[node->value - 1];
    }

    template <Linked_List T,
              typename Value_Type = std::remove_reference_t<typename T::Value_Type>>
    list_node_t* append(T& list, const Value_Type& value) {
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

    template <Linked_List T>
    inline list_node_t* get_node(T& list, u32 id) {
        return id == 0 || id > list.nodes.size ? nullptr : &list.nodes[id - 1];
    }

    template <Linked_List T>
    u32 alloc_node(T& list, list_node_t** new_node) {
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

    template <Linked_List T>
    inline list_node_t* prev(T& list, list_node_t* node) {
        return get_node(list, node->prev);
    }

    template <Linked_List T>
    inline list_node_t* next(T& list, list_node_t* node) {
        return get_node(list, node->next);
    }

    template <Linked_List T>
    u0 init(T& list, alloc_t* alloc = context::top()->alloc.main) {
        list.alloc = alloc;
        list.head  = list.tail = list.size = {};
        array::init(list.nodes, list.alloc);
        array::init(list.values, list.alloc);
    }
}
