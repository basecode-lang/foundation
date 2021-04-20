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

#include <basecode/core/assert.h>
#include <basecode/core/context.h>

namespace basecode::context {
    static constexpr u32        stack_size = 512;

    thread_local u32            t_index = stack_size;
    thread_local context_t*     t_stack[stack_size];

    u0 pop() {
        BC_ASSERT_MSG(t_index < stack_size, "context stack underflow");
        t_index++;
    }

    context_t* top() {
        BC_ASSERT_MSG(t_index < stack_size, "context stack underflow");
        return t_stack[t_index];
    }

    u0 push(context_t* ctx) {
        BC_ASSERT_MSG(t_index > 0, "context stack overflow");
        t_stack[--t_index] = ctx;
    }

    context_t make(s32 argc, const s8** argv) {
        context_t ctx{};
        ctx.user   = {};
        ctx.argc   = argc;
        ctx.argv   = argv;
        return ctx;
    }
}
