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
    static u0 release(alloc_t* allocator) {
        destroy_mspace(allocator->subclass.heap);
    }

    static u0 init(alloc_t* allocator, alloc_config_t* config) {
        auto dl_config = (dl_config_t*) config;
        if (dl_config->base) {
            allocator->subclass.heap = create_mspace_with_base(
                dl_config->base,
                dl_config->heap_size,
                0);
        } else {
            allocator->subclass.heap = create_mspace(dl_config->heap_size, 0);
        }
    }

    static u0 free(alloc_t* allocator, u0* mem, u32& freed_size) {
        auto h = header(mem);
        freed_size = h->size;
        allocator->total_allocated -= freed_size;
        mspace_free(allocator->subclass.heap, h);
    }

    static u0* alloc(alloc_t* allocator, u32 size, u32 align, u32& allocated_size) {
        allocated_size = size_with_padding(size, align);
        auto h = (alloc_header_t*) mspace_malloc(allocator->subclass.heap, allocated_size);
        auto p = data_pointer(h, align);
        fill(h, p, allocated_size);
        allocator->total_allocated += allocated_size;
        return p;
    }

    static u0* realloc(alloc_t* allocator, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size) {
        auto h = header(mem);
        auto new_data = alloc(allocator, size, align, new_size);
        std::memcpy(new_data, mem, h->size);
        free(allocator, mem, old_size);
        return new_data;
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