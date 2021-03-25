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

#include <basecode/core/buf.h>
#include <basecode/core/array.h>
#include <basecode/core/src_loc.h>
#include <basecode/core/hashable.h>

#define TOKEN_CLS(t)            (u32((t)) >> 16U & 0xffffU)
#define TOKEN_TYPE(cls, type)   (u32((cls)) << 16U | u32((type)))

namespace basecode {
    struct token_t;
    struct token_id_t;
    struct token_type_t;

    using token_list_t          = array_t<token_t>;
    using token_id_list_t       = array_t<token_id_t>;

    struct token_id_t final {
        constexpr token_id_t()       : id(0)        {}
        constexpr token_id_t(u32 id) : id(id)       {}
        constexpr operator u32() const              { return id;        }
        [[nodiscard]] constexpr b8 empty() const    { return id == 0;   }
        static constexpr token_id_t null()          { return 0;         }
    private:
        u32                     id;
    };

    struct token_type_t final {
        constexpr token_type_t()         : cls(0), type(0)
                                                    {}
        constexpr token_type_t(u32 type) : cls(type >> 16U & 0xffffU), type(type & 0xffffU)
                                                    {}
        constexpr operator u32() const              { return TOKEN_TYPE(cls, type);     }
        [[nodiscard]] constexpr b8 empty() const    { return type == 0 && cls == 0;     }
        static constexpr token_type_t none()        { return 0;                         }
    private:
        u32                     cls:    16;
        u32                     type:   16;
    };

    struct token_t final {
        token_id_t              id;
        source_info_t           loc;
        token_type_t            type;
    };

    struct token_cache_t final {
        alloc_t*                alloc;
        token_list_t            tokens;
        u32                     idx;
    };

    struct prefix_rule_t final {
        token_type_t            plus;
        token_type_t            minus;
    };

    struct operator_precedence_t final {
        u8                      left;
        u8                      right;
    };

    namespace token {
        namespace cache {
            u0 free(token_cache_t& cache);

            u32 size(token_cache_t& cache);

            b8 has_more(token_cache_t& cache);

            u0 move_next(token_cache_t& cache);

            u0 seek_first(token_cache_t& cache);

            const token_t& current(token_cache_t& cache);

            const token_t& get(token_cache_t& cache, token_id_t id);

            const token_t& peek(token_cache_t& cache, u32 count = 1);

            token_id_t append(token_cache_t& cache, token_type_t type);

            u0 init(token_cache_t& cache, alloc_t* alloc = context::top()->alloc);

            token_id_t append(token_cache_t& cache, token_type_t type, const source_info_t& loc);
        }

        str::slice_t span(const token_t& token, const Buffer_Concept auto& buf) {
            return slice::make(buf.data + token.loc.pos.start, (token.loc.pos.end - token.loc.pos.start) + 1);
        }
    }

    namespace hash {
        inline u64 hash64(const token_type_t& key) {
            u32 value = key;
            return murmur::hash64(&value, sizeof(u32));
        }
    }
}
