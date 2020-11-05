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

#include <cassert>
#include <algorithm>
#include <basecode/core/str.h>
#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/memory.h>
#include <basecode/core/format.h>
#include <basecode/core/locale.h>
#include <basecode/core/context.h>
#include <basecode/core/hashable.h>

namespace basecode {
    template <typename T>
    concept Hash_Table = hash::Hashable<typename T::key_type> && requires(const T& t) {
        {t.hashes}              -> same_as<u64*>;
        {t.keys}                -> same_as<typename T::key_type*>;
        {t.values}              -> same_as<typename T::value_type*>;
        {t.alloc}               -> same_as<alloc_t*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
        {t.load_factor}         -> same_as<f32>;
    };

    template <typename K, typename V>
    struct hashtab_t final {
        using key_type          = K;
        using value_type        = V;
        using each_callback     = b8 (*)(const K&, V&, u0*);

        u64*                    hashes;
        K*                      keys;
        V*                      values;
        alloc_t*                alloc;
        u32                     size;
        u32                     capacity;
        f32                     load_factor;
    };
    static_assert(sizeof(hashtab_t<s32, s32>) <= 48, "hashtab_t<K, V> is now larger than 48 bytes!");

    namespace hashtab {
        u0 clear(Hash_Table auto& table);

        template<typename K>
        b8 find_key(const Hash_Table auto& table,
                    u32 start,
                    u64 hash,
                    const K& key,
                    u32* found);

        u0 init(Hash_Table auto& table,
                alloc_t* alloc = context::top()->alloc,
                f32 load_factor = .5f);

        template <typename T> requires Hash_Table<T>
        u0 rehash(T& table, u32 new_capacity = 16);

        inline b8 find_free_bucket(const u64* hashes,
                                   u32 size,
                                   u32 start,
                                   u32* found);

        inline b8 requires_rehash(u32 capacity, u32 size, f32 load_factor);

        template<typename T> requires Hash_Table<T>
        auto keys(T& table) {
            using Array = array_t<std::remove_pointer_t<typename T::key_type>*>;

            Array list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (std::is_pointer_v<typename T::key_type>) {
                    array::append(list, table.keys[i]);
                } else {
                    array::append(list, &table.keys[i]);
                }
            }
            return list;
        }

