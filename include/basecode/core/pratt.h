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

#include <basecode/core/ast.h>
#include <basecode/core/array.h>
#include <basecode/core/hashtab.h>

namespace basecode {
    struct rule_t;
    struct pratt_ctx_t;

    using rule_map_t            = hashtab_t<token_type_t, rule_t>;
    using std_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using nud_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using led_t                 = ast::node_id_t (*)(pratt_ctx_t*, ast::node_id_t);

    struct rule_t final {
        std_t                   std;
        nud_t                   nud;
        led_t                   led;
        u32                     lbp;
        u8                      postfix:        1;
        u8                      suppress_led:   1;
        u8                      associativity:  2;
        u8                      pad:            3;
    };

    struct grammar_t final {
        alloc_t*                alloc;
        rule_map_t              rules;
        u32*                    operator_types;
        u32                     default_rbp;
    };

    struct pratt_ctx_t final {
        bass_t*                 ast;
        rule_t*                 rule;
        const token_t*          token;
        grammar_t*              grammar;
        token_cache_t*          token_cache;
    };

    namespace pratt {
        namespace associativity {
            [[maybe_unused]] constexpr u8 none  = 0b0000;
            [[maybe_unused]] constexpr u8 left  = 0b0001;
            [[maybe_unused]] constexpr u8 right = 0b0010;
        }

        namespace parser {
            ast::node_id_t expression(pratt_ctx_t& ctx, u32 rbp = 0);
        }

        namespace grammar {
            u0 free(grammar_t& grammar);

            rule_t* stmt(grammar_t& grammar, token_type_t type, std_t std);

            rule_t* prefix(grammar_t& grammar, token_type_t type, nud_t nud);

            rule_t* terminal(grammar_t& grammar, token_type_t type, u32 bp = 0);

            rule_t* infix(grammar_t& grammar, token_type_t type, u32 bp, led_t led);

            rule_t* infixr(grammar_t& grammar, token_type_t type, u32 bp, led_t led);

            rule_t* postfix(grammar_t& grammar, token_type_t type, u32 bp, led_t led);

            u0 init(grammar_t& grammar, u32* operator_types, alloc_t* alloc = context::top()->alloc);
        }
    }
}
