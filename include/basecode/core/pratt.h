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

#include <basecode/core/types.h>

namespace basecode::pratt {
    namespace parser {
        ast::node_id_t expression(pratt_ctx_t& ctx, u32 rbp = 0);
    }

    namespace grammar {
        u0 free(grammar_t& grammar);

        rule_t* infix(grammar_t& grammar,
                      token_type_t type,
                      u32 bp,
                      led_t led);

        rule_t* infixr(grammar_t& grammar,
                       token_type_t type,
                       u32 bp,
                       led_t led);

        rule_t* postfix(grammar_t& grammar,
                        token_type_t type,
                        u32 bp,
                        led_t led);

        rule_t* terminal(grammar_t& grammar,
                         token_type_t type,
                         u32 bp = 0);

        u0 init(grammar_t& grammar,
                u32* operator_types,
                alloc_t* alloc = context::top()->alloc.main);

        rule_t* stmt(grammar_t& grammar, token_type_t type, std_t std);

        rule_t* prefix(grammar_t& grammar, token_type_t type, nud_t nud);
    }
}
