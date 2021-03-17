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

#include <basecode/core/types.h>
#include <basecode/core/memory.h>
#include <basecode/core/hashable.h>

namespace basecode {
    template <typename T> requires hash::Hashable<T>
    struct set_t final {
        using Each_Callback     = u32 (*)(u32, const T&, u0*);

        alloc_t*                alloc;
        u64*                    hashes;
        T*                      values;
        u32                     size;
        u32                     capacity;
        f32                     load_factor;
    };

    namespace set {
        template <typename T> u0 rehash(set_t<T>& set, u32 new_capacity = 16);
        template <typename T> b8 find_value(set_t<T>& set, u32 start, u64 hash, const T& value, u32* found);
        template <typename T> u0 init(set_t<T>& set, alloc_t* alloc = context::top()->alloc, f32 load_factor = .5f);

        inline b8 requires_rehash(u32 size, u32 capacity, f32 load_factor) {
            return capacity == 0 || f32(size) + 1 > f32(capacity - 1) * load_factor;
        }

        inline b8 find_free_bucket(const u64* hashes, u32 size, u32 start, u32* found) {
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

        template <typename T,
                  typename B = std::remove_pointer_t<T>,
                  b8 IsPtr = std::is_pointer<T>::value>
        B* find(set_t<T>& set, const T& value) {
            if (set.size == 0) return nullptr;

            u64 hash         = hash::hash64(value);
            u32 bucket_index = u128(hash) * u128(set.capacity) >> 64U;
            u32 found_index;
            if (find_value(set, bucket_index, hash, value, &found_index)) {
                if constexpr (IsPtr) {
                    return set.values[found_index];
                } else {
                    return &set.values[found_index];
                }
            }

            return nullptr;
        }

        template <typename T,
                  typename B = std::remove_pointer_t<T>,
                  b8 IsPtr = std::is_pointer<T>::value>
        B* insert(set_t<T>& set, const T& value) {
            auto v = find(set, value);
            if (v)
                return v;

            if (requires_rehash(set.size, set.capacity, set.load_factor))
                rehash(set, set.capacity << 1);

            u64 hash         = hash::hash64(value);
            u32 bucket_index = u128(hash) * u128(set.capacity) >> 64U;
            u32 found_index;

            if (find_free_bucket(set.hashes, set.capacity, bucket_index, &found_index)) {
                set.hashes[found_index] = hash;
                set.values[found_index] = value;
                ++set.size;
                if constexpr (IsPtr) {
                    return set.values[found_index];
                } else {
                    return &set.values[found_index];
                }
            }

            return nullptr;
        }

        template <typename T> u0 free(set_t<T>& set) {
            memory::free(set.alloc, set.hashes);
            set.values   = {};
            set.hashes   = {};
            set.capacity = set.size = {};
        }

        template <typename T> u0 clear(set_t<T>& set) {
            free(set);
        }

        template <typename T> u0 reset(set_t<T>& set) {
            if (set.hashes)
                std::memset(set.hashes, 0, set.capacity * sizeof(u64));
            set.size = {};
        }

        template<typename T> inline b8 empty(set_t<T>& set) {
            return set.size == 0;
        }

        template <typename T,
                  typename Each_Callback = typename T::Each_Callback>
        u32 for_each(set_t<T>& set, Each_Callback cb, u0* user = {}) {
            u32 idx{};
            for (u32 i = 0; i < set.capacity; ++i) {
                if (!set.hashes[i]) continue;
                auto rc = cb(idx++, set.values[i], user);
                if (rc)
                    return rc;
            }
            return true;
        }

        template<typename T> b8 remove(set_t<T>& set, const T& value) {
            if (set.size == 0) return false;
            u64 hash         = hash::hash64(value);
            u32 bucket_index = u128(hash) * u128(set.capacity) >> 64U;
            u32 found_index;
            if (!find_value(set, bucket_index, hash, value, &found_index))
                return false;
            set.hashes[found_index] = 0;
            --set.size;
            return true;
        }

        template<typename T> u0 rehash(set_t<T>& set, u32 new_capacity) {
            new_capacity = std::max<u32>(new_capacity, std::ceil(std::max<u32>(16, new_capacity) / set.load_factor));

            const auto size_of_hashes = new_capacity * sizeof(u64);
            const auto size_of_values = new_capacity * sizeof(T);
            const auto buf_size       = size_of_hashes + alignof(T) + size_of_values;

            auto buf = (u8*) memory::alloc(set.alloc, buf_size, alignof(u64));
            std::memset(buf, 0, buf_size);

            u32  values_align{};
            auto new_hashes = (u64*) buf;
            auto new_values = (T*) memory::system::align_forward(buf + size_of_hashes,
                                                                 alignof(T),
                                                                 values_align);

            for (u32 i = 0; i < set.capacity; ++i) {
                if (!set.hashes[i]) continue;

                u32 bucket_index = u128(set.hashes[i]) * u128(new_capacity) >> 64U;
                u32 found_index;
                find_free_bucket(new_hashes, new_capacity, bucket_index, &found_index);

                new_hashes[found_index] = set.hashes[i];
                new_values[found_index] = set.values[i];
            }

            memory::free(set.alloc, set.hashes);

            set.hashes   = new_hashes;
            set.values   = new_values;
            set.capacity = new_capacity;
        }

        template<typename T> u0 reserve(set_t<T>& set, u32 new_capacity) {
            rehash(set, std::ceil(std::max<u32>(16, new_capacity) / std::min(.5f, set.load_factor)));
        }

        template <typename T> u0 init(set_t<T>& set, alloc_t* alloc, f32 load_factor) {
            set.hashes      = {};
            set.values      = {};
            set.alloc       = alloc;
            set.load_factor = load_factor;
            set.size        = set.capacity = {};
        }

        template<typename T> b8 find_value(set_t<T>& set, u32 start, u64 hash, const T& value, u32* found) {
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
