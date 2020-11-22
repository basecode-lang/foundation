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

#include <basecode/core/memory.h>
#include <basecode/vm/configure.h>

namespace basecode {
    struct vm_t;
    struct register_t;
    struct flag_register_t;

    constexpr u32 reg_file_size = 32;

    union register_alias_t final {
        u8                      b;
        s8                      sb;
        u16                     w;
        s16                     sw;
        u32                     dw;
        s32                     sdw;
        f32                     fdw;
        u64                     qw;
        s64                     sqw;
        f64                     fqw;
    };

    struct register_t final {
        register_alias_t        alias;
    };

    struct flag_register_t final {
        u64                     n:      1;
        u64                     z:      1;
        u64                     c:      1;
        u64                     v:      1;
        u64                     i:      1;
        u64                     pad:    58;
    };

    struct vm_opts_t final {
        u32                     heap_size;
        u32                     stack_size;
    };

    struct vm_t final {
        alloc_t*                alloc;
        u0*                     heap;
        register_t              pc;
        register_t              sp;
        register_t              lr;
        flag_register_t         fr;
        register_t              gp[reg_file_size];
        u32                     stack_size;
        u32                     heap_size;
    };

    namespace vm {
        enum class status_t : u32 {
            ok,
            error               = 30000
        };

        u0 free(vm_t& vm);

        status_t init(vm_t& vm, const vm_opts_t& opts = {}, alloc_t* alloc = context::top()->alloc);
    }
}
