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

#include <basecode/core/hashtab.h>

namespace basecode::scm {
    struct ctx_t;
    struct obj_t;
    struct env_t;

    enum class memory_area_t : u8 {
        code,
        heap,
        env_stack,
        code_stack,
        data_stack,
    };

    [[maybe_unused]] constexpr u32 max_memory_areas = 6;

    struct memory_map_entry_t final {
        u64                     addr;
        u32                     offs;
        u32                     size;
        u8                      reg;
        b8                      top;
        b8                      valid;
    };

    struct memory_map_t final {
        u32                     heap_size;
        s32                     reg_to_entry[32];
        memory_map_entry_t      entries[max_memory_areas];
    };

    namespace trap {
        constexpr u8 hash       = 1;
        constexpr u8 functor    = 2;

        str::slice_t name(u8 type);
    }

    struct flag_register_t final {
        u64                     n:      1;
        u64                     z:      1;
        u64                     c:      1;
        u64                     v:      1;
        u64                     i:      1;
        u64                     pad:    58;
    };

    struct vm_t;

    using trap_callback_t       = b8 (*)(vm_t& vm, u64 arg);
    using trap_map_t            = hashtab_t<u32, trap_callback_t>;

    struct vm_t final {
        alloc_t*                alloc;
        u64*                    heap;
        trap_map_t              traps;
        memory_map_t            memory_map;
        b8                      exited;

        u64& operator[](u32 addr)       { return heap[addr >> 3]; }
        u64 operator[](u32 addr) const  { return heap[addr >> 3]; }
    };

    namespace vm {
        enum class status_t : u8 {
            ok,
            fail,
            error,
            unresolved_label,
        };

        u0 free(vm_t& vm);

        u0 reset(vm_t& vm);

        u32 heap_top(vm_t& vm);

        status_t init(vm_t& vm,
                      alloc_t* alloc = context::top()->alloc,
                      u32 heap_size = 8 * 1024 * 1024);

        status_t step(vm_t& vm, ctx_t* ctx, s32 cycles = -1);

        const memory_map_entry_t* find_memory_map_entry(const vm_t& vm, u8 reg);

        u0 memory_map(vm_t& vm, memory_area_t area, u8 reg, u32 size, b8 top = false);
    }
}
