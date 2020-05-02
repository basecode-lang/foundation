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
    // XXX: need to enable configuration of hashing algorithm/size
    template <typename K, typename V>
    struct hashtable_t final {
        enum class state_t : u8 {
            empty,
            filled,
            removed
        };

        state_t*                states;
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
        template<typename K, typename V> b8 find_key(hashtable_t<K, V>& table, u32 start, u64 hash, const K& key, u32* found);
        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc = context::top()->alloc, f32 load_factor = .5f);
        template <typename K, typename V> b8 find_free_bucket(typename hashtable_t<K, V>::state_t* states, u32 states_size, u32 start, u32* found);

        template<typename K, typename V> consteval auto make_key_array(hashtable_t<K, V>& table) {
            if constexpr (std::is_pointer_v<K>) {
                return array_t<K>{};
            } else {
                return array_t<K*>{};
            }
        }

        template<typename K, typename V> consteval auto make_value_array(hashtable_t<K, V>& table) {
            if constexpr (std::is_pointer_v<V>) {
                return array_t<V>{};
            } else {
                return array_t<V*>{};
            }
        }

        template<typename K, typename V> b8 find_key(
                hashtable_t<K, V>& table,
                u32 start,
                u64 hash,
                const K& key,
                u32* found) {
            using state_t = typename hashtable_t<K, V>::state_t;

            for (u32 i = start; i < table.capacity; ++i) {
                const auto& state = table.states[i];
                switch (state) {
                    case state_t::empty:
                        return false;
                    case state_t::filled: {
                        if (hash == table.hashes[i]
                        &&  key == table.keys[i]) {
                            *found = i;
                            return true;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                const auto& state = table.states[i];
                switch (state) {
                    case state_t::empty:
                        return false;
                    case state_t::filled:
                        if (hash == table.hashes[i]
                        &&  key == table.keys[i]) {
                            *found = i;
                            return true;
                        }
                        break;
                    default:
                        break;
                }
            }
            return false;
        }

        template <typename K, typename V> b8 find_free_bucket(
                typename hashtable_t<K, V>::state_t* states,
                u32 states_size,
                u32 start,
                u32* found) {
            using state_t = typename hashtable_t<K, V>::state_t;

            for (u32 i = start; i < states_size; ++i) {
                if (states[i] != state_t::filled) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (states[i] != state_t::filled) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }

        template<typename K, typename V> u0 free(hashtable_t<K, V>& table) {
            clear(table);
        }

        template<typename K, typename V> u0 reset(hashtable_t<K, V>& table) {
            if (table.states) std::memset(table.states, 0, table.capacity);
            table.size = {};
        }

        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table) {
            memory::free(table.alloc, table.states);
            table.keys      = {};
            table.values    = {};
            table.hashes    = {};
            table.states    = {};
            table.capacity  = table.size = {};
        }

        template<typename K, typename V> auto keys(hashtable_t<K, V>& table) {
            using state_t = typename hashtable_t<K, V>::state_t;

            auto list = make_key_array(table);
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (table.states[i] == state_t::filled) {
                    if constexpr(std::is_pointer_v<K>)
                        array::append(list, table.keys[i]);
                    else
                        array::append(list, &table.keys[i]);
                }
            }
            return list;
        }

        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* alloc) {
            hashtable_t<K, V> table;
            init(table, alloc);
            return table;
        }

        template<typename K, typename V> inline b8 empty(hashtable_t<K, V>& table) {
            return table.size == 0;
        }

        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>& table) {
            return table.capacity == 0 || table.size + 1 > (table.capacity - 1) * table.load_factor;
        }

        template<typename K, typename V> decltype(auto) values(hashtable_t<K, V>& table) {
            using state_t = typename hashtable_t<K, V>::state_t;

            auto list = make_value_array(table);
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (table.states[i] == state_t::filled) {
                    if constexpr(std::is_pointer_v<V>)
                        array::append(list, table.values[i]);
                    else
                        array::append(list, &table.values[i]);
                }
            }
            return list;
        }

        template<typename K, typename V> b8 remove(hashtable_t<K, V>& table, const K& key) {
            if (table.capacity == 0)
                return false;
            auto hash = hashing::hash64(key);
            auto bucket_index = hash % table.capacity;
            u32 found_index{};
            if (!find_key(table, bucket_index, hash, key, &found_index))
                return false;
            table.states[found_index] = hashtable_t<K, V>::state_t::removed;
            --table.size;
            return true;
        }

        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity) {
            using state_t = typename hashtable_t<K, V>::state_t;

            new_capacity = std::max<u32>(new_capacity, std::ceil(std::max<u32>(16, new_capacity) / table.load_factor));

            const auto size_of_states = new_capacity;
            const auto size_of_keys   = sizeof(K) * new_capacity;
            const auto size_of_values = sizeof(V) * new_capacity;
            const auto size_of_hashes = sizeof(u64) * new_capacity;

            const auto buf_size = size_of_states + (alignof(K) + size_of_keys) + (alignof(V) + size_of_values) + (alignof(u64) + size_of_hashes);

            u8* buf = (u8*) memory::alloc(table.alloc, buf_size);
            std::memset(buf, 0, size_of_states);
            state_t* new_states = (state_t*) buf;

            u32 align{};
            auto new_keys   = (K*)   memory::system::align_forward(buf + size_of_states, alignof(K), align);
            auto new_values = (V*)   memory::system::align_forward(buf + size_of_states + size_of_keys + align, alignof(V), align);
            auto new_hashes = (u64*) memory::system::align_forward(buf + size_of_states + size_of_keys + size_of_values + align, alignof(u64), align);

            for (u32 i = 0; i < table.capacity; ++i) {
                if (table.states[i] != state_t::filled)
                    continue;

                u64 hash = table.hashes[i];
                u32 bucket_index = hash % new_capacity;
                u32 found_index{};
                find_free_bucket<K, V>(new_states, new_capacity, bucket_index, &found_index);

                new_keys[found_index] = table.keys[i];
                new_values[found_index] = table.values[i];
                new_hashes[found_index] = table.hashes[i];
                new_states[found_index] = state_t::filled;
            }

            memory::free(table.alloc, table.states);

            table.keys      = new_keys;
            table.values    = new_values;
            table.states    = new_states;
            table.hashes    = new_hashes;
            table.capacity  = new_capacity;
        }

        template<typename K, typename V> u0 reserve(hashtable_t<K, V>& table, u32 new_capacity) {
            rehash(table, std::ceil(std::max<u32>(16, new_capacity) / std::min(.5f, table.load_factor)));
        }

        template<typename K, typename V> decltype(auto) find(hashtable_t<K, V>& table, const K& key) {
            u32 found_index{};
            b8 found{};

            if (table.capacity > 0) {
                u64 hash = hashing::hash64(key);
                u32 bucket_index = hash % table.capacity;
                found = find_key(table, bucket_index, hash, key, &found_index);
            }

            if constexpr (std::is_pointer_v<V>) {
                return !found ? nullptr : table.values[found_index];
            } else {
                return !found ? nullptr : &table.values[found_index];
            }
        }

        template<typename K, typename V> decltype(auto) emplace(hashtable_t<K, V>& table, const K& key) {
            using state_t = typename hashtable_t<K, V>::state_t;

            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            u64 hash = hashing::hash64(key);
            u32 bucket_index = hash % table.capacity;
            u32 found_index{};

            find_free_bucket<K ,V>(table.states, table.capacity, bucket_index, &found_index);

            table.keys[found_index]     = key;
            table.hashes[found_index]   = hash;
            table.states[found_index]   = state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }

        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc, f32 load_factor) {
            table.keys          = {};
            table.values        = {};
            table.hashes        = {};
            table.states        = {};
            table.alloc         = alloc;
            table.load_factor   = load_factor;
            table.size = table.capacity = {};
        }

        template<typename K, typename V> decltype(auto) insert(hashtable_t<K, V>& table, const K& key, const V& value) {
            using state_t = typename hashtable_t<K, V>::state_t;

            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            u64 hash = hashing::hash64(key);
            u32 bucket_index = hash % table.capacity;
            u32 found_index{};

            find_free_bucket<K, V>(table.states, table.capacity, bucket_index, &found_index);

            table.keys[found_index]     = key;
            table.hashes[found_index]   = hash;
            table.values[found_index]   = value;
            table.states[found_index]   = state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }
    }
}
