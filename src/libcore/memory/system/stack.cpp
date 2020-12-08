// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/memory/system/stack.h>

namespace basecode::memory::stack {
    static u32 free(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        UNUSED(mem);
        return 0;
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        UNUSED(size);
        UNUSED(align);
        UNUSED(alloc);
        return mem_result_t{alloca(size), s32(size)};
    }

    alloc_system_t                      g_stack_system{
        .size                           = {},
        .init                           = {},
        .fini                           = {},
        .free                           = free,
        .alloc                          = alloc,
        .realloc                        = {},
        .type                           = alloc_type_t::stack,
    };

    alloc_system_t* system() {
        return &g_stack_system;
    }
}
