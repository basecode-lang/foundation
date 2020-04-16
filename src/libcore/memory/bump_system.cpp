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
        subclass->buf = {};
        alloc->backing = bump_config->backing;
        subclass->offset = subclass->end_offset = {};
        assert(alloc->backing);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto subclass = &alloc->subclass.bump;
        if (!subclass->buf || subclass->offset + (size + align) > subclass->end_offset) {
            subclass->buf = alloc->backing->system->alloc(alloc->backing, size, align, allocated_size);
            subclass->offset = {};
            subclass->end_offset = allocated_size;
        }
        auto mem = (u8*) subclass->buf + subclass->offset;
        subclass->offset += size;
        allocated_size = size;
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
        auto subclass = &alloc->subclass.bump;
        subclass->buf = {};
        subclass->offset = subclass->end_offset = {};
    }
}