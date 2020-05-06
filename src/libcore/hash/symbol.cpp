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

#include <basecode/core/hash/symbol.h>

namespace basecode::hash::symbol {
    u64 hash64(const u8* src, usize len) {
        u64 h{};
        u64 g;

        for (u64 i = 0; i < len; ++i) {
            h = (h << 4) + *src++;
            g = h & 0xf0000000;
            if (g)
                h ^= g >> 24;
            h &= ~g;
        }
        return h;
    }
}
