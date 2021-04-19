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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <cstring>
#include <algorithm>
#include <basecode/core/bits.h>
#include <basecode/core/bitset.h>

namespace basecode {
    bitset_t& bitset_t::operator<<(u32 bits) {
        const auto extra_words = bits / 64;
        const auto in_word_shift = bits % 64;
        const auto as = capacity;
        if (in_word_shift == 0) {
            bitset::resize(*this, as + extra_words, false);
            for (auto i = as + extra_words; i > extra_words; i--)
                data[i - 1] = data[i - 1 - extra_words];
        } else {
            bitset::resize(*this, as + extra_words + 1);
            data[as + extra_words] = data[as - 1] >> (64 - in_word_shift);
            for (auto i = as + extra_words; i >= extra_words + 2; i--) {
                data[i - 1] = (data[i - 1 - extra_words] << in_word_shift) |
                              (data[i - 2 - extra_words] >> (64 - in_word_shift));
            }
            data[extra_words] = data[0] << in_word_shift;
        }
        for (auto i = 0; i < extra_words; i++)
            data[i] = 0;
        return *this;
    }

    bitset_t& bitset_t::operator>>(u32 bits) {
        const auto extra_words = bits / 64;
        const auto in_word_shift = bits % 64;
        const auto as = capacity;
        if (in_word_shift == 0) {
            for (auto i = 0; i < as - extra_words; i++)
                data[i] = data[i + extra_words];
            bitset::resize(*this, as - extra_words, false);
        } else {
            for (auto i = 0; i + extra_words + 1 < as; i++) {
                data[i] = (data[i + extra_words] >> in_word_shift) |
                          (data[i + extra_words + 1] << (64 - in_word_shift));
            }
            data[as - extra_words - 1] = (data[as - 1] >> in_word_shift);
            bitset::resize(*this, as - extra_words, false);
        }
        return *this;
    }

    namespace bitset {
        u32 min(bitset_t& set) {
            for (u32 i = 0; i < set.capacity; i++) {
                auto word = set.data[i];
                if (word != 0)
                    return __builtin_ctzll(word) + i * 64;
            }
            return 0;
        }

        u32 max(bitset_t& set) {
            for (u32 i = set.capacity; i > 0; i--) {
                auto word = set.data[i - 1];
                if (word != 0)
                    return 63 - __builtin_clzll(word) + (i - 1) * 64;
            }
            return 0;
        }

        u0 free(bitset_t& set) {
            clear(set);
        }

        u0 trim(bitset_t& set) {
            auto new_capacity = set.capacity;
            while (new_capacity > 0) {
                if (set.data[new_capacity - 1] == 0)
                    new_capacity--;
                else
                    break;
            }
            if (set.capacity == new_capacity)
                return;
            auto new_data = (u64*) memory::alloc(set.alloc, new_capacity * sizeof(u64), alignof(u64));
            if (set.data)
                std::memcpy(new_data, set.data, set.capacity * sizeof(u64));
            memory::free(set.alloc, set.data);
            set.data     = new_data;
            set.capacity = new_capacity;
        }

        b8 empty(bitset_t& set) {
            return count(set) == 0;
        }

        u0 clear(bitset_t& set) {
            memory::free(set.alloc, set.data);
            set.data     = {};
            set.capacity = {};
        }

        u0 reset(bitset_t& set) {
            std::memset(set.data, 0, sizeof(u64) * set.capacity);
        }

        u32 count(bitset_t& set) {
            u32 c{};
            u32 i{};
            for (; i + 7 < set.capacity; i += 8) {
                c += __builtin_popcountll(set.data[i]);
                c += __builtin_popcountll(set.data[i + 1]);
                c += __builtin_popcountll(set.data[i + 2]);
                c += __builtin_popcountll(set.data[i + 3]);
                c += __builtin_popcountll(set.data[i + 4]);
                c += __builtin_popcountll(set.data[i + 5]);
                c += __builtin_popcountll(set.data[i + 6]);
                c += __builtin_popcountll(set.data[i + 7]);
            }
            for (; i + 3 < set.capacity; i += 4) {
                c += __builtin_popcountll(set.data[i]);
                c += __builtin_popcountll(set.data[i + 1]);
                c += __builtin_popcountll(set.data[i + 2]);
                c += __builtin_popcountll(set.data[i + 3]);
            }
            for (; i < set.capacity; i++)
                c += __builtin_popcountll(set.data[i]);
            return c;
        }

