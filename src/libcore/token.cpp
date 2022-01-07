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

#include <basecode/core/token.h>
#include <basecode/core/array.h>

namespace basecode::token::cache {
    u0 free(token_cache_t& cache) {
        array::free(cache.tokens);
    }

    u32 size(token_cache_t& cache) {
        return cache.tokens.size;
    }

    b8 has_more(token_cache_t& cache) {
        return cache.idx < cache.tokens.size;
    }

    u0 move_next(token_cache_t& cache) {
        ++cache.idx;
    }

    u0 seek_first(token_cache_t& cache) {
        cache.idx = 0;
    }

    token_id_t append(token_cache_t& cache,
                      token_type_t type,
                      const source_info_t& loc) {
        auto& token = array::append(cache.tokens);
        token.id   = cache.tokens.size;
        token.type = type;
        token.loc  = loc;
        return token.id;
    }

    const token_t& current(token_cache_t& cache) {
        return cache.tokens[cache.idx];
    }

    u0 init(token_cache_t& cache, alloc_t* alloc) {
        cache.alloc = alloc;
        cache.idx   = 0;
        array::init(cache.tokens, cache.alloc);
    }

    const token_t& peek(token_cache_t& cache, u32 count) {
        return cache.tokens[cache.idx + count];
    }

    const token_t& get(token_cache_t& cache, token_id_t id) {
        return cache.tokens[id - 1];
    }

    token_id_t append(token_cache_t& cache, token_type_t type) {
        auto& token = array::append(cache.tokens);
        token.id   = cache.tokens.size;
        token.type = type;
        return token.id;
    }
}
