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

#include <basecode/core/memory/system/stack.h>

namespace basecode::memory::stack {
    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.stack;
        const auto size = uintptr_t(sc->free) - uintptr_t(sc->buf);
        memory::internal::free(alloc->backing, sc->buf);
        sc->buf  = sc->free = {};
        return size;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        auto sc = &alloc->subclass.stack;
        const auto size = uintptr_t(sc->free) - uintptr_t(mem);
        sc->free = mem;
        return size;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto sc  = &alloc->subclass.stack;
        auto cfg = (stack_config_t*) config;
        sc->max_size   = cfg->max_size;
        alloc->backing = cfg->backing;

        auto r = memory::internal::alloc(alloc->backing,
                                         sc->max_size,
                                         alignof(u64));
        sc->buf  = r.mem;
        sc->free = sc->buf;
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        auto sc = &alloc->subclass.stack;
        assert(size < sc->max_size);
        mem_result_t r{};
        u32 align_adjust{};
        r.mem    = memory::system::align_forward(sc->free,
                                                 align,
                                                 align_adjust);
        r.size   = size + align_adjust;
        sc->free = (u8*) sc->free + r.size;
        return r;
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        auto sc = &alloc->subclass.stack;
        assert(size < sc->max_size);
        mem_result_t r{};
        if (mem) {
            const auto old_size = uintptr_t(sc->free) - uintptr_t(mem);
            r.mem    = mem;
            r.size   = s32(size - old_size);
            sc->free = (u8*) mem + size;
        } else {
            u32 align_adjust{};
            r.mem    = memory::system::align_forward(sc->free,
                                                     align,
                                                     align_adjust);
            r.size   = size + align_adjust;
            sc->free = (u8*) sc->free + size;
        }
        return r;
    }

    alloc_system_t              g_stack_system{
        .size                   = {},
        .init                   = init,
        .fini                   = fini,
        .free                   = free,
        .alloc                  = alloc,
        .realloc                = realloc,
        .type                   = alloc_type_t::stack,
    };

    alloc_system_t* system() {
        return &g_stack_system;
    }
}
