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

#include <basecode/core/memory.h>

namespace basecode {
    struct assoc_key_idx_t final {
        u64                         status: 2;
        u64                         offset: 31;
        u64                         length: 31;
    };

    template <typename V>
    struct assoc_pair_t final {
        using Value_Type            = std::remove_pointer_t<V>;

        str::slice_t                key;
        Value_Type*                 value;
    };

    template <typename V>
    struct assoc_array_t final {
        using Value_Type            = V;

        alloc_t*                    alloc;
        Value_Type*                 values;
        assoc_key_idx_t*            index;
        struct {
            u8*                     data;
            u32                     size;
            u32                     capacity;
        }                           keys;
        u32                         size;
        u32                         capacity;

        assoc_pair_t<Value_Type> operator[](u32 i) const {
            const auto& idx = index[i];
            if constexpr (std::is_pointer_v<Value_Type>) {
                return assoc_pair_t<V>{
                    slice::make(keys.data + idx.offset, idx.length),
                    values[i]
                };
            } else {
                return assoc_pair_t<V>{
                    slice::make(keys.data + idx.offset, idx.length),
                    &values[i]
                };
            }
        }
    };
    static_assert(sizeof(assoc_array_t<s32>) <= 48, "assoc_array_t is now greater than 48 bytes!");

    namespace assoc_array {
        template <typename V>
        u0 free(assoc_array_t<V>& array) {
            memory::free(array.alloc, array.index);
            memory::free(array.alloc, array.values);
            memory::free(array.alloc, array.keys.data);
            array.index     = {};
            array.keys.data = {};
            array.size      = array.capacity      = {};
            array.keys.size = array.keys.capacity = {};
        }

        template <typename V>
        u0 reset(assoc_array_t<V>& array) {
            std::memset(array.values,
                        0,
                        array.capacity * sizeof(V));
            std::memset(array.index,
                        0,
                        array.capacity * sizeof(assoc_key_idx_t));
            array.size = array.keys.size = {};
        }

        template <typename V>
        b8 empty(const assoc_array_t<V>& array) {
            return array.size == 0;
        }

        template <typename V>
        u0 reserve_keys(assoc_array_t<V>& array, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(array.alloc, array.keys.data);
                array.keys.data = {};
                array.keys.size = array.keys.capacity = {};
                return;
            }
            if (new_capacity == array.capacity)  return;
            new_capacity = std::max(array.keys.size, new_capacity);
            array.keys.data     = (u8*) memory::realloc(array.alloc,
                                                        array.keys.data,
                                                        new_capacity);
            array.keys.capacity = new_capacity;
        }

        template <typename V>
        u0 grow_keys(assoc_array_t<V>& array, u32 new_capacity = 0) {
            new_capacity = std::max(new_capacity, array.keys.capacity);
            reserve_keys(array, new_capacity * 2 + 8);
        }

        template <typename V>
        u0 reserve_values(assoc_array_t<V>& array, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(array.alloc, array.index);
                memory::free(array.alloc, array.values);
                array.index  = {};
                array.values = {};
                array.size   = array.capacity = {};
                return;
            }
            if (new_capacity == array.capacity) return;
            new_capacity = std::max(array.size, new_capacity);
            array.values = (V*) memory::realloc(
                array.alloc,
                array.values,
                new_capacity * sizeof(V),
                alignof(V));
            array.index  = (assoc_key_idx_t*) memory::realloc(
                array.alloc,
                array.index,
                new_capacity * sizeof(assoc_key_idx_t),
                alignof(assoc_key_idx_t));
            const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
            std::memset(
                array.values + array.size,
                0,
                size_to_clear * sizeof(V));
            std::memset(
                array.index + array.size,
                0,
                size_to_clear * sizeof(assoc_key_idx_t));
            array.capacity = new_capacity;
        }

        template <typename V>
        u0 grow_values(assoc_array_t<V>& array, u32 new_capacity = 0) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve_values(array, new_capacity * 2 + 8);
        }

        template <typename V>
        u0 append(assoc_array_t<V>& array, const s8* str, const V& value) {
            const auto length = strlen(str) + 1;
            if (array.size + 1 > array.capacity)                grow_values(array);
            if (array.keys.size + length > array.keys.capacity) grow_keys(array, length);
            auto& idx = array.idx[array.size];
            idx.offset = array.keys.size;
            idx.length = length - 1;
            array.values[array.size++] = value;
            std::memcpy(array.keys.data + idx.offset, str, length);
            array.keys.size += length;
        }

        template <typename V>
        u0 init(assoc_array_t<V>& array, alloc_t* alloc = context::top()->alloc) {
            array.alloc     = alloc;
            array.values    = {};
            array.index     = {};
            array.keys.data = {};
            array.size      = array.capacity      = {};
            array.keys.size = array.keys.capacity = {};
        }

        template <typename V,
            typename Value_Type = std::remove_pointer_t<V>>
        Value_Type* find(const assoc_array_t<V>& array, const String_Concept auto& key) {
            for (u32 i = 0; i < array.size; ++i) {
                const auto& idx = array.index[i];
                if (key.length == idx.length
                &&  std::memcmp(array.keys.data + idx.offset, key.data, key.length) == 0) {
                    if constexpr (std::is_pointer_v<V>) {
                        return array.values[i];
                    } else {
                        return &array.values[i];
                    }
                }
            }
            return (Value_Type*) nullptr;
        }

        template <typename V>
        u0 append(assoc_array_t<V>& array, const String_Concept auto& str, const V& value) {
            if (array.size + 1 > array.capacity)                            grow_values(array);
            if (array.keys.size + (str.length + 1) > array.keys.capacity)   grow_keys(array, str.length + 1);
            auto& idx = array.index[array.size];
            idx.offset = array.keys.size;
            idx.length = str.length;
            array.values[array.size++] = value;
            std::memcpy(array.keys.data + idx.offset, str.data, str.length);
            array.keys.size += str.length;
            array.keys.data[array.keys.size++] = '\0';
        }
    }
}

