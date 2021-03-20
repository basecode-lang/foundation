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
#include <basecode/core/array.h>
#include <basecode/core/memory.h>
#include <basecode/core/format.h>
#include <basecode/core/locale.h>
#include <basecode/core/context.h>
#include <basecode/core/hashable.h>
#include <basecode/core/hash_common.h>

namespace basecode {
    template <typename T>
    concept Hash_Table = hash::Hashable<typename T::Key_Type> && requires(const T& t) {
        typename                T::Key_Type;
        typename                T::Value_Type;
        typename                T::Each_Callback;

        {t.hashes}              -> same_as<u64*>;
        {t.keys}                -> same_as<typename T::Key_Type*>;
        {t.values}              -> same_as<typename T::Value_Type*>;
        {t.alloc}               -> same_as<alloc_t*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
        {t.load_factor}         -> same_as<f32>;
    };

    template <typename K, typename V>
    struct hashtab_t final {
        using Key_Type          = K;
        using Value_Type        = V;
        using Each_Callback     = u32 (*)(u32, const Key_Type&, Value_Type&, u0*);

        u64*                    hashes;
        Key_Type*               keys;
        Value_Type*             values;
        alloc_t*                alloc;
        u32                     size;
        u32                     capacity;
        f32                     load_factor;
    };
    static_assert(sizeof(hashtab_t<s32, s32>) <= 48, "hashtab_t<K, V> is now larger than 48 bytes!");

    namespace hashtab {
        template <Hash_Table T>
        u0 clear(T& table);

        template <Hash_Table T>
        u0 init(T& table,
                alloc_t* alloc = context::top()->alloc,
                f32 load_factor = .9f);

        template<Hash_Table T, typename Key_Type = typename T::Key_Type>
        b8 find_key(T& table,
                    u32 start,
                    u64 hash,
                    const Key_Type& key,
                    u32* found);

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type,
                  typename Value_Type = typename T::Value_Type>
        u0 rehash(T& table, u32 new_capacity = 16);

        template <Hash_Table T>
        u0 free(T& table) {
            clear(table);
        }

        template <Hash_Table T>
        u0 reset(T& table) {
            if (table.hashes)
                std::memset(table.hashes, 0, table.capacity * sizeof(u64));
            table.size = {};
        }

