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
#include <basecode/core/array.h>
#include <basecode/core/slice.h>

namespace basecode {
    struct interned_str_t;

    using intern_id             = u32;
    using interned_str_list_t   = array_t<interned_str_t>;

    struct interned_str_t final {
        str::slice_t            value;
        u32                     bucket_index;
    };

    struct intern_t final {
        alloc_t*                alloc;
        intern_id*              ids;
        u64*                    hashes;
        interned_str_list_t     strings;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;
    };
    static_assert(sizeof(intern_t) <= 88, "intern_t is now larger than 88 bytes!");

    namespace intern {
        enum class status_t : u8 {
            ok                  = 0,
            no_bucket           = 135,
            not_found           = 136,
        };

        struct result_t final {
            u64                 hash        {};
            str::slice_t        slice       {};
            intern_id           id          {};
            status_t            status      {};
            b8                  new_value   {};

            b8 operator==(const result_t& other) const {
                return id == other.id;
            }
        };

        u0 free(intern_t& pool);

        u0 reset(intern_t& pool);

        b8 remove(intern_t& pool, intern_id id);

        u0 reserve(intern_t& pool, u32 capacity);

        result_t get(intern_t& pool, intern_id id);

        intern_t make(alloc_t* alloc = context::top()->alloc);

        result_t fold(intern_t& pool, const s8* data, s32 len = -1);

        result_t fold(intern_t& pool, const String_Concept auto& value) {
            return fold(pool, (const s8*) value.data, value.length);
        }

        u0 init(intern_t& pool, alloc_t* alloc = context::top()->alloc, f32 load_factor = .5f);
    }
}

