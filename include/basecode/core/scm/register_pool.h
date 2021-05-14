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

#include <basecode/core/scm/types.h>

namespace basecode::scm::reg_pool {
    inline u0 reset(reg_pool_t& pool) {
        pool.slots = {};
    }

    inline b8 has_free(reg_pool_t& pool) {
        return pool.slots != -1;
    }

    inline reg_t retain(reg_pool_t& pool) {
        u32 msb_zeros = pool.slots != 0 ?
                        __builtin_clzll(pool.slots) : pool.bit_count;
        if (msb_zeros == (pool.bit_count - pool.size))
            return 0;
        u32 idx = pool.bit_count - msb_zeros;
        pool.slots |= (1UL << idx);
        return pool.start + idx;
    }

    inline u0 release(reg_pool_t& pool, reg_t reg) {
        const u32 idx = reg - pool.start;
        if (!(pool.slots & (1UL << idx)))
            return;
        const auto mask = ~(1UL << idx);
        pool.slots &= mask;
    }

    inline u0 init(reg_pool_t& pool, reg_t start, reg_t end) {
        pool.end       = end;
        pool.start     = start;
        pool.size      = pool.end - pool.start;
        pool.slots     = {};
        pool.bit_count = sizeof(u64) * 8;
    }
}
