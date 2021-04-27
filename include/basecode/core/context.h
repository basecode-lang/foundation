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

#include <basecode/core/assert.h>

#define WITH_ALLOC(Alloc, Code)                                     \
        do {                                                        \
            if (context::top()->alloc.main != Alloc) {              \
                auto ctx = *context::top();                         \
                ctx.alloc.main = Alloc;                             \
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

namespace basecode::context {
    constexpr u32 stack_size = 512;

    static inline thread_local u32             t_index = stack_size;
    static inline thread_local context_t*      t_stack[stack_size];

    inline u0 pop() {
        BC_ASSERT_MSG(t_index < stack_size, "context stack underflow");
        t_index++;
    }

    inline context_t* top() {
        BC_ASSERT_MSG(t_index < stack_size, "context stack underflow");
        return t_stack[t_index];
    }

    inline u0 push(context_t* ctx) {
        BC_ASSERT_MSG(t_index > 0, "context stack overflow");
        t_stack[--t_index] = ctx;
    }

    inline context_t make(s32 argc, const s8** argv) {
        return {.argv = argv, .argc = argc};
    }
}
