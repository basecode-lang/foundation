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
#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif
#include <cassert>
#include <sys/mman.h>
#include "system.h"
#include "bump_system.h"
#include "page_system.h"
#include "proxy_system.h"
#include "default_system.h"
#include "dlmalloc_system.h"

namespace basecode::memory {
    static constexpr u32 bootstrap_buffer_size  = sizeof(alloc_t);

    struct system_t final {
        alloc_t*                default_allocator;
        usize                   os_page_size;
        u8                      buffer[bootstrap_buffer_size];
    };

    thread_local system_t g_system{};

    u0 shutdown() {
        release(g_system.default_allocator);
    }

    usize os_page_size() {
        return g_system.os_page_size;
    }

    alloc_t* default_allocator() {
        return g_system.default_allocator;
    }

    u0 initialize(u32 heap_size, u0* base) {
        g_system.default_allocator = new (g_system.buffer) alloc_t;
        dl_config_t config{
            .base = base,
            .heap_size = heap_size
        };
        init(g_system.default_allocator, alloc_type_t::dlmalloc, &config);
#ifdef _WIN32
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        g_system.os_page_size = system_info.dwAllocationGranularity;
#else
        g_system.os_page_size = sysconf(_SC_PAGE_SIZE);
#endif
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

    u0 free(alloc_t* allocator, u0* mem) {
        if (!mem) return;
        assert(allocator && allocator->system);
        u32 freed_size;
        allocator->system->free(allocator, mem, freed_size);
    }

    u0 release(alloc_t* allocator, b8 enforce) {
        assert(allocator && allocator->system);
        if (allocator->system->release)
            allocator->system->release(allocator);
        if (enforce) assert(allocator->total_allocated == 0);
        allocator->backing = {};
    }

    u0* alloc(alloc_t* allocator, u32 size, u32 align, u32* alloc_size) {
        assert(allocator && allocator->system);
        u32 allocated_size;
        auto mem = allocator->system->alloc(allocator, size, align, allocated_size);
        if (alloc_size) *alloc_size = allocated_size;
        return mem;
    }

    u0* realloc(alloc_t* allocator, u0* mem, u32 size, u32 align) {
        if (!mem) return nullptr;
        assert(allocator && allocator->system);
        u32 old_size, new_size;
        return allocator->system->realloc(allocator, mem, size, align, old_size, new_size);
    }

    u0 init(alloc_t* allocator, alloc_type_t type, alloc_config_t* config) {
        assert(allocator);
        switch (type) {
            case alloc_type_t::bump: {
                allocator->system = bump::system();
                break;
            }
            case alloc_type_t::page: {
                allocator->system = page::system();
                break;
            }
            case alloc_type_t::proxy: {
                allocator->system = proxy::system();
                break;
            }
            case alloc_type_t::system: {
                allocator->system = default_::system();
                break;
            }
            case alloc_type_t::dlmalloc: {
                allocator->system = dl::system();
                break;
            }
        }
        allocator->backing = {};
        allocator->total_allocated = {};
        if (allocator->system->init)
            allocator->system->init(allocator, config);
    }
}