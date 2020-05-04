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

#include "dlmalloc_system.h"

namespace basecode::memory::dl {
    static u0 release(alloc_t* alloc) {
        destroy_mspace(alloc->subclass.dl.heap);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto subclass = &alloc->subclass.dl;
        auto dl_config = (dl_config_t*) config;
        if (dl_config->base) {
            subclass->base = dl_config->base;
            subclass->size = dl_config->heap_size;
            subclass->heap = create_mspace_with_base(dl_config->base, dl_config->heap_size, 0);
        } else {
            subclass->heap = create_mspace(dl_config->heap_size, 0);
            subclass->base = {};
            subclass->size = dl_config->heap_size;
        }
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto h = system::header(mem);
        freed_size = h->size;
        alloc->total_allocated -= freed_size;
        mspace_free(alloc->subclass.dl.heap, h);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        alloc_size = system::size_with_padding(size, align);
        auto h = (alloc_header_t*) mspace_malloc(alloc->subclass.dl.heap, alloc_size);
        auto p = system::data_pointer(h, align);
        system::fill(h, p, alloc_size);
        alloc->total_allocated += alloc_size;
        return p;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size) {
        auto h = system::header(mem);
        old_size = h->size;
        auto alloc_size = system::size_with_padding(size, align);
        h = (alloc_header_t*) mspace_realloc(alloc->subclass.dl.heap, h, alloc_size);
        h->size = alloc_size;
        alloc->total_allocated += (s32) (alloc_size - old_size);
        return system::data_pointer(h, align);
    }

    alloc_system_t g_system{
        .init       = init,
        .type       = alloc_type_t::dlmalloc,
        .free       = free,
        .alloc      = alloc,
        .release    = release,
        .realloc    = realloc,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}