        bitset_t make(alloc_t* alloc) {
            bitset_t set{};
            init(set, alloc);
            return set;
        }

        b8 read(bitset_t& set, u32 bit) {
            const auto shifted_bit = bit >> 6U;
            if (shifted_bit >= set.capacity)
                return false;
            return (set.data[shifted_bit] & (1ULL << (bit % 64))) != 0;
        }

        u32 size_in_bits(bitset_t& set) {
            return set.capacity * 64;
        }

        u32 size_in_words(bitset_t& set) {
            return set.capacity;
        }

        u32 size_in_bytes(bitset_t& set) {
            return set.capacity * sizeof(u64);
        }

        u0 init(bitset_t& set, alloc_t* alloc) {
            set.alloc    = alloc;
            set.data     = {};
            set.capacity = {};
        }

        b8 next_set_bit(bitset_t& set, u32& bit) {
            u32 word_index = bit >> 6U;
            if (word_index >= set.capacity) {
                bit = set.capacity << 6U;
                return false;
            }
            u64 word = set.data[word_index];
            word >>= bit & 63U;
            if (word != 0) {
                bit += __builtin_ctzll(word);
                return true;
            }
            word_index++;
            while (word_index < set.capacity) {
                word = set.data[word_index];
                if (word != 0) {
                    bit = (word_index << 6U) + __builtin_ctzll(word);
                    return true;
                }
                word_index++;
            }
            bit = set.capacity << 6U;
            return false;
        }

        u0 write(bitset_t& set, u32 bit, b8 flag) {
            const auto shifted_bit = bit >> 6U;
            const auto mask        = 1ULL << (bit % 64);
            const auto new_mask    = ((u64) flag) << (bit % 64);
            if (shifted_bit >= set.capacity)
                resize(set, (set.capacity * 2) * 64);
            auto word = set.data[shifted_bit];
            word &= ~mask;
            word |= new_mask;
            set.data[shifted_bit] = word;
        }

        b8 next_clear_bit(bitset_t& set, u32& bit) {
            constexpr u64 high_mask = 1ULL + ~1ULL;
            u32 word_index = bit >> 6U;
            if (word_index >= set.capacity) {
                bit = set.capacity << 6U;
                return false;
            }
            u64 word = ~set.data[word_index];
            if (word == high_mask)
                return true;
            word >>= bit & 63U;
            if (word != 0) {
                bit += __builtin_ctzll(word);
                return true;
            }
            word_index++;
            while (word_index < set.capacity) {
                word = ~set.data[word_index];
                if (word != 0) {
                    bit = (word_index << 6U) + __builtin_ctzll(word);
                    return true;
                }
                word_index++;
            }
            bit = set.capacity << 6U;
            return false;
        }

        b8 any_set(const bitset_t& set, u32 start_bit) {
            if (start_bit >= set.capacity)
                return false;
            for (auto i = start_bit; i < set.capacity; i++) {
                if (set.data[i] != 0)
                    return false;
            }
            return true;
        }

        u0 union_of(bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (size_t i = 0; i < min_length; ++i)
                lhs.data[i] |= rhs.data[i];
            if (rhs.capacity > lhs.capacity) {
                auto old_capacity = lhs.capacity;
                resize(lhs, rhs.capacity, false);
                std::memcpy(
                    lhs.data + old_capacity,
                    rhs.data + old_capacity,
                    (rhs.capacity - old_capacity) * sizeof(u64));
            }
        }

        u0 difference_of(bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (auto i = 0; i < min_length; ++i)
                lhs.data[i] &= ~(rhs.data[i]);
        }

        b8 disjoint(const bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (auto i = 0; i < min_length; i++) {
                if ((lhs.data[i] & rhs.data[i]) != 0)
                    return false;
            }
            return true;
        }

        u0 intersection_of(bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            u32 i{};
            for (; i < min_length; ++i)
                lhs.data[i] &= rhs.data[i];
            for (; i < lhs.capacity; ++i)
                lhs.data[i] = 0;
        }

        b8 contains_all(const bitset_t& lhs, const bitset_t& rhs) {
            for (auto i = 0; i < lhs.capacity; i++) {
                if ((lhs.data[i] & rhs.data[i]) != rhs.data[i])
                    return false;
            }
            if (rhs.capacity > lhs.capacity)
                return !any_set(rhs, lhs.capacity);
            return true;
        }

