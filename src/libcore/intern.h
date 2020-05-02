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

#include "types.h"
#include "array.h"
#include "slice.h"
#include "hashtable.h"

namespace basecode {
    struct intern_t final {
        u8*                             index;
        alloc_t*                        alloc;
        alloc_t*                        page_alloc;
        alloc_t*                        bump_alloc;
        u32*                            ids;
        u64*                            hashes;
        string::slice_t*                slices;
        u32                             id;
        u32                             size;
        u32                             capacity;
    };

    namespace intern {
        enum class status_t : u8 {
            ok,
            no_bucket,
            not_found,
        };

        struct result_t final {
            u64                         hash;
            string::slice_t             slice;
            u32                         id;
            status_t                    status;
            b8                          new_value;
        };

        u0 free(intern_t& pool);

        u0 reset(intern_t& pool);

        result_t get(intern_t& pool, u32 id);

        result_t intern(intern_t& pool, string::slice_t value);

        intern_t make(alloc_t* alloc = context::top()->alloc);

        u0 init(intern_t& pool, alloc_t* alloc = context::top()->alloc, u32 num_pages = 16);
    }
}

