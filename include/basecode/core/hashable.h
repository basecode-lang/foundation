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

#pragma once

#include <chrono>
#include <basecode/core/types.h>
#include <basecode/core/hash/murmur.h>

namespace basecode::hash {
    static const u64 s_fixed_random = std::chrono::steady_clock::now().time_since_epoch().count();

    template <typename K> u32 hash32(const K& value);

    template <typename K> u64 hash64(const K& value);

    template <typename T> concept Hashable = requires(T hashable) {
        { hash::hash32(hashable) } -> convertible_to<u32>;
        { hash::hash64(hashable) } -> convertible_to<u64>;
    };

    // http://xorshift.di.unimi.it/splitmix64.c
    inline u64 splitmix64(u64 x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    inline u64 hash64(u0* const& key) {
        static const usize shift = std::log2(1 + sizeof(u0*));
        return splitmix64(s_fixed_random + (usize(key) >> shift));
    }

    inline u32 hash32(const u8& key) { return hash32((u32) key); }

    inline u64 hash64(const u8& key) { return hash64((u64) key); }

    inline u32 hash32(const s8& key) { return hash32((s32) key); }

    inline u64 hash64(const s8& key) { return hash64((s64) key); }

    inline u32 hash32(const u16& key) { return hash32((u32) key); }

    inline u64 hash64(const u16& key) { return hash64((u64) key); }

    inline u32 hash32(const s16& key) { return hash32((s32) key); }

    inline u64 hash64(const s16& key) { return hash64((s64) key); }

    inline u32 hash32(const u32& key) { return splitmix64(s_fixed_random + key); }

    inline u64 hash64(const u32& key) { return splitmix64(s_fixed_random + key); }

    inline u32 hash32(const s32& key) { return splitmix64(s_fixed_random + key); }

    inline u64 hash64(const s32& key) { return splitmix64(s_fixed_random + key); }

    inline u64 hash64(const s64& key) { return splitmix64(s_fixed_random + key); }

    inline u64 hash64(const u64& key) { return splitmix64(s_fixed_random + key); }
}
