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

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif
#include <cassert>
#include <sys/mman.h>
#include <basecode/core/array.h>
#include <basecode/core/format.h>
#include <basecode/core/memory.h>
#include <basecode/core/memory/meta.h>
#include <basecode/core/memory/system/dl.h>
#include <basecode/core/memory/system/page.h>
#include <basecode/core/memory/system/bump.h>
#include <basecode/core/memory/system/slab.h>
#include <basecode/core/memory/system/proxy.h>
#include <basecode/core/memory/system/trace.h>
#include <basecode/core/memory/system/stack.h>
#include <basecode/core/memory/system/default.h>


namespace basecode::memory {
    struct system_t final {
        alloc_t                 default_alloc;
        alloc_t                 slab_alloc;
        array_t<alloc_t*>       allocators;
        usize                   os_page_size;
    };

    static str::slice_t         s_status_names[] = {
        "ok"_ss,
        "invalid allocator"_ss,
        "invalid default allocator"_ss,
        "invalid allocation system"_ss,
    };

    static str::slice_t         s_type_names[] = {
        "default"_ss,
        "bump"_ss,
        "page"_ss,
        "slab"_ss,
        "proxy"_ss,
        "trace"_ss,
        "stack"_ss,
        "dlmalloc"_ss,
    };

    thread_local system_t       t_system{};

    namespace system {
        u0 fini() {
            for (auto alloc : t_system.allocators) {
                memory::fini(alloc);
                memory::free(&t_system.slab_alloc, alloc);
            }
            array::free(t_system.allocators);
            memory::fini(&t_system.slab_alloc);
            meta::fini();
            memory::fini(&t_system.default_alloc);
        }

        usize os_page_size() {
            return t_system.os_page_size;
        }

        u0 print_allocators() {
            format::print("g_system.allocators.size = {}\n", t_system.allocators.size);
            for (auto alloc : t_system.allocators) {
                format::print("alloc = {}, alloc->system->type = {}\n", (u0*) alloc, type_name(alloc->system->type));
            }
        }

        alloc_t* default_alloc() {
            return &t_system.default_alloc;
        }

        b8 set_page_executable(u0* ptr, usize size) {
            const auto page_size = t_system.os_page_size;
            u64 start, end;
            start = (u64) ptr & ~(page_size - 1);
            end = (u64) ptr + size;
            end = (end + page_size - 1) & ~(page_size - 1);
            return mprotect(
                (u0*) start,
                end - start,
                PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
        }

        u0 free(alloc_t* alloc, b8 enforce, u32* freed_size) {
            if (!array::erase(t_system.allocators, alloc))
                return;
            u32 temp_freed{};
            memory::fini(alloc, enforce, &temp_freed);
            memory::free(&t_system.slab_alloc, alloc);
            if (freed_size) *freed_size = temp_freed;
        }

        alloc_t* make(alloc_type_t type, alloc_config_t* config) {
            auto alloc = (alloc_t*) memory::alloc(&t_system.slab_alloc);
            memory::init(alloc, type, config);
            array::append(t_system.allocators, alloc);
            return alloc;
        }

        status_t init(alloc_type_t type, u32 heap_size, u0* base) {
#ifdef _WIN32
            SYSTEM_INFO system_info;
            GetSystemInfo(&system_info);
            t_system.os_page_size = system_info.dwAllocationGranularity;
#else
            t_system.os_page_size = sysconf(_SC_PAGE_SIZE);
#endif
            switch (type) {
                case alloc_type_t::default_: {
                    auto status = memory::init(&t_system.default_alloc, alloc_type_t::default_);
                    if (!OK(status))
                        return status;
                    break;
                }
                case alloc_type_t::dlmalloc: {
                    dl_config_t dl_config{};
                    dl_config.base = base;
                    dl_config.heap_size = heap_size;
                    auto status = memory::init(&t_system.default_alloc, alloc_type_t::dlmalloc, &dl_config);
                    if (!OK(status))
                        return status;
                    break;
                }
                default: {
                    return status_t::invalid_default_allocator;
                }
            }

            meta::init(&t_system.default_alloc);
            array::init(t_system.allocators, &t_system.default_alloc);

            slab_config_t slab_config{};
            slab_config.backing = &t_system.default_alloc;
            slab_config.buf_size = sizeof(alloc_t);
            slab_config.buf_align = alignof(alloc_t);
            memory::init(&t_system.slab_alloc, alloc_type_t::slab, &slab_config);

            return status_t::ok;
        }
    }

    alloc_t* unwrap(alloc_t* alloc) {
        while (alloc->system->type == alloc_type_t::proxy)
            alloc = alloc->backing;
        return alloc;
    }

    str::slice_t type_name(alloc_type_t type) {
        return s_type_names[(u32) type];
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    u0* alloc(alloc_t* alloc, u32* alloc_size) {
        if (!alloc->system || !alloc->system->alloc) return {};
        u32 temp{};
        auto mem = alloc->system->alloc(alloc, 0, 0, temp);
        if (alloc_size) *alloc_size = temp;
        return mem;
    }

    u0 free(alloc_t* alloc, u0* mem, u32* freed_size) {
        if (!mem || !alloc->system || !alloc->system->free) return;
        u32 temp{};
        alloc->system->free(alloc, mem, temp);
        if (freed_size) *freed_size = temp;
    }

    u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        if (!alloc->system || !alloc->system->fini) return;
        u32 temp{};
        alloc->system->fini(alloc, enforce, &temp);
        meta::untrack(alloc);
        alloc->backing = {};
        if (freed_size) *freed_size = temp;
    }

    u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        if (!alloc->system || !alloc->system->realloc) return {};
        u32 old_size{};
        return alloc->system->realloc(alloc, mem, size, align, old_size);
    }

    u0* alloc(alloc_t* alloc, u32 size, u32 align, u32* alloc_size) {
        if (!size || !alloc->system || !alloc->system->alloc) return {};
        u32 temp{};
        auto mem = alloc->system->alloc(alloc, size, align, temp);
        if (alloc_size) *alloc_size = temp;
        return mem;
    }

    status_t init(alloc_t* alloc, alloc_type_t type, alloc_config_t* config) {
        if (!alloc)
            return status_t::invalid_allocator;
        switch (type) {
            case alloc_type_t::bump:        alloc->system = bump::system();     break;
            case alloc_type_t::slab:        alloc->system = slab::system();     break;
            case alloc_type_t::page:        alloc->system = page::system();     break;
            case alloc_type_t::proxy:       alloc->system = proxy::system();    break;
            case alloc_type_t::trace:       alloc->system = trace::system();    break;
            case alloc_type_t::stack:       alloc->system = stack::system();    break;
            case alloc_type_t::default_:    alloc->system = default_::system(); break;
            case alloc_type_t::dlmalloc:    alloc->system = dl::system();       break;
        }
        alloc->backing         = {};
        alloc->total_allocated = {};
        if (alloc->system->init)
            alloc->system->init(alloc, config);
        meta::track(alloc);
        return status_t::ok;
    }
}