        u32 union_count(const bitset_t& lhs, const bitset_t& rhs) {
            u32 i{};
            u32 c{};
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (; i + 3 < min_length; i += 4) {
                c += __builtin_popcountll(lhs.data[i]     | rhs.data[i]);
                c += __builtin_popcountll(lhs.data[i + 1] | rhs.data[i + 1]);
                c += __builtin_popcountll(lhs.data[i + 2] | rhs.data[i + 2]);
                c += __builtin_popcountll(lhs.data[i + 3] | rhs.data[i + 3]);
            }
            for (; i < min_length; ++i)
                c += __builtin_popcountll(lhs.data[i] | rhs.data[i]);
            if (rhs.capacity > lhs.capacity) {
                for (; i + 3 < rhs.capacity; i += 4) {
                    c += __builtin_popcountll(rhs.data[i]);
                    c += __builtin_popcountll(rhs.data[i + 1]);
                    c += __builtin_popcountll(rhs.data[i + 2]);
                    c += __builtin_popcountll(rhs.data[i + 3]);
                }
                for (; i < rhs.capacity; ++i)
                    c += __builtin_popcountll(rhs.data[i]);
            } else {
                for (; i + 3 < lhs.capacity; i += 4) {
                    c += __builtin_popcountll(lhs.data[i]);
                    c += __builtin_popcountll(lhs.data[i + 1]);
                    c += __builtin_popcountll(lhs.data[i + 2]);
                    c += __builtin_popcountll(lhs.data[i + 3]);
                }
                for (; i < lhs.capacity; ++i)
                    c += __builtin_popcountll(lhs.data[i]);
            }
            return c;
        }

        b8 intersection_of(const bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (auto i = 0; i < min_length; i++) {
                if ((lhs.data[i] & rhs.data[i]) != 0)
                    return true;
            }
            return false;
        }

        u0 resize(bitset_t& set, u32 new_capacity, b8 pad_with_zeros) {
            new_capacity = std::max<u32>(next_power_of_two(new_capacity), 64);
            const auto capacity_in_words = std::max<u32>(new_capacity / 64, 1);
            const auto smallest = new_capacity < set.capacity ? new_capacity : set.capacity;
            if (set.capacity < new_capacity) {
                u64* new_data;
                new_data = (u64*) memory::alloc(set.alloc,
                                                capacity_in_words * sizeof(u64),
                                                alignof(u64));
                if (set.data)
                    std::memcpy(new_data, set.data, sizeof(u64) * set.capacity);
                memory::free(set.alloc, set.data);
                set.data     = new_data;
                set.capacity = capacity_in_words;
            }
            if (pad_with_zeros && (capacity_in_words > smallest)) {
                std::memset(
                    set.data + smallest,
                    0,
                    sizeof(u64) * (capacity_in_words - smallest));
            }
        }

        u0 symmetric_difference_of(bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            u32 i{};
            for (; i < min_length; ++i)
                lhs.data[i] ^= rhs.data[i];
            if (rhs.capacity > lhs.capacity) {
                const auto old_capacity = lhs.capacity;
                resize(lhs, rhs.capacity, false);
                std::memcpy(
                    lhs.data + old_capacity,
                    rhs.data + old_capacity,
                    (rhs.capacity - old_capacity) * sizeof(u64));
            }
        }

        u32 difference_count(const bitset_t& lhs, const bitset_t& rhs) {
            u32 i{};
            u32 c{};
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (; i < min_length; ++i)
                c += __builtin_popcountll(lhs.data[i] & ~(rhs.data[i]));
            for (; i < lhs.capacity; ++i)
                c += __builtin_popcountll(lhs.data[i]);
            return c;
        }

        u32 intersection_count(const bitset_t& lhs, const bitset_t& rhs) {
            u32 c{};
            const size_t min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            for (auto i = 0; i < min_length; ++i)
                c += __builtin_popcountll(lhs.data[i] & rhs.data[i]);
            return c;
        }

        u32 symmetric_difference_count(const bitset_t& lhs, const bitset_t& rhs) {
            const auto min_length = lhs.capacity < rhs.capacity ? lhs.capacity : rhs.capacity;
            u32 i{};
            u32 c{};
            for (; i < min_length; ++i)
                c += __builtin_popcountll(lhs.data[i] ^ rhs.data[i]);
            if (rhs.capacity > lhs.capacity) {
                for (; i < rhs.capacity; ++i)
                    c += __builtin_popcountll(rhs.data[i]);
            } else {
                for (; i < lhs.capacity; ++i)
                    c += __builtin_popcountll(lhs.data[i]);
            }
            return c;
        }
    }
}
