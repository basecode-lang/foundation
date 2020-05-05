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

#include <cassert>
#include <algorithm>
#include "str.h"
#include "types.h"
#include "array.h"
#include "format.h"
#include "context.h"
#include "hashable.h"
#include "memory/memory.h"

#pragma once

namespace basecode {
    template <typename K, typename V>
    struct hashtable_t final {
        u64*                    hashes;
        K*                      keys;
        V*                      values;
        alloc_t*                alloc;
        u32                     size;
        u32                     capacity;
        f32                     load_factor;
    };

    namespace hashtable {
        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table);
        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>& table);
        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity = 16);
        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* alloc = context::top()->alloc);
        template <typename K, typename V> b8 find_free_bucket(u64* hashes, u32 size, u32 start, u32* found);
        template<typename K, typename V> b8 find_key(hashtable_t<K, V>& table, u32 start, u64 hash, const K& key, u32* found);
        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc = context::top()->alloc, f32 load_factor = .5f);

        template<typename K, typename V,
            typename B=std::remove_pointer_t<K>,
            typename A=array_t<B*>,
            b8 IsPtr=std::is_pointer<K>::value>
        auto keys(hashtable_t<K, V>& table) {
            A list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (IsPtr) {
                    array::append(list, table.keys[i]);
                } else {
                    array::append(list, &table.keys[i]);
                }
            }
            return list;
        }

        template<typename K, typename V,
            typename B=std::remove_pointer_t<V>,
            typename A=array_t<B*>,
            b8 IsPtr=std::is_pointer<V>::value>
        auto values(hashtable_t<K, V>& table) {
            A list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (IsPtr) {
                    array::append(list, table.values[i]);
                } else {
                    array::append(list, &table.values[i]);
                }
            }
            return list;
        }

        template<typename K, typename V> u0 free(hashtable_t<K, V>& table) {
            clear(table);
        }

        template<typename K, typename V> u0 reset(hashtable_t<K, V>& table) {
            if (table.hashes) std::memset(table.hashes, 0, table.capacity * sizeof(u64));
            table.size = {};
        }

        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table) {
            assert(table.alloc);
            memory::free(table.alloc, table.hashes);
            table.keys      = {};
            table.values    = {};
            table.hashes    = {};
            table.capacity  = table.size = {};
        }

        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* alloc) {
            hashtable_t<K, V> table;
            init(table, alloc);
            return table;
        }

        template<typename K, typename V> inline b8 empty(hashtable_t<K, V>& table) {
            return table.size == 0;
        }

        template<typename K, typename V,
            typename B = std::remove_pointer_t<V>,
            b8 IsPtr=std::is_pointer<V>::value>
        B* find(hashtable_t<K, V>& table, const K& key) {
            if (table.capacity <= 0) return nullptr;

            u64 hash            = hashing::hash64(key);
            u32 bucket_index    = ((u128) hash * (u128) table.capacity) >> 64;
            u32 found_index;
            if (find_key(table, bucket_index, hash, key, &found_index)) {
                if constexpr (IsPtr) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return nullptr;
        }

        template<typename K, typename V,
            typename B=std::remove_pointer_t<V>,
            b8 IsPtr=std::is_pointer<V>::value>
        B* emplace(hashtable_t<K, V>& table, const K& key) {
            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            u64 hash                    = hashing::hash64(key);
            u32 bucket_index            = ((u128) hash * (u128) table.capacity) >> 64;
            u32 found_index;

            if (find_free_bucket<K ,V>(table.hashes, table.capacity, bucket_index, &found_index)) {
                table.keys[found_index] = key;
                table.hashes[found_index] = hash;
                ++table.size;
                if constexpr (IsPtr) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return nullptr;
        }

        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>& table) {
            return table.capacity == 0 || table.size + 1 > (table.capacity - 1) * table.load_factor;
        }

        template<typename K, typename V> b8 remove(hashtable_t<K, V>& table, const K& key) {
            if (table.capacity == 0)
                return false;
            u64 hash = hashing::hash64(key);
            u32 bucket_index = ((u128) hash * (u128) table.capacity) >> 64;
            u32 found_index;
            if (!find_key(table, bucket_index, hash, key, &found_index))
                return false;
            table.hashes[found_index] = 0;
            --table.size;
            return true;
        }

        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity) {
            new_capacity = std::max<u32>(new_capacity, std::ceil(std::max<u32>(16, new_capacity) / table.load_factor));

            const auto size_of_hashes = new_capacity * sizeof(u64);
            const auto size_of_keys   = new_capacity * sizeof(K);
            const auto size_of_values = new_capacity * sizeof(V);

            const auto buf_size = size_of_hashes + alignof(K) + size_of_keys + alignof(V) + size_of_values;

            auto buf = (u8*) memory::alloc(table.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, size_of_hashes);
            auto new_hashes = (u64*) buf;

            u32 keys_align{}, values_align{};
            auto new_keys   = (K*) memory::system::align_forward(buf + size_of_hashes, alignof(K), keys_align);
            auto new_values = (V*) memory::system::align_forward(buf + size_of_hashes + size_of_keys + keys_align, alignof(V), values_align);

            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;

                u32 bucket_index = ((u128) table.hashes[i] * (u128) new_capacity) >> 64;
                u32 found_index;
                find_free_bucket<K, V>(new_hashes, new_capacity, bucket_index, &found_index);

                new_keys[found_index]   = table.keys[i];
                new_values[found_index] = table.values[i];
                new_hashes[found_index] = table.hashes[i];
            }

            memory::free(table.alloc, table.hashes);

            table.keys      = new_keys;
            table.values    = new_values;
            table.hashes    = new_hashes;
            table.capacity  = new_capacity;
        }

        template<typename K, typename V> u0 reserve(hashtable_t<K, V>& table, u32 new_capacity) {
            rehash(table, std::ceil(std::max<u32>(16, new_capacity) / std::min(.5f, table.load_factor)));
        }

        template<typename K, typename V,
            typename B=std::remove_pointer_t<V>,
            b8 IsPtr=std::is_pointer<V>::value>
        B* insert(hashtable_t<K, V>& table, const K& key, const V& value) {
            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            u64 hash                    = hashing::hash64(key);
            u32 bucket_index            = ((u128) hash * (u128) table.capacity) >> 64;
            u32 found_index;

            if (find_free_bucket<K, V>(table.hashes, table.capacity, bucket_index, &found_index)) {
                table.keys[found_index] = key;
                table.hashes[found_index] = hash;
                table.values[found_index] = value;
                ++table.size;
                if constexpr (IsPtr) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return nullptr;
        }

        template <typename K, typename V> b8 find_free_bucket(u64* hashes, u32 size, u32 start, u32* found) {
            for (u32 i = start; i < size; ++i) {
                if (!hashes[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!hashes[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }

        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc, f32 load_factor) {
            table.keys          = {};
            table.values        = {};
            table.hashes        = {};
            table.alloc         = alloc;
            table.load_factor   = load_factor;
            table.size = table.capacity = {};
        }

        template<typename K, typename V> b8 find_key(hashtable_t<K, V>& table, u32 start, u64 hash, const K& key, u32* found) {
            for (u32 i = start; i < table.capacity; ++i) {
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }
    }
}
