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

#include <basecode/core/array.h>
#include <basecode/core/memory.h>
#include <basecode/core/hashable.h>
#include <basecode/core/iterator.h>
#include <basecode/core/hash_common.h>

namespace basecode {
    template <typename T>
    concept Hash_Set = hash::Hashable<T> && requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Is_Pointer;
        typename                T::Value_Type_Base;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.flags}               -> same_as<u64*>;
        {t.hashes}              -> same_as<u64*>;
        {t.values}              -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
        {t.load_factor}         -> same_as<f32>;
    };

    struct set_buf_size_t final {
        u32                     total_size;
        u32                     size_of_flags;
        u32                     num_flag_words;
        u32                     size_of_hashes;
        u32                     size_of_values;
    };

    template <typename T>
    struct set_t final {
        using Value_Type        = T;
        using Is_Pointer        = std::integral_constant<b8, std::is_pointer_v<T>>;
        using Value_Type_Base   = std::remove_pointer_t<T>;

        alloc_t*                alloc;
        u64*                    flags;
        u64*                    hashes;
        Value_Type*             values;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const set_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const set_t* ref) {
                while (pos < ref->capacity) {
                    if (ref->hashes[++pos])
                        break;
                }
            }

            inline Value_Type get(set_t* ref) {
                return ref->values[pos];
            }

            inline u0 begin(const set_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (ref->hashes[pos])
                        break;
                    ++pos;
                }
            }

            inline const Value_Type get(const set_t* ref) {
                return ref->values[pos];
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(set_t, Value_Type, iterator_state_t);
    };
    static_assert(sizeof(set_t<s32>) <= 56, "set_t<T> is now larger than 56 bytes!");

    namespace set {
        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        u0 rehash(T& set, s32 new_capacity = -1);

        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        b8 find_value(T& set, u32 start, u64 hash, const Value_Type& value, u32* found);

        template <Hash_Set T>
        u0 init(T& set, alloc_t* alloc = context::top()->alloc, f32 load_factor = .75f);

        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        inline set_buf_size_t size_of_buffer(T& set, u32 capacity = 0) {
            if (capacity == 0)
                capacity = set.capacity;
            set_buf_size_t size{};
            size.num_flag_words = hash_common::flag_words_for_capacity(capacity);
            size.size_of_flags  = size.num_flag_words * sizeof(u64);
            size.size_of_hashes = capacity * sizeof(u64);
            size.size_of_values = capacity * sizeof(Value_Type);
            size.total_size     = size.size_of_flags
                                  + size.size_of_hashes
                                  + alignof(Value_Type)
                                  + size.size_of_values;
            return size;
        }

        template <Hash_Set T>
        u0 free(T& set) {
            memory::free(set.alloc, set.flags);
            set.flags    = {};
            set.values   = {};
            set.hashes   = {};
            set.capacity = set.size = {};
        }

        template <Hash_Set T>
        u0 clear(T& set) {
            free(set);
        }

        template <Hash_Set T>
        u0 reset(T& set) {
            if (set.hashes) {
                const auto set_size = size_of_buffer(set);
                std::memset(set.flags, 0, set_size.total_size);
            }
            set.size = {};
        }

        template <Hash_Set T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        auto values(T& set) {
            using Array = array_t<Value_Type_Base*>;

            Array list{};
            array::init(list, set.alloc);
            array::reserve(list, set.size);
            for (u32 i = 0; i < set.capacity; ++i) {
                if (!hash_common::read_flag(set.flags, i)) continue;
                if constexpr (Is_Pointer) {
                    array::append(list, set.values[i]);
                } else {
                    array::append(list, &set.values[i]);
                }
            }
            return list;
        }

        template<Hash_Set T>
        inline b8 empty(T& set) {
            return set.size == 0;
        }

        template <Hash_Set T, typename Value_Type>
        u0 rehash(T& set, s32 new_capacity) {
            s32 idx = new_capacity == -1 ? set.cap_idx : hash_common::find_nearest_prime_capacity(new_capacity);
            f32 lf;
            do {
                new_capacity = hash_common::prime_capacity(idx++);
                lf = f32(set.size) / f32(new_capacity);
            } while (lf > set.load_factor);
            set.cap_idx = idx;

            const auto buf_size = size_of_buffer(set, new_capacity);
            auto buf = (u8*) memory::alloc(set.alloc, buf_size.total_size, alignof(u64));
            std::memset(buf, 0, buf_size.total_size);

            u32  align{};
            auto new_flags  = (u64*) buf;
            auto new_hashes = (u64*) memory::system::align_forward(
                new_flags + buf_size.num_flag_words,
                alignof(u64),
                align);
            auto new_values = (Value_Type*) memory::system::align_forward(
                new_hashes + new_capacity,
                alignof(Value_Type),
                align);

            for (u32 i = 0; i < set.capacity; ++i) {
                if (!hash_common::read_flag(set.flags, i)) continue;
                u32 bucket_index = hash_common::range_reduction(set.hashes[i], new_capacity);
                b8  found = hash_common::find_free_bucket2(new_flags, new_capacity, bucket_index);
                if (found) {
                    hash_common::write_flag(new_flags, bucket_index, true);
                    new_hashes[bucket_index] = set.hashes[i];
                    new_values[bucket_index] = set.values[i];
                }
            }

            memory::free(set.alloc, set.flags);

            set.flags    = new_flags;
            set.hashes   = new_hashes;
            set.values   = new_values;
            set.capacity = new_capacity;
        }

        template<Hash_Set T>
        u0 reserve(T& set, u32 new_capacity) {
            rehash(set, std::max<u32>(17, new_capacity));
        }

        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        b8 remove(T& set, const Value_Type& value) {
            if (set.size == 0) return false;
            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash, set.capacity);
            u32 found_index;
            if (!find_value(set, bucket_index, hash, value, &found_index))
                return false;
            hash_common::write_flag(set.flags, found_index, false);
            --set.size;
            return true;
        }

        template <Hash_Set T>
        u0 init(T& set, alloc_t* alloc, f32 load_factor) {
            set.size        = set.capacity = {};
            set.flags       = {};
            set.alloc       = alloc;
            set.hashes      = {};
            set.values      = {};
            set.cap_idx     = {};
            set.load_factor = load_factor;
        }

        template <Hash_Set T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type = typename T::Value_Type,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        Value_Type_Base* find(T& set, const Value_Type& value) {
            if (set.size == 0)
                return nullptr;

            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash, set.capacity);
            u32 found_index;
            if (find_value(set, bucket_index, hash, value, &found_index)) {
                if constexpr (Is_Pointer) {
                    return set.values[found_index];
                } else {
                    return &set.values[found_index];
                }
            }

            return nullptr;
        }

        template <Hash_Set T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type = typename T::Value_Type,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        Value_Type_Base* insert(T& set, const Value_Type& value) {
            auto v = find(set, value);
            if (v)
                return v;

            if (hash_common::requires_rehash(set.size, set.capacity, set.load_factor))
                rehash(set);

            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash, set.capacity);
            b8  found        = hash_common::find_free_bucket2(set.flags, set.capacity, bucket_index);
            if (found) {
                hash_common::write_flag(set.flags, bucket_index, true);
                set.hashes[bucket_index] = hash;
                set.values[bucket_index] = value;
                ++set.size;
                if constexpr (Is_Pointer) {
                    return set.values[bucket_index];
                } else {
                    return &set.values[bucket_index];
                }
            }

            return (Value_Type_Base*) nullptr;
        }

        template <Hash_Set T, typename Value_Type>
        b8 find_value(T& set, u32 start, u64 hash, const Value_Type& value, u32* found) {
            for (u32 i = start; i < set.capacity; ++i) {
                if (!hash_common::read_flag(set.flags, i)) return false;
                if (hash == set.hashes[i] && value == set.values[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!hash_common::read_flag(set.flags, i)) return false;
                if (hash == set.hashes[i] && value == set.values[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }
    }
}
