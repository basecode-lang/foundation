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
#include <basecode/core/types.h>
#include <basecode/core/string/str.h>
#include <basecode/core/array/array.h>
#include <basecode/core/memory/system.h>
#include <basecode/core/context/system.h>
#include <basecode/core/hashing/hashable.h>

#pragma once

namespace basecode {
    enum class bucket_state_t : u8 {
        empty,
        filled,
        removed
    };

    template <typename K, typename V>
    struct hashtable_t final {
        u8*                     buf;
        bucket_state_t*         states;
        u64*                    hashes;
        K*                      keys;
        V*                      values;
        alloc_t*                allocator;
        u32                     size;
        u32                     capacity;
    };

    namespace hashtable {
        template<typename K, typename V> u0 clear(hashtable_t<K, V>&);
        template<typename K, typename V> u32 buffer_size(u32 capacity);
        template<typename K, typename V> u0 rehash(hashtable_t<K, V>&, u32);
        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>&);
        inline b8 find_free_bucket(bucket_state_t*, u32, u32, u32*);
        template<typename K, typename V> b8 find_key(hashtable_t<K, V>&, u32, u64, const K&, u32*);
        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* allocator = context::top()->alloc);
        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* allocator = context::top()->alloc);

        template<typename K, typename V> b8 find_key(
                hashtable_t<K, V>& table,
                u32 start,
                u64 hash,
                const K& key,
                u32* found) {
            for (u32 i = start; i < table.capacity; ++i) {
                const auto& state = table.states[i];
                switch (state) {
                    case bucket_state_t::empty:
                        return false;
                    case bucket_state_t::filled: {
                        if (hash == table.hashes[i]
                            && key == table.keys[i]) {
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
                    case bucket_state_t::empty:
                        return false;
                    case bucket_state_t::filled:
                        if (hash == table.hashes[i]
                            && key == table.keys[i]) {
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

        inline b8 find_free_bucket(
                bucket_state_t* states,
                u32 states_size,
                u32 start,
                u32* found) {
            for (u32 i = start; i < states_size; ++i) {
                if (states[i] != bucket_state_t::filled) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (states[i] != bucket_state_t::filled) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }

        template<typename K, typename V> u32 buffer_size(u32 capacity) {
            return ((sizeof(K) * capacity) + alignof(K))
                   + ((sizeof(V) * capacity) + alignof(V))
                   + ((sizeof(u64) * capacity) + alignof(u64))
                   + sizeof(bucket_state_t) * capacity;
        }

        template<typename K, typename V> u0 free(hashtable_t<K, V>& table) {
            clear(table);
        }

        template<typename K, typename V> u0 reset(hashtable_t<K, V>& table) {
            std::memset(table.buf, 0, buffer_size<K, V>(table.capacity));
            table.size = {};
        }

        template<typename K, typename V> u0 clear(hashtable_t<K, V>& table) {
            memory::free(table.allocator, table.buf);
            table.buf = {};
            table.keys = {};
            table.values = {};
            table.hashes = {};
            table.states = {};
            table.capacity = table.size = {};
        }

        template<typename K, typename V> inline b8 empty(hashtable_t<K, V>& table) {
            return table.size == 0;
        }

        template<typename K, typename V> hashtable_t<K, V> make(alloc_t* allocator) {
            hashtable_t<K, V> table;
            init(table, allocator);
            return table;
        }

        template<typename K, typename V> b8 requires_rehash(hashtable_t<K, V>& table) {
            return table.capacity == 0 || table.size * 3 > table.capacity * 2;
        }

        template<typename K, typename V> decltype(auto) keys(hashtable_t<K, V>& table) {
            if constexpr (std::is_pointer_v<K>) {
                array_t<K> list;
                array::init(list, table.allocator);
                array::reserve(list, table.size);
                for (u32 i = 0; i < table.capacity; ++i) {
                    if (table.states[i] == bucket_state_t::filled) {
                        array::append(list, table.keys[i]);
                    }
                }
                return list;
            } else {
                array_t<K*> list;
                array::init(list, table.allocator);
                array::reserve(list, table.size);
                for (u32 i = 0; i < table.capacity; ++i) {
                    if (table.states[i] == bucket_state_t::filled) {
                        array::append(list, &table.keys[i]);
                    }
                }
                return list;
            }
        }

        template<typename K, typename V> decltype(auto) values(hashtable_t<K, V>& table) {
            if constexpr (std::is_pointer_v<V>) {
                array_t<V> list;
                array::init(list, table.allocator);
                array::reserve(list, table.size);
                for (u32 i = 0; i < table.capacity; ++i) {
                    if (table.states[i] == bucket_state_t::filled) {
                        array::append(list, table.values[i]);
                    }
                }
                return list;
            } else {
                array_t<V*> list;
                array::init(list, table.allocator);
                array::reserve(list, table.size);
                for (u32 i = 0; i < table.capacity; ++i) {
                    if (table.states[i] == bucket_state_t::filled) {
                        array::append(list, &table.values[i]);
                    }
                }
                return list;
            }
        }

        template<typename K, typename V> b8 remove(hashtable_t<K, V>& table, const K& key) {
            if (table.capacity == 0)
                return false;
            auto hash = hashing::hash64(key);
            auto bucket_index = hash * table.capacity >> (u32) 32;
            u32 found_index{};
            if (!find_key(table, bucket_index, hash, key, &found_index))
                return false;
            table.states[found_index] = bucket_state_t::removed;
            --table.size;
            return true;
        }

        template<typename K, typename V> u0 init(hashtable_t<K, V>& table, alloc_t* allocator) {
            table.buf = {};
            table.keys = {};
            table.values = {};
            table.hashes = {};
            table.states = {};
            table.allocator = allocator;
            table.size = table.capacity = {};
        }

        template<typename K, typename V> u0 rehash(hashtable_t<K, V>& table, u32 new_capacity) {
            new_capacity = std::max(std::max(new_capacity, table.size), (u32) 8);
            const auto buf_size = buffer_size<K, V>(new_capacity);
            auto new_buf = (u8*) memory::alloc(table.allocator, buf_size);
            auto new_states = (bucket_state_t*) new_buf;
            auto new_hashes = (u64*) memory::align_forward(
                (u8*) new_states + (sizeof(bucket_state_t) * new_capacity),
                sizeof(u64));
            auto new_keys = (K*) memory::align_forward(
                (u8*) new_hashes + (sizeof(u64) * new_capacity),
                alignof(K));
            auto new_values = (V*) memory::align_forward(
                (u8*) new_keys + (sizeof(K) * new_capacity),
                alignof(V));

            std::memset(new_states, 0, buf_size);

            if (table.capacity == 0) {
                table.buf = new_buf;
                table.keys = new_keys;
                table.values = new_values;
                table.states = new_states;
                table.hashes = new_hashes;
                table.capacity = new_capacity;
                return;
            }

            if (table.capacity > 0) {
                u32 cursor{};
                for (u32 i = 0; i < table.capacity; ++i) {
                    if (table.states[i] != bucket_state_t::filled) {
                        ++cursor;
                        continue;
                    }

                    u64 hash = table.hashes[i];
                    auto bucket_index = hash * new_capacity >> (u32) 32;
                    u32 found_index{};
                    auto found = find_free_bucket(
                        new_states,
                        new_capacity,
                        bucket_index,
                        &found_index);
                    assert(found);
                    (void) found;

                    new_keys[found_index] = table.keys[cursor];
                    new_values[found_index] = table.values[cursor];
                    new_hashes[found_index] = table.hashes[cursor];
                    new_states[found_index] = bucket_state_t::filled;

                    ++cursor;
                }

                memory::free(table.allocator, table.buf);

                table.buf = new_buf;
                table.capacity = new_capacity;

                table.keys = new_keys;
                table.values = new_values;
                table.states = new_states;
                table.hashes = new_hashes;
            }
        }

        template<typename K, typename V> u0 reserve(hashtable_t<K, V>& table, u32 new_capacity) {
            rehash(table, new_capacity);
        }

        template<typename K, typename V> decltype(auto) find(hashtable_t<K, V>& table, const K& key) {
            b8 found{};
            u32 found_index{};

            if (table.capacity > 0) {
                auto hash = hashing::hash64(key);
                auto bucket_index = hash * table.capacity >> (u32) 32;
                found = find_key(table, bucket_index, hash, key, &found_index);
            }

            if constexpr (std::is_pointer_v<V>) {
                return !found ? nullptr : table.values[found_index];
            } else {
                return !found ? nullptr : &table.values[found_index];
            }
        }

        template<typename K, typename V> decltype(auto) emplace(hashtable_t<K, V>& table, const K& key) {
            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            auto hash = hashing::hash64(key);
            auto bucket_index = hash * table.capacity >> (u32) 32;
            u32 found_index{};
            auto found = find_free_bucket(
                table.states,
                table.capacity,
                bucket_index,
                &found_index);
            assert(found);
            (void) found;

            table.keys[found_index] = key;
            table.hashes[found_index] = hash;
            table.states[found_index] = bucket_state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }

        template<typename K, typename V> decltype(auto) insert(hashtable_t<K, V>& table, const K& key, const V& value) {
            if (requires_rehash(table))
                rehash(table, table.capacity * 2);

            auto hash = hashing::hash64(key);
            auto bucket_index = hash * table.capacity >> (u32) 32;
            u32 found_index{};
            auto found = find_free_bucket(
                table.states,
                table.capacity,
                bucket_index,
                &found_index);
            assert(found);
            (void) found;

            table.keys[found_index] = key;
            table.hashes[found_index] = hash;
            table.values[found_index] = value;
            table.states[found_index] = bucket_state_t::filled;
            ++table.size;

            if constexpr (std::is_pointer_v<V>) {
                return table.values[found_index];
            } else {
                return &table.values[found_index];
            }
        }
    }
}