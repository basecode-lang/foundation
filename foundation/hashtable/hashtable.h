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
#include <foundation/types.h>
#include <foundation/memory/system.h>
#include <foundation/hashing/hashable.h>

#pragma once

namespace basecode::hashtable {
    enum class bucket_state_t : u8 {
        empty,
        filled,
        removed
    };

    struct bucket_t final {
        u64*                    hashes;
        bucket_state_t*         states;
    };

    template <typename K, typename V>
    struct table_t final {
        u8*                     buf{};
        u32                     size{};
        K*                      keys{};
        V*                      values{};
        bucket_t                buckets{};
        u32                     capacity{};
        memory::allocator_t*    allocator{};

        ~table_t();
    };

    template<typename K, typename V> u0 clear(table_t<K, V>&);
    template <typename K, typename V> u0 rehash(table_t<K, V>&, u32);
    template <typename K, typename V> inline b8 requires_rehash(table_t<K, V>&);
    static inline b8 find_free_bucket(bucket_state_t*, u32, u32, u32*);
    template <typename K, typename V> static inline b8 find_key(table_t<K, V>&, u32, u64, const K&, u32*);

    template <typename K, typename V>
    u0 init(
            table_t<K, V>& table,
            memory::allocator_t* allocator = context::current()->allocator) {
        table.allocator = allocator;
        table.keys = {};
        table.values = {};
        table.buckets = {};
        table.size = table.capacity = {};
    }

    template <typename K, typename V>
    u0 free(table_t<K, V>& table) {
        clear(table);
    }

    template <typename K, typename V>
    u0 clear(table_t<K, V>& table) {
        memory::deallocate(table.allocator, table.buf);
        table.buf = {};
        table.keys = {};
        table.values = {};
        table.buckets.hashes = {};
        table.buckets.states = {};
        table.capacity = table.size = {};
    }

    template <typename K, typename V>
    u0 reset(table_t<K, V>& table) {
        table.size = {};
    }

    template <typename K, typename V>
    inline b8 empty(table_t<K, V>& table) {
        return table.size == 0;
    }

    template <typename K, typename V>
    b8 remove(table_t<K, V>& table, const K& key) {
        if (table.capacity == 0)
            return false;
        auto hash = hashing::hash64(key);
        auto bucket_index = hash % table.capacity;
        u32 found_index{};
        if (!find_key(table, bucket_index, hash, key, &found_index))
            return false;
        table.buckets.states[found_index] = bucket_state_t::removed;
        --table.size;
        return true;
    }

    template <typename K, typename V>
    decltype(auto) find(table_t<K, V>& table, const K& key) {
        b8 found{};
        u32 found_index{};

        if (table.capacity > 0) {
            auto hash = hashing::hash64(key);
            auto bucket_index = hash % table.capacity;
            found = find_key(table, bucket_index, hash, key, &found_index);
        }

        if constexpr (std::is_pointer_v<V>) {
            return !found ? nullptr : table.values[found_index];
        } else {
            return !found ? nullptr : &table.values[found_index];
        }
    }

    template <typename K, typename V>
    u0 insert(table_t<K, V>& table, const K& key, const V& value) {
        if (requires_rehash(table))
            rehash(table, table.capacity * 2);

        auto hash = hashing::hash64(key);
        auto bucket_index = hash % table.capacity;
        u32 found_index{};
        auto found = find_free_bucket(
            table.buckets.states,
            table.capacity,
            bucket_index,
            &found_index);
        assert(found);

        table.keys[found_index] = key;
        table.values[found_index] = value;
        table.buckets.hashes[found_index] = hash;
        table.buckets.states[found_index] = bucket_state_t::filled;
        ++table.size;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename K, typename V>
    static inline b8 find_key(
            table_t<K, V>& table,
            u32 start,
            u64 hash,
            const K& key,
            u32* found) {
        for (u32 i = start; i < table.capacity; ++i) {
            const auto& state = table.buckets.states[i];
            switch (state) {
                case bucket_state_t::empty:
                    return false;
                case bucket_state_t::filled: {
                    if (hash == table.buckets.hashes[i]
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
            const auto& state = table.buckets.states[i];
            switch (state) {
                case bucket_state_t::empty:
                    return false;
                case bucket_state_t::filled:
                    if (hash == table.buckets.hashes[i]
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

    static inline b8 find_free_bucket(
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

    template <typename K, typename V>
    b8 requires_rehash(table_t<K, V>& table) {
        return table.capacity == 0 || table.size * 3 > table.capacity * 2;
    }

    template <typename K, typename V>
    u0 rehash(table_t<K, V>& table, u32 new_capacity) {
        new_capacity = std::max(std::max(new_capacity, table.size), (u32) 8);

        const auto buf_size = ((sizeof(K) * new_capacity) + alignof(K))
            + ((sizeof(V) * new_capacity) + alignof(V))
            + ((sizeof(u64) * new_capacity) + alignof(u64))
            + sizeof(bucket_state_t) * new_capacity;
        auto new_buf = (u8*) memory::allocate(table.allocator, buf_size);
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
            table.capacity = new_capacity;

            table.keys = new_keys;
            table.values = new_values;
            table.buckets.states = new_states;
            table.buckets.hashes = new_hashes;
            return;
        }

        if (table.capacity > 0) {
            u32 cursor{};
            for (u32 i = 0; i < table.capacity; ++i) {
                if (table.buckets.states[i] != bucket_state_t::filled) {
                    ++cursor;
                    continue;
                }

                u64 hash = table.buckets.hashes[i];
                auto bucket_index = hash % new_capacity;
                u32 found_index{};
                auto found = find_free_bucket(
                    new_states,
                    new_capacity,
                    bucket_index,
                    &found_index);
                assert(found);

                new_keys[found_index]   = table.keys[cursor];
                new_values[found_index] = table.values[cursor];
                new_states[found_index] = bucket_state_t::filled;
                new_hashes[found_index] = table.buckets.hashes[cursor];

                ++cursor;
            }

            memory::deallocate(table.allocator, table.buf);

            table.buf = new_buf;
            table.capacity = new_capacity;

            table.keys = new_keys;
            table.values = new_values;
            table.buckets.states = new_states;
            table.buckets.hashes = new_hashes;
        }
    }

    template <typename K, typename V>
    table_t<K,V> make(memory::allocator_t* allocator = context::current()->allocator) {
        table_t<K,V> table;
        init(table);
        return table;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename K, typename V>
    inline table_t<K, V>::~table_t() {
        clear(*this);
    }
}