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
#include <basecode/core/hash/murmur.h>

namespace basecode::hash {
    template <typename K> u32 hash32(const K& value);

    template <typename K> u64 hash64(const K& value);

    inline u64 hash64(u0* const& key) {
        static const usize shift = std::log2(1 + sizeof(u0*));
        return usize(key) >> shift;
    }

    inline u32 hash32(const u8& key) { return hash32((u32) key); }

    inline u64 hash64(const u8& key) { return hash64((u64) key); }

    inline u32 hash32(const s8& key) { return hash32((s32) key); }

    inline u64 hash64(const s8& key) { return hash64((s64) key); }

    inline u32 hash32(const u16& key) { return hash32((u32) key); }

    inline u64 hash64(const u16& key) { return hash64((u64) key); }

    inline u32 hash32(const s16& key) { return hash32((s32) key); }

    inline u64 hash64(const s16& key) { return hash64((s64) key); }

    inline u32 hash32(const u32& key) { return murmur::hash32(&key, sizeof(u32)); }

    inline u64 hash64(const u32& key) { return murmur::hash64(&key, sizeof(u32)); }

    inline u32 hash32(const s32& key) { return murmur::hash32(&key, sizeof(s32)); }

    inline u64 hash64(const s32& key) { return murmur::hash64(&key, sizeof(s32)); }

    inline u64 hash64(const s64& key) { return murmur::hash64(&key, sizeof(s64)); }

    inline u64 hash64(const u64& key) { return murmur::hash64(&key, sizeof(u64)); }

    template <typename T> concept Hashable = requires(T hashable) {
        { hash::hash32(hashable) } -> convertible_to<u32>;
        { hash::hash64(hashable) } -> convertible_to<u64>;
    };
}
