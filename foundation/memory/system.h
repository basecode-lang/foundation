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

#pragma once

#include <foundation/types.h>
#include <foundation/context/context.h>
#include "dlmalloc_config.h"

namespace basecode::memory {

    struct header_t final {
        u32 size;
    };

    const u32 header_pad_value = 0xffffffffu;

    inline header_t* header(u0* data) {
        auto p = static_cast<u32*>(data);
        while (p[-1] == header_pad_value)
            --p;
        return reinterpret_cast<header_t*>(p - 1);
    }

    inline u0* align_forward(u0* p, u32 align) {
        auto pi = uintptr_t(p);
        const u32 mod = pi % align;
        if (mod)
            pi += (align - mod);
        return (u0*) pi;
    }

    inline u0* data_pointer(header_t* header, u32 align) {
        u0* p = header + 1;
        return align_forward(p, align);
    }

    inline u0 fill(header_t* header, u0* data, u32 size) {
        header->size = size;
        auto p = reinterpret_cast<u32*>(header + 1);
        while (p < data)
            *p++ = header_pad_value;
    }

    inline u32 size_with_padding(u32 size, u32 align) {
        return size + align + sizeof(header_t);
    }

    ///////////////////////////////////////////////////////////////////////////

    struct allocator_config_t {};

    using init_callback_t       = u0  (*)(allocator_t*, allocator_config_t*);
    using release_callback_t    = u0  (*)(allocator_t*);
    using deallocate_callback_t = u0  (*)(allocator_t*, u0* mem);
    using allocate_callback_t   = u0* (*)(allocator_t*, u32 size, u32 align);
    using reallocate_callback_t = u0* (*)(allocator_t*, u0* mem, u32 new_size, u32 align);

    enum class allocator_type_t : u8 {
        system,
        dlmalloc,
    };

    struct allocator_system_t final {
        init_callback_t init{};
        allocator_type_t type{};
        release_callback_t release{};
        allocate_callback_t  allocate{};
        deallocate_callback_t deallocate{};
        reallocate_callback_t reallocate{};
    };

    struct allocator_t final {
        u32 total_allocated{};
        allocator_t* backing{};
        allocator_system_t* system{};
        union {
            mspace heap;
        } subclass{.heap = {}};
    };

    struct dl_allocator_config_t : allocator_config_t {
        u0* base{};
        u32 heap_size{};
    };

    ///////////////////////////////////////////////////////////////////////////

    u0 shutdown();

    usize os_page_size();

    allocator_t* default_allocator();

    b8 set_page_executable(u0* ptr, usize size);

    u0 initialize(u32 heap_size = 32*1024*1024, u0* base = nullptr);

    ///////////////////////////////////////////////////////////////////////////

    u0 init_allocator(
        allocator_t* allocator,
        allocator_type_t type,
        allocator_config_t* config = nullptr);

    u0 release_allocator(allocator_t* allocator);

    u0 deallocate(allocator_t* allocator, u0* mem);

    u0* allocate(allocator_t* allocator, u32 size, u32 align = sizeof(u32));

    u0* reallocate(allocator_t* allocator, u0* mem, u32 new_size, u32 align = sizeof(u32));

}

