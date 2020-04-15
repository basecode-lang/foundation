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
#include <cstring>
#include <sys/mman.h>
#include "system.h"

namespace basecode::memory {

    struct system_t final {
        static constexpr u32 bootstrap_buffer_size = sizeof(allocator_t);

        u8 buffer[bootstrap_buffer_size]{};

        usize os_page_size{};
        allocator_t* default_allocator{};
    };

    system_t g_system{};

    ///////////////////////////////////////////////////////////////////////////

    u0 shutdown() {
        release_allocator(g_system.default_allocator);
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
        dl_config_t config{
            .base = base,
            .heap_size = heap_size
        };
        init_allocator(
            g_system.default_allocator,
            allocator_type_t::dlmalloc,
            &config);
#ifdef _WIN32
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        g_system.os_page_size = system_info.dwAllocationGranularity;
#else
        g_system.os_page_size = sysconf(_SC_PAGE_SIZE);
#endif
    }

    ///////////////////////////////////////////////////////////////////////////

    namespace bump {
        static u0 release(allocator_t* allocator) {
            memory::deallocate(allocator->backing, allocator->subclass.bump.buf);
        }

        static u0 deallocate(allocator_t* allocator, u0* mem) {
        }

        static u0* allocate(allocator_t* allocator, u32 size, u32 align) {
            return {};
        }

        static u0 init(allocator_t* allocator, allocator_config_t* config) {
            auto bump_config = (bump_config_t*)config;
            allocator->backing = bump_config->backing;
            allocator->subclass.bump.page_size = bump_config->page_size;
        }

        static u0* reallocate(allocator_t* allocator, u0* mem, u32 new_size, u32 align) {
            return {};
        }
    }

    namespace system {
        static u0 deallocate(allocator_t* allocator, u0* mem) {
            auto h = header(mem);
            allocator->total_allocated -= h->size;
            std::free(h);
        }

        static u0* allocate(allocator_t* allocator, u32 size, u32 align) {
            const u32 total_size = size_with_padding(size, align);
            auto h = (header_t*) std::malloc(total_size);
            auto p = data_pointer(h, align);
            fill(h, p, total_size);
            allocator->total_allocated += total_size;
            return p;
        }

        static u0* reallocate(allocator_t* allocator, u0* mem, u32 new_size, u32 align) {
            auto h = header(mem);
            auto new_data = system::allocate(allocator, new_size, align);
            std::memcpy(new_data, mem, h->size);
            system::deallocate(allocator, mem);
            return new_data;
        }
    }

    namespace dlmalloc {
        static u0 release(allocator_t* allocator) {
            destroy_mspace(g_system.default_allocator->subclass.heap);
        }

        static u0 deallocate(allocator_t* allocator, u0* mem) {
            auto h = header(mem);
            allocator->total_allocated -= h->size;
            mspace_free(allocator->subclass.heap, h);
        }

        static u0* allocate(allocator_t* allocator, u32 size, u32 align) {
            const u32 total_size = size_with_padding(size, align);
            auto h = (header_t*) mspace_malloc(allocator->subclass.heap, total_size);
            auto p = data_pointer(h, align);
            fill(h, p, total_size);
            allocator->total_allocated += total_size;
            return p;
        }

        static u0 init(allocator_t* allocator, allocator_config_t* config) {
            auto dl_config = (dl_config_t*) config;
            if (dl_config->base) {
                g_system.default_allocator->subclass.heap = create_mspace_with_base(
                    dl_config->base,
                    dl_config->heap_size,
                    0);
            } else {
                g_system.default_allocator->subclass.heap = create_mspace(
                    dl_config->heap_size,
                    0);
            }
        }

        static u0* reallocate(allocator_t* allocator, u0* mem, u32 new_size, u32 align) {
            auto h = header(mem);
            auto new_data = system::allocate(allocator, new_size, align);
            std::memcpy(new_data, mem, h->size);
            system::deallocate(allocator, mem);
            return new_data;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    allocator_system_t g_bump_system{
        .init       = bump::init,
        .type       = allocator_type_t::bump,
        .release    = bump::release,
        .allocate   = bump::allocate,
        .deallocate = bump::deallocate,
        .reallocate = bump::reallocate,
    };

    allocator_system_t g_default_system{
        .type       = allocator_type_t::system,
        .allocate   = system::allocate,
        .deallocate = system::deallocate,
        .reallocate = system::reallocate,
    };

    allocator_system_t g_dlmalloc_system{
        .init       = dlmalloc::init,
        .type       = allocator_type_t::dlmalloc,
        .release    = dlmalloc::release,
        .allocate   = dlmalloc::allocate,
        .deallocate = dlmalloc::deallocate,
        .reallocate = dlmalloc::reallocate,
    };

    ///////////////////////////////////////////////////////////////////////////

    u0 init_allocator(
            allocator_t* allocator,
            allocator_type_t type,
            allocator_config_t* config) {
        assert(allocator);
        switch (type) {
            case allocator_type_t::bump: {
                allocator->system = &g_bump_system;
                break;
            }
            case allocator_type_t::system: {
                allocator->system = &g_default_system;
                break;
            }
            case allocator_type_t::dlmalloc: {
                allocator->system = &g_dlmalloc_system;
                break;
            }
        }
        if (allocator->system->init)
            allocator->system->init(allocator, config);
    }

    u0 release_allocator(allocator_t* allocator) {
        assert(allocator && allocator->system);
        if (allocator->system->release)
            allocator->system->release(allocator);
        assert(allocator->total_allocated == 0);
    }

    u0 deallocate(allocator_t* allocator, u0* mem) {
        if (!mem) return;
        assert(allocator && allocator->system);
        allocator->system->deallocate(allocator, mem);
    }

    u0* allocate(allocator_t* allocator, u32 size, u32 align) {
        assert(allocator && allocator->system);
        return allocator->system->allocate(allocator, size, align);
    }

    u0* reallocate(allocator_t* allocator, u0* mem, u32 new_size, u32 align) {
        if (!mem) return nullptr;
        assert(allocator && allocator->system);
        return allocator->system->reallocate(allocator, mem, new_size, align);
    }

}