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
    namespace mem_area {
        status_t init(mem_area_t& area,
                      vm_t* vm,
                      u32 id,
                      mem_area_type_t type,
                      reg_t reg,
                      alloc_t* alloc,
                      u32 min_capacity,
                      b8 top);

        u0 free(mem_area_t& area);

        u0 resize(mem_area_t& area, u32 new_size);

        u0 reserve(mem_area_t& area, u32 new_capacity);

        u0 reset(mem_area_t& area, b8 zero_mem = false);

        u0 grow(mem_area_t& area, u32 new_capacity = 16);
    }

    u0 free(vm_t& vm);

    u0 reset(vm_t& vm);

    mem_area_t& add_mem_area(vm_t& vm,
                             mem_area_type_t type,
                             reg_t reg,
                             alloc_t* alloc,
                             u32 min_capacity,
                             b8 top = false);

    mem_area_t* get_mem_area(vm_t& vm, u32 id);

    mem_area_t* get_mem_area_by_reg(vm_t& vm, reg_t reg);

    status_t step(vm_t& vm, ctx_t* ctx, s32 cycles = -1);

    status_t init(vm_t& vm, alloc_t* alloc = context::top()->alloc);
}
