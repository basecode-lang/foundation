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
#include <basecode/core/scm/bytecode.h>

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

        inline u64 base_addr(mem_area_t& area) {
            return area.top ? u64(area.data) + ((area.capacity - area.size) * sizeof(u64)) :
                   u64(area.data) + (area.size * sizeof(u64));
        }

        u0 resize(mem_area_t& area, u32 new_size);

        u0 reserve(mem_area_t& area, u32 new_capacity);

        u0 reset(mem_area_t& area, b8 zero_mem = false);

        u0 grow(mem_area_t& area, u32 new_capacity = 16);

        u0 shrink_to_size(mem_area_t& area, u32 new_size);

        template <typename T>
        T top(mem_area_t& area) {
            auto& vm = *area.vm;
            return area.size == 0 ? T{} : T(HU(G(area.reg)));
        }

        template <typename T>
        T pop(mem_area_t& area) {
            auto& vm = *area.vm;
            if (area.size > 0) {
                auto tos = HU(G(area.reg));
                if (area.top) {
                    G(area.reg) += sizeof(u64);
                } else {
                    G(area.reg) -= sizeof(u64);
                }
                --area.size;
                return T(tos);
            }
            return {};
        }

        template <typename T>
        u0 push(mem_area_t& area, T value) {
            auto& vm = *area.vm;
            if (area.size + 1 > area.capacity)
                grow(area);
            if (area.top) {
                G(area.reg) -= sizeof(u64);
                HU(G(area.reg)) = u64(value);
            } else {
                HU(G(area.reg)) = u64(value);
                G(area.reg) += sizeof(u64);
            }
            ++area.size;
        }
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
