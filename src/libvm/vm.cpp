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
    u0 free(vm_t& vm) {
    }

    status_t init(vm_t& vm, const vm_opts_t& opts, alloc_t* alloc) {
        vm.alloc      = alloc;
        vm.heap_size  = opts.heap_size;
        vm.stack_size = opts.stack_size;
        return status_t::ok;
    }
}
