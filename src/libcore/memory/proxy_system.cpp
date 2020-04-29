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

#include "system.h"
#include "proxy_system.h"

namespace basecode::memory::proxy {
    static u0 release(alloc_t* alloc) {
        alloc->backing->system->release(alloc->backing);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto proxy_config = (proxy_config_t*) config;
        alloc->backing = proxy_config->backing;
        assert(alloc->backing);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        alloc->backing->system->free(alloc->backing, mem, freed_size);
        alloc->total_allocated -= freed_size;
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto mem = alloc->backing->system->alloc(alloc->backing, size, align, allocated_size);
        alloc->total_allocated += allocated_size;
        return mem;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size) {
        auto new_mem = alloc->backing->system->realloc(
            alloc->backing,
            mem,
            size,
            align,
            old_size,
            new_size);
        alloc->total_allocated += (s32) new_size - (s32) old_size;
        return new_mem;
    }

    alloc_system_t g_system{
        .init       = init,
        .type       = alloc_type_t::proxy,
        .free       = free,
        .alloc      = alloc,
        .release    = release,
        .realloc    = realloc,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}