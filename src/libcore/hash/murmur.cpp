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

#include <basecode/core/hash/murmur.h>

namespace basecode::hash::murmur {
    u32 hash32(const u0* src, usize len) {
        return hash32(src, len, 0x9747b28c);
    }

    u32 hash32(const u0* src, usize len, u32 seed) {
        u32 const c1 = 0xcc9e2d51;
        u32 const c2 = 0x1b873593;
        u32 const r1 = 15;
        u32 const r2 = 13;
        u32 const m = 5;
        u32 const n = 0xe6546b64;

        s32 i, nblocks = len / 4;
        u32 hash = seed, k1 = 0;
        auto const* blocks = static_cast<u32 const*>(src);
        u8 const* tail = static_cast<u8 const*>(src) + nblocks * 4;

        for (i = 0; i < nblocks; i++) {
            u32 k = blocks[i];
            k *= c1;
            k = (k << r1) | (k >> (32 - r1));
            k *= c2;

            hash ^= k;
            hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
        }

        switch (len & 3U) {
            case 3: k1 ^= tail[2] << 16U;
            case 2: k1 ^= tail[1] << 8U;
            case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
        }

        hash ^= len;
        hash ^= (hash >> 16U);
        hash *= 0x85ebca6b;
        hash ^= (hash >> 13U);
        hash *= 0xc2b2ae35;
        hash ^= (hash >> 16U);

        return hash;
    }

    u64 hash64(const u0* src, usize len) {
        return hash64(src, len, 0x9747b28c);
    }

    u64 hash64(const u0* src, usize len, u64 seed) {
        u64 const m = 0xc6a4a7935bd1e995ULL;
        u32 const r = 47;

        u64 h = seed ^ (len * m);

        const u64* data = (const u64*)(src);
        const u8* data2 = (const u8*)(src);
        const u64* end = data + (len / 8);

        while (data != end) {
            u64 k = *data++;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        switch (len & 7U) {
            case 7: h ^= (u64) data2[6] << 48U;
            case 6: h ^= (u64) data2[5] << 40U;
            case 5: h ^= (u64) data2[4] << 32U;
            case 4: h ^= (u64) data2[3] << 24U;
            case 3: h ^= (u64) data2[2] << 16U;
            case 2: h ^= (u64) data2[1] << 8U;
            case 1: h ^= (u64) data2[0]; h *= m;
        }

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }
}
