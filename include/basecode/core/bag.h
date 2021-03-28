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
#ifdef __clang__
#   include <basecode/core/getopt.h>            // XXX: only clang complains about not finding
                                                //      the hash64 function unless i include the
                                                //      header here.
#endif
#include <basecode/core/hashable.h>
#include <basecode/core/iterator.h>
#include <basecode/core/hash_common.h>

namespace basecode {
    template <typename T>
    concept Hash_Bag = hash::Hashable<T> && requires(const T& t) {
        typename                T::Item_Type;
        typename                T::Value_Type;
        typename                T::Is_Pointer;
        typename                T::Value_Type_Base;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.flags}               -> same_as<u64*>;
        {t.hashes}              -> same_as<u64*>;
        {t.counts}              -> same_as<u32*>;
        {t.values}              -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
        {t.load_factor}         -> same_as<f32>;
    };

    struct bag_buf_size_t final {
        u32                     total_size;
        u32                     size_of_flags;
        u32                     num_flag_words;
        u32                     size_of_hashes;
        u32                     size_of_counts;
        u32                     size_of_values;
    };

    template <typename T>
    struct bag_t final {
        using Value_Type        = T;
        using Is_Pointer        = std::integral_constant<b8, std::is_pointer_v<T>>;
        using Value_Type_Base   = std::remove_pointer_t<T>;

        struct item_t final {
            Value_Type_Base*    value;
            u32                 count;
        };
        using Item_Type         = item_t;

        alloc_t*                alloc;
        u64*                    flags;
        u64*                    hashes;
        u32*                    counts;
        Value_Type*             values;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const bag_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const bag_t* ref) {
                while (pos < ref->capacity) {
                    if (ref->hashes[++pos])
                        break;
                }
            }

            inline Item_Type get(bag_t* ref) {
                return {&ref->values[pos], ref->counts[pos]};
            }

            inline u0 begin(const bag_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (ref->hashes[pos])
                        break;
                    ++pos;
                }
            }

            inline const Item_Type get(const bag_t* ref) {
                return {&ref->values[pos], ref->counts[pos]};
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(bag_t, Item_Type, iterator_state_t);
    };
    static_assert(sizeof(bag_t<s32>) <= 56, "bag_t<T> is now larger than 56 bytes!");

    namespace bag {
        template <Hash_Bag T,
                  typename Value_Type = typename T::Value_Type>
        u0 rehash(T& bag, s32 new_capacity = -1);

        template <Hash_Bag T,
                  typename Value_Type = typename T::Value_Type>
        b8 find_value(T& bag, u32 start, u64 hash, const Value_Type& value, u32* found);

        template <Hash_Bag T>
        u0 init(T& bag, alloc_t* alloc = context::top()->alloc, f32 load_factor = .75f);

        template <Hash_Bag T,
                  typename Value_Type = typename T::Value_Type>
        inline bag_buf_size_t size_of_buffer(T& bag, u32 capacity = 0);

        template <Hash_Bag T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type = typename T::Value_Type,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        Value_Type_Base* find(T& bag, const Value_Type& value, u32& found_index);

        template <Hash_Bag T>
        u0 free(T& bag) {
            memory::free(bag.alloc, bag.flags);
            bag.flags    = {};
            bag.values   = {};
            bag.hashes   = {};
            bag.counts   = {};
            bag.capacity = bag.size = {};
        }

        template <Hash_Bag T>
        u0 clear(T& bag) {
            free(bag);
        }

        template <Hash_Bag T>
        u0 reset(T& bag) {
            if (bag.hashes) {
                const auto buf_size = size_of_buffer(bag);
                std::memset(bag.flags, 0, buf_size.total_size);
            }
            bag.size = {};
        }

        template <Hash_Bag T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Item_Type = typename T::Item_Type>
        auto values(T& bag) {
            using Array = array_t<Item_Type>;

            Array list{};
            array::init(list, bag.alloc);
            array::reserve(list, bag.size);
            for (u32 i = 0; i < bag.capacity; ++i) {
                if (!hash_common::read_flag(bag.flags, i)) continue;
                if constexpr (Is_Pointer) {
                    array::append(list, {bag.values[i], bag.counts[i]});
                } else {
                    array::append(list, {&bag.values[i], bag.counts[i]});
                }
            }
            return list;
        }

        template<Hash_Bag T>
        inline b8 empty(T& bag) {
            return bag.size == 0;
        }

        template <Hash_Bag T, typename Value_Type>
        u0 rehash(T& bag, s32 new_capacity) {
            s32 idx = new_capacity == -1 ?
                bag.cap_idx :
                hash_common::find_nearest_prime_capacity(new_capacity);
            f32 lf;
            do {
                new_capacity = hash_common::prime_capacity(idx++);
                lf = f32(bag.size) / f32(new_capacity);
            } while (lf > bag.load_factor);
            bag.cap_idx = idx;

            const auto buf_size = size_of_buffer(bag, new_capacity);
            auto buf = (u8*) memory::alloc(bag.alloc, buf_size.total_size, alignof(u64));
            std::memset(buf, 0, buf_size.total_size);

            u32  align{};
            auto new_flags  = (u64*) buf;
            auto new_hashes = new_flags + buf_size.num_flag_words;
            auto new_counts = (u32*) memory::system::align_forward(
                new_hashes + new_capacity,
                alignof(u32),
                align);
            auto new_values = (Value_Type*) memory::system::align_forward(
                new_counts + new_capacity,
                alignof(Value_Type),
                align);

            for (u32 i = 0; i < bag.capacity; ++i) {
                if (!hash_common::read_flag(bag.flags, i)) continue;
                u32 bucket_index = hash_common::range_reduction(bag.hashes[i], new_capacity);
                b8  found        = hash_common::find_free_bucket2(new_flags, new_capacity, bucket_index);
                if (found) {
                    hash_common::write_flag(new_flags, bucket_index, true);
                    new_hashes[bucket_index] = bag.hashes[i];
                    new_values[bucket_index] = bag.values[i];
                    new_counts[bucket_index] = bag.counts[i];
                }
            }

            memory::free(bag.alloc, bag.flags);

            bag.flags    = new_flags;
            bag.hashes   = new_hashes;
            bag.counts   = new_counts;
            bag.values   = new_values;
            bag.capacity = new_capacity;
        }

        template<Hash_Bag T>
        u0 reserve(T& bag, u32 new_capacity) {
            rehash(bag, std::max<u32>(17, new_capacity));
        }

        template <Hash_Bag T,
                  typename Value_Type = typename T::Value_Type>
        u32 count(T& bag, const Value_Type& value) {
            u32 found_index{};
            auto v = find(bag, value, found_index);
            return v ? bag.counts[found_index] : 0;
        }

        template <Hash_Bag T,
                  typename Value_Type = typename T::Value_Type>
        b8 remove(T& bag, const Value_Type& value) {
            if (bag.size == 0) return false;
            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash, bag.capacity);
            u32 found_index;
            if (!find_value(bag, bucket_index, hash, value, &found_index))
                return false;
            auto& count = bag.counts[found_index];
            if (count > 0) {
                --count;
            } else {
                hash_common::write_flag(bag.flags, found_index, false);
                --bag.size;
            }
            return true;
        }

        template <Hash_Bag T, typename Value_Type>
        inline bag_buf_size_t size_of_buffer(T& bag, u32 capacity) {
            if (capacity == 0)
                capacity = bag.capacity;
            bag_buf_size_t size{};
            size.num_flag_words = hash_common::flag_words_for_capacity(capacity);
            size.size_of_flags  = size.num_flag_words * sizeof(u64);
            size.size_of_hashes = capacity * sizeof(u64);
            size.size_of_counts = capacity * sizeof(u32);
            size.size_of_values = capacity * sizeof(Value_Type);
            size.total_size     = size.size_of_flags
                                  + size.size_of_hashes
                                  + alignof(u32)
                                  + size.size_of_counts
                                  + alignof(Value_Type)
                                  + size.size_of_values;
            return size;
        }

        template <Hash_Bag T>
        u0 init(T& bag, alloc_t* alloc, f32 load_factor) {
            bag.size        = bag.capacity = {};
            bag.flags       = {};
            bag.alloc       = alloc;
            bag.hashes      = {};
            bag.counts      = {};
            bag.values      = {};
            bag.cap_idx     = {};
            bag.load_factor = load_factor;
        }

        template <Hash_Bag T,
                  b8 Is_Pointer = T::Is_Pointer::value,
                  typename Value_Type = typename T::Value_Type,
                  typename Value_Type_Base = typename T::Value_Type_Base>
        Value_Type_Base* insert(T& bag, const Value_Type& value) {
            u32 found_index{};
            auto v = find(bag, value, found_index);
            if (v) {
                auto& count = bag.counts[found_index];
                ++count;
                return v;
            }

            if (hash_common::requires_rehash(bag.size, bag.capacity, bag.load_factor))
                rehash(bag);

            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash,
                                                            bag.capacity);
            b8  found        = hash_common::find_free_bucket2(bag.flags,
                                                              bag.capacity,
                                                              bucket_index);
            if (found) {
                hash_common::write_flag(bag.flags, bucket_index, true);
                bag.hashes[bucket_index] = hash;
                bag.values[bucket_index] = value;
                bag.counts[bucket_index] = 1;
                ++bag.size;
                if constexpr (Is_Pointer) {
                    return bag.values[bucket_index];
                } else {
                    return &bag.values[bucket_index];
                }
            }

            return (Value_Type_Base*) nullptr;
        }

        template <Hash_Bag T, b8 Is_Pointer, typename Value_Type, typename Value_Type_Base>
        Value_Type_Base* find(T& bag, const Value_Type& value, u32& found_index) {
            if (bag.size == 0)
                return nullptr;

            u64 hash         = hash::hash64(value);
            u32 bucket_index = hash_common::range_reduction(hash, bag.capacity);
            if (find_value(bag, bucket_index, hash, value, &found_index)) {
                if constexpr (Is_Pointer) {
                    return bag.values[found_index];
                } else {
                    return &bag.values[found_index];
                }
            }

            return nullptr;
        }

        template <Hash_Bag T, typename Value_Type>
        b8 find_value(T& bag, u32 start, u64 hash, const Value_Type& value, u32* found) {
            for (u32 i = start; i < bag.capacity; ++i) {
                if (!hash_common::read_flag(bag.flags, i)) return false;
                if (hash == bag.hashes[i] && value == bag.values[i]) {
                    *found = i;
                    return true;
                }
            }
            for (u32 i = 0; i < start; ++i) {
                if (!hash_common::read_flag(bag.flags, i)) return false;
                if (hash == bag.hashes[i] && value == bag.values[i]) {
                    *found = i;
                    return true;
                }
            }
            return false;
        }
    }
}
