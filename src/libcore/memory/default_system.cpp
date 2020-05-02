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

#include "default_system.h"

namespace basecode::memory::default_ {
    static u0 free(alloc_t* allocator, u0* mem, u32& freed_size) {
        auto h = system::header(mem);
        freed_size = h->size;
        allocator->total_allocated -= freed_size;
        std::free(h);
    }

    static u0* alloc(alloc_t* allocator, u32 size, u32 align, u32& allocated_size) {
        allocated_size = system::size_with_padding(size, align);
        auto h = (alloc_header_t*) std::malloc(allocated_size);
        auto p = system::data_pointer(h, align);
        system::fill(h, p, allocated_size);
        allocator->total_allocated += allocated_size;
        return p;
    }

    static u0* realloc(alloc_t* allocator, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size) {
        auto h = system::header(mem);
        auto new_data = alloc(allocator, size, align, new_size);
        std::memcpy(new_data, mem, h->size);
        free(allocator, mem, old_size);
        return new_data;
    }

    alloc_system_t g_system{
        .type       = alloc_type_t::system,
        .free       = free,
        .alloc      = alloc,
        .realloc    = realloc,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}
