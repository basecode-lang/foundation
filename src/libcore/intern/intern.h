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
#include <basecode/core/array/array.h>
#include <basecode/core/slice/slice.h>
#include <basecode/core/hashtable/hashtable.h>

namespace basecode::intern {
    enum class status_t : u8 {
        ok,
        no_bucket,
        not_found,
    };

    struct index_t final {
        u32*                            ids;
        u64*                            hashes;
        string::slice_t*                slices;
    };

    struct result_t final {
        u64                             hash{};
        string::slice_t                 slice{};
        u32                             id{};
        status_t                        status{};
    };

    struct pool_t final {
        u8*                             index;
        alloc_t*                        alloc;
        alloc_t                         page_alloc;
        alloc_t                         bump_alloc;
        u32                             id;
        u32                             size;
        u32                             capacity;
    };

    namespace pool {
        u0 free(pool_t& pool);

        u0 reset(pool_t& pool);

        result_t get(pool_t& pool, u32 id);

        result_t intern(pool_t& pool, string::slice_t value);

        pool_t make(alloc_t* alloc = context::top()->alloc);

        u0 init(pool_t& pool, alloc_t* alloc = context::top()->alloc);
    }
}