        template <Hash_Table T>
        u0 clear(T& table) {
            memory::free(table.alloc, table.hashes);
            table.keys     = {};
            table.values   = {};
            table.hashes   = {};
            table.capacity = table.size = {};
        }

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type,
                  b8 Is_Pointer = std::is_pointer_v<Key_Type>>
        auto keys(T& table) {
            using Array = array_t<std::remove_pointer_t<Key_Type>*>;

            Array list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (Is_Pointer) {
                    array::append(list, table.keys[i]);
                } else {
                    array::append(list, &table.keys[i]);
                }
            }
            return list;
        }

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>>
        auto values(T& table) {
            using Array = array_t<std::remove_pointer_t<Value_Type>*>;

            Array list{};
            array::init(list, table.alloc);
            array::reserve(list, table.size);
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                if constexpr (Is_Pointer) {
                    array::append(list, table.values[i]);
                } else {
                    array::append(list, &table.values[i]);
                }
            }
            return list;
        }

        template <Hash_Table T, typename Key_Type>
        b8 find_key(T& table,
                    u32 start,
                    u64 hash,
                    const Key_Type& key,
                    u32* found) {
            for (u32 i = start; i < table.capacity; ++i) {
                if (!table.hashes[i])
                    continue;
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!table.hashes[i])
                    continue;
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }

        template <Hash_Table T>
        inline b8 empty(const T& table) {
            return table.size == 0;
        }

        template <Hash_Table T, typename Key_Type, typename Value_Type>
        u0 rehash(T& table, u32 new_capacity) {
            new_capacity = std::max<u32>(new_capacity,
                                         std::ceil(std::max<u32>(16, new_capacity) / table.load_factor));
            if (new_capacity % 2 != 0)
                new_capacity++;

            const auto size_of_hashes = new_capacity * sizeof(u64);
            const auto size_of_keys   = new_capacity * sizeof(Key_Type);
            const auto size_of_values = new_capacity * sizeof(Value_Type);

            const auto buf_size = size_of_hashes
                                  + alignof(Key_Type) + size_of_keys
                                  + alignof(Value_Type) + size_of_values;

            auto buf = (u8*) memory::alloc(table.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, size_of_hashes);
            auto new_hashes = (u64*) buf;

            u32 keys_align{}, values_align{};
            auto new_keys = (Key_Type*) memory::system::align_forward(buf + size_of_hashes,
                                                                      alignof(Key_Type),
                                                                      keys_align);
            auto new_values = (Value_Type*) memory::system::align_forward(buf + size_of_hashes + size_of_keys + keys_align,
                                                                            alignof(Value_Type),
                                                                         values_align);

            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;

                u32 bucket_index = u128(table.hashes[i]) * u128(new_capacity) >> 64U;
                if (!find_free_bucket(new_hashes, new_capacity, bucket_index))
                    return;

                new_keys[bucket_index]   = table.keys[i];
                new_values[bucket_index] = table.values[i];
                new_hashes[bucket_index] = table.hashes[i];
            }

            memory::free(table.alloc, table.hashes);

            table.keys      = new_keys;
            table.values    = new_values;
            table.hashes    = new_hashes;
            table.capacity  = new_capacity;
        }

        template <Hash_Table T>
        u0 reserve(T& table, u32 new_capacity) {
            rehash(table, std::ceil(std::max<u32>(16, new_capacity) / table.load_factor));
        }

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type>
        b8 remove(T& table, const Key_Type& key) {
            if (table.size == 0 || table.capacity == 0)
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

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Value_Type = std::remove_pointer_t<Value_Type>*>
        Base_Value_Type find(T& table, const auto& key) {
            if (table.capacity <= 0)
                return nullptr;

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;
            u32 found_index{};
            if (find_key(table, bucket_index, hash, key, &found_index)) {
                if constexpr (Is_Pointer) {
                    return table.values[found_index];
                } else {
                    return &table.values[found_index];
                }
            }

            return nullptr;
        }

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Value_Type = std::remove_pointer_t<Value_Type>*>
        Base_Value_Type emplace(T& table, const auto& key) {
            auto v = find(table, key);
            if (v)
                return v;

            if (requires_rehash(table.size, table.capacity, table.load_factor))
                rehash(table, table.capacity << 1);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;

            if (find_free_bucket(table.hashes, table.capacity, bucket_index)) {
                table.keys[bucket_index]   = key;
                table.hashes[bucket_index] = hash;
                ++table.size;
                if constexpr (Is_Pointer) {
                    return table.values[bucket_index];
                } else {
                    return &table.values[bucket_index];
                }
            }

            return nullptr;
        }

        template <Hash_Table T>
        u0 init(T& table, alloc_t* alloc, f32 load_factor) {
            table.keys          = {};
            table.values        = {};
            table.hashes        = {};
            table.alloc         = alloc;
            table.load_factor   = load_factor;
            table.size = table.capacity = {};
        }

        template <Hash_Table T,
                  typename Each_Callback = typename T::Each_Callback>
        u32 for_each_pair(T& table, Each_Callback cb, u0* user = {}) {
            u32 idx{};
            for (u32 i = 0; i < table.capacity; ++i) {
                if (!table.hashes[i]) continue;
                auto rc = cb(idx++, table.keys[i], table.values[i], user);
                if (rc)
                    return rc;
            }
            return true;
        }

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Type_Value = std::remove_pointer_t<Value_Type>*>
        Base_Type_Value insert(T& table, const Key_Type& key, const Value_Type& value) {
            auto v = find(table, key);
            if (v)
                return v;

            if (requires_rehash(table.size, table.capacity, table.load_factor))
                rehash(table, table.capacity << 1);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = u128(hash) * u128(table.capacity) >> 64U;

            if (find_free_bucket(table.hashes, table.capacity, bucket_index)) {
                table.keys[bucket_index]   = key;
                table.hashes[bucket_index] = hash;
                table.values[bucket_index] = value;
                ++table.size;
                if constexpr (Is_Pointer) {
                    return table.values[bucket_index];
                } else {
                    return &table.values[bucket_index];
                }
            }

            return nullptr;
        }
    }
}