        template<typename T> requires Hash_Table<T>
        auto values(T& table) {
            using Array = array_t<std::remove_pointer_t<typename T::value_type>*>;

            Array list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (std::is_pointer_v<typename T::value_type>) {
                    array::append(list, table.values[i]);
                } else {
                    array::append(list, &table.values[i]);
                }
            }
            return list;
        }

        template<typename T> requires Hash_Table<T>
        auto find(T& table, const auto& key) {
            using V = std::remove_pointer_t<typename T::value_type>*;

            if (table.capacity <= 0) return V(nullptr);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;
            u32 found_index;
            if (find_key(table, bucket_index, hash, key, &found_index)) {
                if constexpr (std::is_pointer_v<typename T::value_type>) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return V(nullptr);
        }

        template<typename T> requires Hash_Table<T>
        auto emplace(T& table, const auto& key) {
            using Value_Type = std::remove_pointer_t<typename T::value_type>*;

            auto v = find(table, key);
            if (v)
                return Value_Type(v);

            if (requires_rehash(table.capacity, table.size, table.load_factor))
                rehash(table, table.capacity << 1);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;
            u32 found_index;

            if (find_free_bucket(table.hashes, table.capacity, bucket_index, &found_index)) {
                table.keys[found_index]   = key;
                table.hashes[found_index] = hash;
                ++table.size;
                if constexpr (std::is_pointer_v<typename T::value_type>) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return Value_Type(nullptr);
        }

        u0 free(Hash_Table auto& table) {
            clear(table);
        }

        u0 reset(Hash_Table auto& table) {
            if (table.hashes)
                std::memset(table.hashes, 0, table.capacity * sizeof(u64));
            table.size = {};
        }

        u0 clear(Hash_Table auto& table) {
            memory::free(table.alloc, table.hashes);
            table.keys      = {};
            table.values    = {};
            table.hashes    = {};
            table.capacity  = table.size = {};
        }

        template<typename K>
        b8 find_key(const Hash_Table auto& table,
                    u32 start,
                    u64 hash,
                    const K& key,
                    u32* found) {
            for (u32 i = start; i < table.capacity; ++i) {
                if (!table.hashes[i]) return false;
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!table.hashes[i]) return false;
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }

        inline b8 find_free_bucket(const u64* hashes,
                                   u32 size,
                                   u32 start,
                                   u32* found) {
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

        inline b8 empty(const Hash_Table auto& table) {
            return table.size == 0;
        }

        template<typename K>
        b8 remove(Hash_Table auto& table, const K& key) {
            if (table.capacity == 0)
                return false;
            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;
            u32 found_index;
            if (!find_key(table, bucket_index, hash, key, &found_index))
                return false;
            table.hashes[found_index] = 0;
            --table.size;
            return true;
        }

        template<typename T> requires Hash_Table<T>
        auto insert(T& table, const auto& key, const auto& value) {
            using Value_Type = std::remove_pointer_t<typename T::value_type>*;

            auto v = find(table, key);
            if (v)
                return Value_Type(v);

            if (requires_rehash(table.capacity, table.size, table.load_factor))
                rehash(table, table.capacity << 1);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;
            u32 found_index;

            if (find_free_bucket(table.hashes, table.capacity, bucket_index, &found_index)) {
                table.keys[found_index]   = key;
                table.hashes[found_index] = hash;
                table.values[found_index] = value;
                ++table.size;
                if constexpr (std::is_pointer_v<typename T::value_type>) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return Value_Type(nullptr);
        }

        template<typename T> requires Hash_Table<T>
        u0 rehash(T& table, u32 new_capacity) {
            using K = typename T::key_type;
            using V = typename T::value_type;

            new_capacity = std::max<u32>(new_capacity,
                                         std::ceil(std::max<u32>(16, new_capacity) / table.load_factor));

            const auto size_of_hashes = new_capacity * sizeof(u64);
            const auto size_of_keys   = new_capacity * sizeof(K);
            const auto size_of_values = new_capacity * sizeof(V);

            const auto buf_size = size_of_hashes
                + alignof(K) + size_of_keys
                + alignof(V) + size_of_values;

            auto buf = (u8*) memory::alloc(table.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, size_of_hashes);
            auto new_hashes = (u64*) buf;

            u32 keys_align{}, values_align{};
            auto new_keys   = (K*) memory::system::align_forward(buf + size_of_hashes,
                                                                 alignof(K),
                                                                 keys_align);
            auto new_values = (V*) memory::system::align_forward(buf + size_of_hashes + size_of_keys + keys_align,
                                                                 alignof(V),
                                                                 values_align);

            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;

                u32 bucket_index = u128(table.hashes[i]) * u128(new_capacity) >> 64U;
                u32 found_index;
                find_free_bucket(new_hashes, new_capacity, bucket_index, &found_index);

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

        u0 reserve(Hash_Table auto& table, u32 new_capacity) {
            rehash(table,
                   std::ceil(std::max<u32>(16, new_capacity) / std::min(.5f, table.load_factor)));
        }

        u0 init(Hash_Table auto& table, alloc_t* alloc, f32 load_factor) {
            table.keys          = {};
            table.values        = {};
            table.hashes        = {};
            table.alloc         = alloc;
            table.load_factor   = load_factor;
            table.size = table.capacity = {};
        }

        template<typename T> requires Hash_Table<T>
        b8 for_each_pair(T& table, typename T::each_callback cb, u0* user = {}) {
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if (!cb(table.keys[i], table.values[i], user))
                    return false;
            }
            return true;
        }

        inline b8 requires_rehash(u32 capacity, u32 size, f32 load_factor) {
            return capacity == 0 || f32(size) + 1 > f32(capacity - 1) * load_factor;
        }
    }
}
