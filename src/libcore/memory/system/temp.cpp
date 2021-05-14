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

#include <basecode/core/bits.h>
#include <basecode/core/assert.h>
#include <basecode/core/memory/system/temp.h>

namespace basecode::memory::temp {
    static mem_result_t temp_alloc(alloc_t* alloc, u32 size, u32 align);

    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.temp;
        memory::internal::free(alloc->backing, sc->buf);
        sc->buf        = {};
        sc->offset     = {};
        sc->last_alloc = {};
        sc->end_offset = {};
        return alloc->total_allocated;
    }

    static mem_result_t temp_realloc(alloc_t* alloc,
                                     u0* mem,
                                     u32 size,
                                     u32 align) {
        auto sc = &alloc->subclass.temp;
        if (mem && mem == sc->last_alloc) {
            BC_ASSERT_MSG(sc->offset + size < sc->end_offset,
                          "temp allocator exhausted");
            return mem_result_t{mem, s32(size)};
        }
        return temp_alloc(alloc, size, align);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (temp_config_t*) config;
        alloc->backing = cfg->backing.alloc;
        BC_ASSERT_NOT_NULL(alloc->backing);
        auto sc = &alloc->subclass.temp;
        auto r = memory::internal::alloc(alloc->backing,
                                         cfg->size,
                                         sizeof(u64));
        sc->buf        = (u8*) r.mem;
        sc->offset     = {};
        sc->last_alloc = {};
        sc->end_offset = r.size;
    }

    static mem_result_t temp_alloc(alloc_t* alloc, u32 size, u32 align) {
        auto sc = &alloc->subclass.temp;
        const auto next_offset = sc->offset + size + (align * 2);
        BC_ASSERT_MSG(sc->buf && next_offset < sc->end_offset,
                      "temp allocator exhausted");
        u32  align_adjust{};
        auto mem = memory::system::align_forward(sc->buf + sc->offset,
                                                 align,
                                                 align_adjust);
        sc->last_alloc = (u8*) mem;
        sc->offset += size + align_adjust;
        return mem_result_t{mem, s32(size + align_adjust)};
    }

    alloc_system_t g_system{
        .size       = {},
        .init       = init,
        .fini       = fini,
        .free       = {},
        .alloc      = temp_alloc,
        .realloc    = temp_realloc,
        .type       = alloc_type_t::temp,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        BC_ASSERT_MSG(a && a->system->type == alloc_type_t::temp,
                      "expected a non-null temp allocator");
        auto sc = &a->subclass.temp;
        sc->offset = {};
        alloc->total_allocated = {};
    }
}
