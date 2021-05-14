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
#include <basecode/core/memory/system/bump.h>

namespace basecode::memory::bump {
    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.bump;
        sc->buf        = {};
        sc->offset     = {};
        sc->end_offset = {};
        return alloc->total_allocated;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (bump_config_t*) config;
        auto sc  = &alloc->subclass.bump;
        switch (cfg->type) {
            case bump_type_t::existing:
                sc->buf = cfg->backing.buf;
                break;
            case bump_type_t::allocator:
                sc->buf = {};
                alloc->backing  = cfg->backing.alloc;
                BC_ASSERT_NOT_NULL(alloc->backing);
                break;
        }
        sc->offset     = {};
        sc->end_offset = {};
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        auto sc = &alloc->subclass.bump;
        const auto next_offset = sc->offset + size + (align * 2);
        if (!sc->buf || next_offset > sc->end_offset) {
            auto r = memory::internal::alloc(alloc->backing, size, align);
            sc->buf        = r.mem;
            sc->offset     = {};
            sc->end_offset = r.size;
        }
        u32  align_adjust{};
        auto mem = memory::system::align_forward((u8*) sc->buf + sc->offset,
                                                 align,
                                                 align_adjust);
        sc->offset += size + align_adjust;
        return mem_result_t{mem, s32(size + align_adjust)};
    }

    alloc_system_t g_system{
        .size       = {},
        .init       = init,
        .fini       = fini,
        .free       = {},
        .alloc      = alloc,
        .realloc    = {},
        .type       = alloc_type_t::bump,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        BC_ASSERT_MSG(a && a->system->type == alloc_type_t::bump,
                      "expected a non-null bump allocator");
        auto sc = &a->subclass.bump;
        sc->buf        = {};
        sc->offset     = {};
        sc->end_offset = {};
    }

    u0 buf(alloc_t* alloc, u0* buf, u32 size) {
        auto a = unwrap(alloc);
        BC_ASSERT_MSG(a && a->system->type == alloc_type_t::bump,
                      "expected a non-null bump allocator");
        auto sc = &a->subclass.bump;
        sc->buf        = buf;
        sc->offset     = {};
        sc->end_offset = size;
    }
}
