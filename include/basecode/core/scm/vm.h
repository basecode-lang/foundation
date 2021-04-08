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

#include <basecode/core/scm/types.h>

namespace basecode::scm::vm {
    u0 free(vm_t& vm);

    u0 reset(vm_t& vm);

    u32 heap_top(vm_t& vm);

    status_t init(vm_t& vm,
                  alloc_t* alloc = context::top()->alloc,
                  u32 heap_size = 8 * 1024 * 1024);

    status_t step(vm_t& vm, ctx_t* ctx, s32 cycles = -1);

    const mem_map_entry_t* find_mem_map_entry(const vm_t& vm, u8 reg);

    u0 mem_map(vm_t& vm, mem_area_t area, u8 reg, u32 size, b8 top = false);
}
