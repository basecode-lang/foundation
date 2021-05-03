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

namespace basecode::rpn {
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
