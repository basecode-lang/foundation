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

#define WITH_ALLOC(Alloc, Code)                                     \
        do {                                                        \
            if (context::top()->alloc != Alloc) {                   \
                auto ctx = *context::top();                         \
                ctx.alloc = Alloc;                                  \
                context::push(&ctx);                                \
                Code                                                \
                context::pop();                                     \
            } else {                                                \
                Code                                                \
            }                                                       \
        } while (false)
#define WITH_LOGGER(Logger, Code)                                   \
        do {                                                        \
            if (context::top()->logger != Logger) {                 \
                auto ctx = *context::top();                         \
                ctx.logger = Logger;                                \
                context::push(&ctx);                                \
                Code                                                \
                context::pop();                                     \
            } else {                                                \
                Code                                                \
            }                                                       \
        } while (false)

namespace basecode {
    struct alloc_t;
    struct logger_t;

    namespace scm {
        struct ctx_t;
    }

    struct context_t {
        alloc_t*            alloc;
        alloc_t*            scratch_alloc;
        logger_t*           logger;
        scm::ctx_t*         scm_ctx;
        u0*                 user;
        const s8**          argv;
        s32                 argc;
    };

    namespace context {
        u0 pop();

        context_t* top();

        u0 push(context_t* ctx);

        context_t make(s32 argc, const s8** argv, alloc_t* alloc, logger_t* logger = {});
    }
}
