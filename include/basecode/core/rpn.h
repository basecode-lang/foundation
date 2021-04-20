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

#include <basecode/core/token.h>

namespace basecode {
    struct postfix_expr_t final {
        token_id_list_t         tokens;
        u32                     arg_count:  8;
        u32                     stmt_count: 24;
    };

    using postfix_expr_list_t   = array_t<postfix_expr_t>;

    struct postfix_t final {
        alloc_t*                alloc;
        token_cache_t*          tokens;
        operator_precedence_t*  operator_precedences;
        postfix_expr_list_t     exprs;
    };

    namespace token::cls {
        [[maybe_unused]] constexpr u16 none                 = 0;
        [[maybe_unused]] constexpr u16 operator_            = 1;
        [[maybe_unused]] constexpr u16 param_end            = 2;
        [[maybe_unused]] constexpr u16 scope_end            = 3;
        [[maybe_unused]] constexpr u16 param_begin          = 4;
        [[maybe_unused]] constexpr u16 scope_begin          = 5;
        [[maybe_unused]] constexpr u16 call_operator        = 7;
        [[maybe_unused]] constexpr u16 call_terminator      = 8;
        [[maybe_unused]] constexpr u16 stmt_terminator      = 9;
        [[maybe_unused]] constexpr u16 operator_consumer    = 10;

        str::slice_t name(token_type_t type);
    }

    namespace rpn {
        enum class status_t : u8 {
            ok                                  = 0,
            error                               = 200,
            invalid_operator_precedence_array   = 201
        };

        namespace expr {
            u0 free(postfix_expr_t& expr);

            u0 init(postfix_expr_t& expr,
                    alloc_t* alloc = context::top()->alloc.main);
        }

        u0 free(postfix_t& postfix);

        b8 to_postfix(postfix_t& postfix);

        postfix_expr_t* add_expr(postfix_t& postfix);

        status_t init(postfix_t& postfix,
                      token_cache_t* tokens,
                      alloc_t* alloc = context::top()->alloc.main);
    }
}
