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

#include <basecode/core/hash/adler.h>

namespace basecode::hash::adler {
    u32 hash32(u0 const* data, usize len) {
        u32 const MOD_ALDER = 65521;
        u32 a = 1, b = 0;
        s32 i, block_len;
        auto const* bytes = static_cast<u8 const *>(data);

        block_len = len % 5552;

        while (len) {
            for (i = 0; i+7 < block_len; i += 8) {
                a += bytes[0], b += a;
                a += bytes[1], b += a;
                a += bytes[2], b += a;
                a += bytes[3], b += a;
                a += bytes[4], b += a;
                a += bytes[5], b += a;
                a += bytes[6], b += a;
                a += bytes[7], b += a;

                bytes += 8;
            }
            for (; i < block_len; i++) {
                a += *bytes++, b += a;
            }

            a %= MOD_ALDER, b %= MOD_ALDER;
            len -= block_len;
            block_len = 5552;
        }

        return (b << (u32) 16) | a;
    }
}
