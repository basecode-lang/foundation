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

#include <basecode/vm/vm.h>

namespace basecode::vm {
    static status_t decode_and_execute(vm_t& vm) {
        static u0* s_micro_op[] = {
            &&nop,
            &&exit,
        };

        u8*                 heap_ptr    {(u8*) vm.heap};
        instruction_t*      inst;
        status_t            status;

        VM_NEXT(vm);

        nop:
        {
            VM_NEXT(vm);
        }
        exit:
        {
            status = status_t::exited;
        }

        return status;
    }

    u0 free(vm_t& vm) {
    }

    status_t resume(vm_t& vm) {
        if ((VM_GET_PC(vm) % sizeof(instruction_t)) != 0)
            return status_t::unaligned_bytecode_address;
        return decode_and_execute(vm);
    }

    status_t execute(vm_t& vm, u32 address) {
        if ((address % sizeof(instruction_t)) != 0)
            return status_t::unaligned_bytecode_address;
        VM_SET_PC(vm, address);
        return decode_and_execute(vm);
    }

    status_t init(vm_t& vm, const vm_opts_t& opts, alloc_t* alloc) {
        vm.alloc      = alloc;
        vm.heap_size  = opts.heap_size;
        vm.stack_size = opts.stack_size;
        return status_t::ok;
    }
}
