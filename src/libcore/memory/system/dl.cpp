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

#include <basecode/core/memory/system/dl.h>
#include <basecode/core/memory/system/dlmalloc_config.h>

namespace basecode::memory::dl {
    static u32 fini(alloc_t* alloc) {
        destroy_mspace(alloc->subclass.dl.heap);
        return alloc->total_allocated;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        return mspace_usable_size(mem);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto sc        = &alloc->subclass.dl;
        auto dl_config = (dl_config_t*) config;
        if (dl_config->base) {
            sc->base = dl_config->base;
            sc->size = dl_config->heap_size;
            sc->heap = create_mspace_with_base(dl_config->base,
                                               dl_config->heap_size,
                                               0);
        } else {
            sc->heap = create_mspace(dl_config->heap_size, 0);
            sc->base = {};
            sc->size = dl_config->heap_size;
        }
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        const auto freed_size = mspace_usable_size(mem);
        mspace_free(alloc->subclass.dl.heap, mem);
        return freed_size;
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        mem_result_t r{};
        r.mem  = mspace_memalign(alloc->subclass.dl.heap, align, size);
        r.size = mspace_usable_size(r.mem);
        return r;
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        auto sc = &alloc->subclass.dl;
        mem_result_t r{};
        if (mem) {
            r.size = mspace_usable_size(mem);
            r.mem = mspace_realloc_in_place(sc->heap, mem, size);
            if (!r.mem) {
                r.mem = mspace_memalign(sc->heap, align, size);
                std::memcpy(r.mem, mem, size < r.size ? size : r.size);
                mspace_free(sc->heap, mem);
            }
        } else {
            r.size = 0;
            r.mem = mspace_memalign(sc->heap, align, size);
        }
        const auto actual_size = mspace_usable_size(r.mem);
        r.size = s32(actual_size - r.size);
        return r;
    }

    alloc_system_t g_system{
        .size                   = size,
        .init                   = init,
        .fini                   = fini,
        .free                   = free,
        .alloc                  = alloc,
        .realloc                = realloc,
        .type                   = alloc_type_t::dlmalloc,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}
