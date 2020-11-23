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
#include <basecode/vm/bytecode.h>
#include <basecode/vm/configure.h>

#define VM_INC_PC(v)            ((v).gp[register_file::pc].qw++)
#define VM_GET_PC(v)            ((v).gp[register_file::pc].qw)
#define VM_SET_PC(v, a)         ((v).gp[register_file::pc].qw = (a))
#define VM_NEXT(v)              SAFE_SCOPE(                                     \
        inst      = (instruction_t*) qword_ptr[VM_INC_PC((v))];                 \
        inst_data = inst->data;                                                 \
        opers     = (operand_data_t*) &inst_data;                               \
        goto *s_micro_op[0];                                                    \
    )

namespace basecode {
    struct vm_t;
    struct gp_register_t;
    struct flag_register_t;

    constexpr u32 reg_file_size = 36;

    struct gp_register_t final {
        union {
            u8                  b;
            s8                  sb;
            u16                 w;
            s16                 sw;
            u32                 dw;
            s32                 sdw;
            f32                 fdw;
            u64                 qw;
            s64                 sqw;
            f64                 fqw;
        };
    };

    struct fr_register_t final {
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
        fr_register_t           fr;
        gp_register_t           gp[reg_file_size];
        u32                     stack_size;
        u32                     heap_size;
    };

    namespace vm {
        enum class status_t : u32 {
            ok,
            error                       = 30000,
            exited,
            suspend,
            unaligned_bytecode_address,
        };

        u0 free(vm_t& vm);

        status_t init(vm_t& vm,
                      const vm_opts_t& opts = {},
                      alloc_t* alloc = context::top()->alloc);

        status_t resume(vm_t& vm);

        status_t execute(vm_t& vm, u32 address);
    }
}
