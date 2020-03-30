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

#include <new>
#include <sys/mman.h>
#include "system.h"

namespace basecode::memory {

    struct system_t final {
        static constexpr u32 bootstrap_buffer_size = sizeof(allocator_t);

        u8 buffer[bootstrap_buffer_size]{};

        usize os_page_size = 4096;         // XXX: this is temporary!
        allocator_t* default_allocator{};
    };

    system_t g_system{};

    ///////////////////////////////////////////////////////////////////////////

    u0 shutdown() {
        destroy_mspace(g_system.default_allocator->subclass.heap);
    }

    usize os_page_size() {
        return g_system.os_page_size;
    }

    allocator_t* default_allocator() {
        return g_system.default_allocator;
    }

    b8 set_page_executable(u0* ptr, usize size) {
        const auto page_size = g_system.os_page_size;
        u64 start, end;
        start = (u64) ptr & ~(page_size - 1);
        end = (u64) ptr + size;
        end = (end + page_size - 1) & ~(page_size - 1);
        return mprotect(
            (u0*) start,
            end - start,
            PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
    }

    u0 initialize(u32 heap_size, u0* base) {
        g_system.default_allocator = new (g_system.buffer) allocator_t;
        g_system.default_allocator->type = allocator_type_t::dlmalloc;
        if (base) {
            g_system.default_allocator->subclass.heap = create_mspace_with_base(base, heap_size, 0);
        } else {
            g_system.default_allocator->subclass.heap = create_mspace(heap_size, 0);
        }
    }

}