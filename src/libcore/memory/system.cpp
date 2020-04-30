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
        alloc_t*                default_alloc;
        usize                   os_page_size;
        u8                      buffer[bootstrap_buffer_size];
    };

    thread_local system_t       g_system{};

    u0 shutdown() {
        release(g_system.default_alloc);
    }

    usize os_page_size() {
        return g_system.os_page_size;
    }

    alloc_t* default_alloc() {
        return g_system.default_alloc;
    }

    u0 free(alloc_t* alloc, u0* mem) {
        if (!mem || !alloc->system || !alloc->system->free) return;
        u32 freed_size;
        alloc->system->free(alloc, mem, freed_size);
    }

    u0 initialize(u32 heap_size, u0* base) {
        g_system.default_alloc = new (g_system.buffer) alloc_t;
        dl_config_t config{
            .base = base,
            .heap_size = heap_size
        };
        init(g_system.default_alloc, alloc_type_t::dlmalloc, &config);
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

    u0 release(alloc_t* alloc, b8 enforce) {
        if (!alloc->system || !alloc->system->release) return;
        alloc->system->release(alloc);
        if (enforce) assert(alloc->total_allocated == 0);
        alloc->backing = {};
    }

    u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        if (!mem || !alloc->system || !alloc->system->realloc) return nullptr;
        u32 old_size, new_size;
        return alloc->system->realloc(alloc, mem, size, align, old_size, new_size);
    }

    u0* alloc(alloc_t* alloc, u32 size, u32 align, u32* alloc_size) {
        if (!size || !alloc->system || !alloc->system->alloc) return nullptr;
        u32 allocated_size;
        auto mem = alloc->system->alloc(alloc, size, align, allocated_size);
        if (alloc_size) *alloc_size = allocated_size;
        return mem;
    }

    status_t init(alloc_t* alloc, alloc_type_t type, alloc_config_t* config) {
        if (!alloc)
            return status_t::invalid_allocator;
        switch (type) {
            case alloc_type_t::bump: {
                alloc->system = bump::system();
                break;
            }
            case alloc_type_t::page: {
                alloc->system = page::system();
                break;
            }
            case alloc_type_t::proxy: {
                alloc->system = proxy::system();
                break;
            }
            case alloc_type_t::system: {
                alloc->system = default_::system();
                break;
            }
            case alloc_type_t::dlmalloc: {
                alloc->system = dl::system();
                break;
            }
        }
        alloc->backing = {};
        alloc->total_allocated = {};
        if (alloc->system->init)
            alloc->system->init(alloc, config);
        return status_t::ok;
    }
}