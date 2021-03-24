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
#include <basecode/core/bits.h>
#include <basecode/core/array.h>
#include <basecode/core/memory.h>
#include <basecode/core/locale.h>
#include <basecode/core/context.h>
#include <basecode/core/iterator.h>
#include <basecode/core/hashable.h>
#include <basecode/core/hash_common.h>

namespace basecode {
    template <typename T>
    concept Hash_Table = hash::Hashable<typename T::Key_Type> && requires(const T& t) {
        typename                T::Key_Type;
        typename                T::Pair_Type;
        typename                T::Value_Type;

        {t.flags}              -> same_as<u64*>;
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
        struct pair_t;

        using Key_Type          = K;
        using Pair_Type         = pair_t;
        using Value_Type        = V;

        u64*                    flags;
        u64*                    hashes;
        Key_Type*               keys;
        Value_Type*             values;
        alloc_t*                alloc;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        Value_Type& operator[](u32 bucket_idx) {
            return values[bucket_idx];
        }

        struct pair_t final {
            Key_Type            key;
            Value_Type          value;
            u32                 bucket_idx;
        };

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const hashtab_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const hashtab_t* ref) {
                while (pos < ref->capacity) {
                    if (hash_common::read_flag(ref->flags, ++pos))
                        break;
                }
            }

            inline Pair_Type get(hashtab_t* ref) {
                pair_t p{};
                p.key        = ref->keys[pos];
                p.value      = ref->values[pos];
                p.bucket_idx = pos;
                return p;
            }

