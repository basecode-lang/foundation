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

#include <cassert>
#include <basecode/core/memory/system/bump.h>

namespace basecode::memory::bump {
    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.bump;
        sc->buf    = {};
        sc->offset = sc->end_offset = {};
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
                alloc->backing  = cfg->backing.alloc;
                assert(alloc->backing);
                break;
        }
        sc->offset = sc->end_offset = {};
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        u32 temp_size{};
        auto sc  = &alloc->subclass.bump;
        if (!sc->buf || sc->offset + (size + align) > sc->end_offset) {
            if (alloc->backing) {
                const auto r = alloc->backing->system->alloc(alloc->backing, size, align);
                sc->buf = r.mem;
            } else {
                // XXX:
                assert(false);
            }
            sc->offset     = {};
            sc->end_offset = temp_size;
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
        assert(a && a->system->type == alloc_type_t::bump);
        auto sc = &a->subclass.bump;
        sc->buf    = {};
        sc->offset = sc->end_offset = {};
    }

    u0 buf(alloc_t* alloc, u0* buf, u32 size) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::bump);
        auto sc = &a->subclass.bump;
        sc->buf        = buf;
        sc->offset     = {};
        sc->end_offset = size;
    }
}
