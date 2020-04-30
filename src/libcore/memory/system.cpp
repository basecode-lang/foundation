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
#include <basecode/core/array/array.h>
#include "system.h"
#include "page_system.h"
#include "slab_system.h"
#include "proxy_system.h"
#include "trace_system.h"
#include "default_system.h"
#include "dlmalloc_system.h"

namespace basecode::memory {
    struct system_t final {
        alloc_t                 default_alloc;
        alloc_t                 slab_alloc;
        array_t<alloc_t*>       allocators;
        usize                   os_page_size;
    };

    thread_local system_t       g_system{};

    namespace system {
        u0 shutdown() {
            for (auto alloc : g_system.allocators) {
                memory::release(alloc);
                memory::free(&g_system.slab_alloc, alloc);
            }
            array::free(g_system.allocators);
            release(&g_system.slab_alloc);
            release(&g_system.default_alloc);
        }

        usize os_page_size() {
            return g_system.os_page_size;
        }

        alloc_t* default_alloc() {
            return &g_system.default_alloc;
        }

        u0 free(alloc_t* alloc, b8 enforce) {
            auto idx = array::contains(g_system.allocators, alloc);
            if (idx == -1)
                return;
            array::erase(g_system.allocators, idx);
            memory::release(alloc, enforce);
            memory::free(&g_system.slab_alloc, alloc);
        }

        u0 initialize(u32 heap_size, u0* base) {
#ifdef _WIN32
            SYSTEM_INFO system_info;
            GetSystemInfo(&system_info);
            g_system.os_page_size = system_info.dwAllocationGranularity;
#else
            g_system.os_page_size = sysconf(_SC_PAGE_SIZE);
#endif
            dl_config_t dl_config{};
            dl_config.base = base;
            dl_config.heap_size = heap_size;
            init(&g_system.default_alloc, alloc_type_t::dlmalloc, &dl_config);

            slab_config_t slab_config{};
            slab_config.backing = &g_system.default_alloc;
            slab_config.buf_size = sizeof(alloc_t);
            slab_config.buf_align = alignof(alloc_t);
            memory::init(&g_system.slab_alloc, alloc_type_t::slab, &slab_config);

            array::init(g_system.allocators, &g_system.default_alloc);
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

        alloc_t* make(alloc_type_t type, alloc_config_t* config) {
            auto alloc = (alloc_t*) memory::alloc(&g_system.slab_alloc);
            memory::init(alloc, type, config);
            array::append(g_system.allocators, alloc);
            return alloc;
        }
    }

    u0 release(alloc_t* alloc, b8 enforce) {
        if (!alloc->system || !alloc->system->release) return;
        alloc->system->release(alloc);
        if (enforce) assert(alloc->total_allocated == 0);
        alloc->backing = {};
    }

    u0* alloc(alloc_t* alloc, u32* alloc_size) {
        if (!alloc->system || !alloc->system->alloc) return {};
        u32 temp;
        auto mem = alloc->system->alloc(alloc, 0, 0, temp);
        if (alloc_size) *alloc_size = temp;
        return mem;
    }

    u0 free(alloc_t* alloc, u0* mem, u32* freed_size) {
        if (!mem || !alloc->system || !alloc->system->free) return;
        u32 temp;
        alloc->system->free(alloc, mem, temp);
        if (freed_size) *freed_size = temp;
    }

    u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        if (!mem || !alloc->system || !alloc->system->realloc) return {};
        u32 old_size, new_size;
        return alloc->system->realloc(alloc, mem, size, align, old_size, new_size);
    }

    u0* alloc(alloc_t* alloc, u32 size, u32 align, u32* alloc_size) {
        if (!size || !alloc->system || !alloc->system->alloc) return {};
        u32 temp;
        auto mem = alloc->system->alloc(alloc, size, align, temp);
        if (alloc_size) *alloc_size = temp;
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
            case alloc_type_t::slab: {
                alloc->system = slab::system();
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
            case alloc_type_t::trace: {
                alloc->system = trace::system();
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
