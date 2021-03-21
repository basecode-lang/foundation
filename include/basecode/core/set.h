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
#include <basecode/core/hash_common.h>

namespace basecode {
    template <typename T>
    concept Hash_Set = hash::Hashable<T> && requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Each_Callback;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.hashes}              -> same_as<u64*>;
        {t.values}              -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
        {t.load_factor}         -> same_as<f32>;
    };

    template <typename T>
    struct set_t final {
        using Value_Type        = T;
        using Value_Type_Base   = std::remove_pointer_t<T>;
        using Is_Pointer        = std::integral_constant<b8, std::is_pointer_v<T>>;
        using Each_Callback     = u32 (*)(u32, const T&, u0*);

        alloc_t*                alloc;
        u64*                    hashes;
        Value_Type*             values;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;
    };

    namespace set {
        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        u0 rehash(T& set, s32 new_capacity = -1);

        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        b8 find_value(T& set, u32 start, u64 hash, const Value_Type& value, u32* found);

        template <Hash_Set T>
        u0 init(T& set, alloc_t* alloc = context::top()->alloc, f32 load_factor = .9f);

        template <Hash_Set T,
                  typename Value_Type = typename T::Value_Type>
        inline u32 size_of_buffer(T& set, u32 capacity = 0) {
            if (capacity == 0)
                capacity = set.capacity;
            const auto size_of_hashes = capacity * sizeof(u64);
            const auto size_of_values = capacity * sizeof(Value_Type);
            return size_of_hashes + alignof(Value_Type) + size_of_values;
        }

        template <Hash_Set T>
        u0 free(T& set) {
            memory::free(set.alloc, set.hashes);
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
            if (set.hashes)
                std::memset(set.hashes, 0, size_of_buffer(set));
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
                if (!set.hashes[i]) continue;
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
            if (new_capacity == -1) {
                new_capacity = prime_capacity(set.cap_idx++);
            } else {
                new_capacity = std::max<u32>(new_capacity, std::max<u32>(17, new_capacity));
            }
            const auto buf_size = size_of_buffer(set, new_capacity);
            auto buf = (u8*) memory::alloc(set.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, buf_size);

            const auto size_of_hashes = new_capacity * sizeof(u64);
            u32  values_align{};
            auto new_hashes = (u64*) buf;
            auto new_values = (Value_Type*) memory::system::align_forward(buf + size_of_hashes,
                                                                          alignof(Value_Type),
                                                                          values_align);

            for (u32 i = 0; i < set.capacity; ++i) {
                if (!set.hashes[i])
                    continue;
                u32 bucket_index = range_reduction(set.hashes[i], new_capacity);
                if (find_free_bucket(new_hashes, new_capacity, bucket_index)) {
                    new_hashes[bucket_index] = set.hashes[i];
                    new_values[bucket_index] = set.values[i];
                }
            }

            memory::free(set.alloc, set.hashes);

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
            u32 bucket_index = range_reduction(hash, set.capacity);
            u32 found_index;
            if (!find_value(set, bucket_index, hash, value, &found_index))
                return false;
            set.hashes[found_index] = 0;
            --set.size;
            return true;
        }

        template <Hash_Set T>
        u0 init(T& set, alloc_t* alloc, f32 load_factor) {
            set.size        = set.capacity = {};
            set.alloc       = alloc;
            set.hashes      = {};
            set.values      = {};
            set.cap_idx     = {};
            set.load_factor = load_factor;
        }

        template <Hash_Set T,
                  typename Each_Callback = typename T::Each_Callback>
        u32 for_each(T& set, Each_Callback cb, u0* user = {}) {
            u32 idx{};
            for (u32 i = 0; i < set.capacity; ++i) {
                if (!set.hashes[i]) continue;
                auto rc = cb(idx++, set.values[i], user);
                if (rc)
                    return rc;
            }
            return true;
        }

        template <Hash_Set T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type = typename T::Value_Type,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        Value_Type_Base* find(T& set, const Value_Type& value) {
            if (set.size == 0)
                return nullptr;

            u64 hash         = hash::hash64(value);
            u32 bucket_index = range_reduction(hash, set.capacity);
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

            if (requires_rehash(set.size, set.capacity, set.load_factor))
                rehash(set);

            u64 hash         = hash::hash64(value);
            u32 bucket_index = range_reduction(hash, set.capacity);

            if (find_free_bucket(set.hashes, set.capacity, bucket_index)) {
                set.hashes[bucket_index] = hash;
                set.values[bucket_index] = value;
                ++set.size;
                if constexpr (Is_Pointer) {
                    return set.values[bucket_index];
                } else {
                    return &set.values[bucket_index];
                }
            }

            return nullptr;
        }

        template <Hash_Set T, typename Value_Type>
        b8 find_value(T& set, u32 start, u64 hash, const Value_Type& value, u32* found) {
            for (u32 i = start; i < set.capacity; ++i) {
                if (!set.hashes[i]) return false;
                if (hash == set.hashes[i] && value == set.values[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!set.hashes[i]) return false;
                if (hash == set.hashes[i] && value == set.values[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }
    }
}