            inline u0 begin(const hashtab_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (hash_common::read_flag(ref->flags, pos))
                        break;
                    ++pos;
                }
            }

            inline const Pair_Type get(const hashtab_t* ref) {
                pair_t p{};
                p.key        = ref->keys[pos];
                p.value      = ref->values[pos];
                p.bucket_idx = pos;
                return p;
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(hashtab_t, Pair_Type, iterator_state_t);
    };
    static_assert(sizeof(hashtab_t<s32, s32>) <= 56, "hashtab_t<K, V> is now larger than 56 bytes!");

    namespace hashtab {
        template <Hash_Table T>
        u0 clear(T& table);

        template <Hash_Table T>
        u0 init(T& table,
                alloc_t* alloc = context::top()->alloc,
                f32 load_factor = .75f);

        template<Hash_Table T, typename Key_Type = typename T::Key_Type>
        b8 find_key(T& table,
                    u32 start,
                    u64 hash,
                    const Key_Type& key,
                    u32* found);

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type,
                  typename Value_Type = typename T::Value_Type>
        u0 rehash(T& table, s32 new_capacity = -1);

        template <Hash_Table T>
        u0 free(T& table) {
            clear(table);
        }

        template <Hash_Table T>
        u0 reset(T& table) {
            if (table.flags) {
                std::memset(table.flags,
                            0,
                            hash_common::flag_words_for_capacity(table.capacity) * sizeof(u64));
            }
            table.size = {};
        }

        template <Hash_Table T>
        u0 clear(T& table) {
            memory::free(table.alloc, table.flags);
            table.keys     = {};
            table.flags    = {};
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
            for (u32 i = 0; i < table.capacity;) {
                if (!hash_common::read_flag(table.flags, i))
                    continue;
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
                if (!hash_common::read_flag(table.flags, i))
                    continue;
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
                if (!hash_common::read_flag(table.flags, i)) return false;
                if (hash == table.hashes[i] && key == table.keys[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!hash_common::read_flag(table.flags, i)) return false;
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
        u0 rehash(T& table, s32 new_capacity) {
            s32 idx = new_capacity == -1 ? table.cap_idx : hash_common::find_nearest_prime_capacity(new_capacity);
            f32 lf;
            do {
                new_capacity = hash_common::prime_capacity(idx++);
                lf = f32(table.size) / f32(new_capacity);
            } while (lf > table.load_factor);
            table.cap_idx = idx;

            const auto num_of_flags   = hash_common::flag_words_for_capacity(new_capacity);
            const auto size_of_hashes = new_capacity * sizeof(u64);
            const auto size_of_keys   = new_capacity * sizeof(Key_Type);
            const auto size_of_values = new_capacity * sizeof(Value_Type);

            const auto buf_size = (num_of_flags * sizeof(u64))
                                  + size_of_hashes
                                  + alignof(Key_Type) + size_of_keys
                                  + alignof(Value_Type) + size_of_values;

            auto buf = (u8*) memory::alloc(table.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, buf_size);

            u32 keys_align, values_align;
            auto new_flags  = (u64*) buf;
            auto new_hashes = new_flags + num_of_flags;
            auto new_keys = (Key_Type*) memory::system::align_forward(
                new_hashes + new_capacity,
                alignof(Key_Type),
                keys_align);
            auto new_values = (Value_Type*) memory::system::align_forward(
                new_keys + new_capacity,
                alignof(Value_Type),
                values_align);

            for (u32 i = 0; i < table.capacity; ++i) {
                if (!hash_common::read_flag(table.flags, i)) continue;
                u32 bucket_index = hash_common::range_reduction(table.hashes[i], new_capacity);
                assert(hash_common::find_free_bucket2(new_flags, new_capacity, bucket_index));
                hash_common::write_flag(new_flags, bucket_index, true);
                new_keys[bucket_index]   = table.keys[i];
                new_values[bucket_index] = table.values[i];
                new_hashes[bucket_index] = table.hashes[i];
            }

            memory::free(table.alloc, table.flags);

            table.keys      = new_keys;
            table.flags     = new_flags;
            table.values    = new_values;
            table.hashes    = new_hashes;
            table.capacity  = new_capacity;
        }

        template <Hash_Table T>
        u0 reserve(T& table, u32 new_capacity) {
            rehash(table, new_capacity);
        }

        template <Hash_Table T,
                  typename Key_Type = typename T::Key_Type>
        b8 remove(T& table, const Key_Type& key) {
            if (table.size == 0)
                return false;
            u64 hash         = hash::hash64(key);
            u32 bucket_index = hash_common::range_reduction(hash, table.capacity);
            u32 found_index;
            if (!find_key(table, bucket_index, hash, key, &found_index))
                return false;
            hash_common::write_flag(table.flags, found_index, false);
            --table.size;
            return true;
        }

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Value_Type = std::remove_pointer_t<Value_Type>*>
        Base_Value_Type find(T& table, const auto& key) {
            if (table.size == 0)
                return nullptr;

            u64 hash         = hash::hash64(key);
            u32 bucket_index = hash_common::range_reduction(hash, table.capacity);
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

        template <Hash_Table T>
        u0 init(T& table, alloc_t* alloc, f32 load_factor) {
            table.keys          = {};
            table.alloc         = alloc;
            table.flags         = {};
            table.values        = {};
            table.hashes        = {};
            table.cap_idx       = {};
            table.load_factor   = load_factor;
            table.size = table.capacity = {};
        }

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Value_Type = std::remove_pointer_t<Value_Type>*>
        Base_Value_Type emplace(T& table, const auto& key) {
            auto v = find(table, key);
            if (v)
                return v;

            if (hash_common::requires_rehash(table.size, table.capacity, table.load_factor))
                rehash(table);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = hash_common::range_reduction(hash, table.capacity);
            assert(hash_common::find_free_bucket2(table.flags, table.capacity, bucket_index));
            hash_common::write_flag(table.flags, bucket_index, true);
            table.keys[bucket_index]   = key;
            table.hashes[bucket_index] = hash;
            ++table.size;
            if constexpr (Is_Pointer) {
                return table.values[bucket_index];
            } else {
                return &table.values[bucket_index];
            }
        }

        template <Hash_Table T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  typename Base_Value_Type = std::remove_pointer_t<Value_Type>*>
        std::pair<Base_Value_Type, b8> emplace2(T& table, const auto& key) {
            auto v = find(table, key);
            if (v)
                return {v, false};

            if (hash_common::requires_rehash(table.size, table.capacity, table.load_factor))
                rehash(table);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = hash_common::range_reduction(hash, table.capacity);
            assert(hash_common::find_free_bucket2(table.flags, table.capacity, bucket_index));
            hash_common::write_flag(table.flags, bucket_index, true);
            table.keys[bucket_index]   = key;
            table.hashes[bucket_index] = hash;
            ++table.size;
            if constexpr (Is_Pointer) {
                return {table.values[bucket_index], true};
            } else {
                return {&table.values[bucket_index], true};
            }
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

            if (hash_common::requires_rehash(table.size, table.capacity, table.load_factor))
                rehash(table);

            u64 hash         = hash::hash64(key);
            u32 bucket_index = hash_common::range_reduction(hash, table.capacity);
            assert(hash_common::find_free_bucket2(table.flags, table.capacity, bucket_index));
            hash_common::write_flag(table.flags, bucket_index, true);
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
    }
}
