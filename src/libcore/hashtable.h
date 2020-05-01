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
#include <basecode/core/str.h>
#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/format.h>
#include <basecode/core/context.h>
#include <basecode/core/memory/memory.h>
#include <basecode/core/hashing/hashable.h>
#include <basecode/core/memory/bump_system.h>

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

        u8*                     buf;
        state_t*                states;
        u64*                    hashes;
        K*                      keys;
        V*                      values;
        alloc_t*                alloc;
        alloc_t*                bump_alloc;
        u32                     size;
        u32                     capacity;
        u32                     weighted_capacity;
    };

    namespace hashtable {
        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table);
        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>& table);
        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity);
        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* alloc = context::top()->alloc);
        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc = context::top()->alloc);
        template<typename K, typename V> b8 find_key(hashtable_t<K, V>& table, u32 start, u64 hash, const K& key, u32* found);
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
            using state_t = typename hashtable_t<K, V>::state_t;

            if (table.buf)
                std::memset(table.states, 0, sizeof(state_t) * table.capacity);
            table.size = {};
        }

        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table) {
            memory::free(table.alloc, table.buf);
            memory::system::free(table.bump_alloc);
            table.buf = {};
            table.keys = {};
            table.values = {};
            table.hashes = {};
            table.states = {};
            table.capacity = table.size = {};
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
            return table.capacity == 0 || table.size > table.weighted_capacity;
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

        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* alloc) {
            table.buf = {};
            table.keys = {};
            table.values = {};
            table.hashes = {};
            table.states = {};
            table.alloc = alloc;

            bump_config_t config{};
            config.type = bump_type_t::existing;
            table.bump_alloc = memory::system::make(alloc_type_t::bump, &config);

            table.size = table.capacity = table.weighted_capacity = {};
        }

        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity) {
            using state_t = typename hashtable_t<K, V>::state_t;

            new_capacity = std::max(std::max(new_capacity, table.size), (u32) 8);

            auto buf_size = ((sizeof(K) * new_capacity) + alignof(K))
                            + ((sizeof(V) * new_capacity) + alignof(V))
                            + ((sizeof(u64) * new_capacity) + alignof(u64))
                            + sizeof(state_t) * new_capacity;

            auto new_buf = (u8*) memory::alloc(table.alloc, buf_size);
            memory::bump::buf(table.bump_alloc, new_buf, buf_size);

            auto new_states = (state_t*) memory::alloc(table.bump_alloc, new_capacity * sizeof(state_t));
            auto new_hashes = (u64*) memory::alloc(table.bump_alloc, new_capacity * sizeof(u64), alignof(u64));
            auto new_keys = (K*) memory::alloc(table.bump_alloc, new_capacity * sizeof(K), alignof(K));
            auto new_values = (V*) memory::alloc(table.bump_alloc, new_capacity * sizeof(V), alignof(V));

            std::memset(new_states, 0, new_capacity * sizeof(state_t));

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

            memory::free(table.alloc, table.buf);

            table.buf = new_buf;
            table.keys = new_keys;
            table.values = new_values;
            table.states = new_states;
            table.hashes = new_hashes;
            table.capacity = new_capacity;
            table.weighted_capacity = new_capacity - (new_capacity * .15f);
        }

        template<typename K, typename V> u0 reserve(hashtable_t<K, V>& table, u32 new_capacity) {
            rehash(table, new_capacity * 3);
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

        // XXX: this should be working, but isn't.
        //      investigate before heat death of universe.
        force_inline u32 fast_range(u32 x, u32 y) {
            return ((u64) x * (u64) y) >> 32;
        }

        template<typename K, typename V> decltype(auto) emplace(hashtable_t<K, V>& table, const K& key) {
            using state_t = typename hashtable_t<K, V>::state_t;

            if (requires_rehash(table))
                rehash(table, table.capacity * 3);

            u64 hash = hashing::hash64(key);
            u32 bucket_index = hash % table.capacity;
            u32 found_index{};

            find_free_bucket<K ,V>(table.states, table.capacity, bucket_index, &found_index);

            table.keys[found_index] = key;
            table.hashes[found_index] = hash;
            table.states[found_index] = state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }

        template<typename K, typename V> decltype(auto) insert(hashtable_t<K, V>& table, const K& key, const V& value) {
            using state_t = typename hashtable_t<K, V>::state_t;

            if (requires_rehash(table))
                rehash(table, table.capacity * 3);

            u64 hash = hashing::hash64(key);
            u32 bucket_index = hash % table.capacity;
            u32 found_index{};

            find_free_bucket<K, V>(table.states, table.capacity, bucket_index, &found_index);

            table.keys[found_index] = key;
            table.hashes[found_index] = hash;
            table.values[found_index] = value;
            table.states[found_index] = state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }
    }
}
