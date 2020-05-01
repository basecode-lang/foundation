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
#include <basecode/core/format/system.h>
#include "system.h"
#include "bump_system.h"

namespace basecode::memory::bump {
    static u0 release(alloc_t* alloc) {
        auto subclass = &alloc->subclass.bump;
        subclass->buf = {};
        subclass->offset = subclass->end_offset = {};
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto bump_config = (bump_config_t*) config;
        auto subclass = &alloc->subclass.bump;
        switch (bump_config->type) {
            case bump_type_t::exiting:
                subclass->buf = bump_config->backing.buf;
                break;
            case bump_type_t::allocator:
                alloc->backing = bump_config->backing.alloc;
                assert(alloc->backing);
                break;
        }
        subclass->offset = subclass->end_offset = {};
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto subclass = &alloc->subclass.bump;
        if (!subclass->buf || subclass->offset + (size + align) > subclass->end_offset) {
            if (alloc->backing) {
                subclass->buf = alloc->backing->system->alloc(alloc->backing, size, align, allocated_size);
            } else {
                // XXX:
                assert(false);
            }
            subclass->offset = {};
            subclass->end_offset = allocated_size;
        }
        u32 align_adjust{};
        auto mem = memory::system::align_forward((u8*) subclass->buf + subclass->offset, align, align_adjust);
        subclass->offset += size + align_adjust;
        allocated_size = size + align_adjust;
        return mem;
    }

    alloc_system_t g_system{
        .init       = init,
        .type       = alloc_type_t::bump,
        .alloc      = alloc,
        .release    = release,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::bump);
        auto subclass = &a->subclass.bump;
        subclass->buf = {};
        subclass->offset = subclass->end_offset = {};
    }

    u0 buf(alloc_t* alloc, u0* buf, u32 size) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::bump);
        auto subclass = &a->subclass.bump;
        subclass->buf = buf;
        subclass->offset = {};
        subclass->end_offset = size;
    }
}
