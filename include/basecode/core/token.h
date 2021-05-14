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

#include <basecode/core/context.h>

namespace basecode::token {
    namespace cache {
        u0 free(token_cache_t& cache);

        u32 size(token_cache_t& cache);

        b8 has_more(token_cache_t& cache);

        u0 move_next(token_cache_t& cache);

        u0 seek_first(token_cache_t& cache);

        token_id_t append(token_cache_t& cache,
                          token_type_t type,
                          const source_info_t& loc);

        const token_t& current(token_cache_t& cache);

        u0 init(token_cache_t& cache,
                alloc_t* alloc = context::top()->alloc.main);

        const token_t& get(token_cache_t& cache, token_id_t id);

        const token_t& peek(token_cache_t& cache, u32 count = 1);

        token_id_t append(token_cache_t& cache, token_type_t type);
    }

    str::slice_t span(const token_t& token, const Buffer_Concept auto& buf) {
        return slice::make(buf.data + token.loc.pos.start,
                           (token.loc.pos.end - token.loc.pos.start) + 1);
    }
}
