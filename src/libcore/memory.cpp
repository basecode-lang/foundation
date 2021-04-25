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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#ifdef _WIN32
#   ifndef _MSC_VER
#       include <sysinfoapi.h>
#   endif
#endif
#include <atomic>
#include <unistd.h>
#include <sys/mman.h>
#include <basecode/core/array.h>
#include <basecode/core/format.h>
#include <basecode/core/string.h>
#include <basecode/core/memory/meta.h>
#include <basecode/core/memory/system/dl.h>
#include <basecode/core/memory/system/page.h>
#include <basecode/core/memory/system/bump.h>
#include <basecode/core/memory/system/slab.h>
#include <basecode/core/memory/system/base.h>
#include <basecode/core/memory/system/proxy.h>
#include <basecode/core/memory/system/trace.h>
#include <basecode/core/memory/system/stack.h>
#include <basecode/core/memory/system/buddy.h>
#include <basecode/core/memory/system/scratch.h>

namespace basecode::memory {
    struct name_command_t;

    using alloc_array_t         = array_t<alloc_t*>;
    using name_command_array_t  = array_t<name_command_t>;

    struct name_command_t final {
        alloc_t*                alloc;
        s8*                     name;
        s32                     len;
    };

    struct system_t final {
        alloc_t                 temp;
        alloc_t                 main;
        alloc_t                 scratch;
        alloc_t                 slab_alloc;
        alloc_array_t           allocators;
        name_command_array_t    naming_queue;
        usize                   os_page_size;
        usize                   os_alloc_granularity;
        std::atomic<b8>         initialized;
    };

    static const s8*            s_status_names[] = {
        "ok",
        "invalid allocator",
        "invalid default allocator",
        "invalid allocation system",
    };

    static const s8*            s_type_names[] = {
        "none",
        "base",
        "bump",
        "page",
        "slab",
        "temp",
        "proxy",
        "trace",
        "stack",
        "buddy",
        "scratch",
        "dlmalloc",
    };

    thread_local system_t       t_system{};

    namespace system {
        u0 fini() {
            for (auto& cmd : t_system.naming_queue) {
                if (cmd.name)
                    memory::free(&t_system.main, cmd.name);
            }
            array::free(t_system.naming_queue);
            for (auto alloc : t_system.allocators) {
                memory::internal::fini(alloc);
                memory::internal::free(&t_system.slab_alloc, alloc);
            }
            array::free(t_system.allocators);
            memory::internal::fini(&t_system.slab_alloc);
            meta::system::fini();
            memory::internal::fini(&t_system.temp);
            memory::internal::fini(&t_system.scratch);
            memory::internal::fini(&t_system.main);
            t_system.naming_queue.alloc = {};
            t_system.initialized = false;
        }

        static u0 enqueue_naming_command(alloc_t* alloc,
                                         const s8* name,
                                         s32 len = -1);

        usize os_page_size() {
            return t_system.os_page_size;
        }

        alloc_t* temp_alloc() {
            return &t_system.temp;
        }

        alloc_t* main_alloc() {
            return &t_system.main;
        }

        u0 mark_initialized() {
            t_system.initialized = true;
            for (auto& cmd : t_system.naming_queue) {
                memory::set_name(cmd.alloc, cmd.name, cmd.len);
                memory::free(&t_system.main, cmd.name);
            }
            array::reset(t_system.naming_queue);
        }

        u0 print_allocators() {
            format::print("g_system.allocators.size = {}\n",
                          t_system.allocators.size);
            for (auto alloc : t_system.allocators) {
                format::print("alloc = {}, alloc->system->type = {}\n",
                              (u0*) alloc,
                              type_name(alloc->system->type));
            }
        }

        alloc_t* scratch_alloc() {
            return &t_system.scratch;
        }

        u32 free(alloc_t* alloc) {
            if (!array::erase(t_system.allocators, alloc))
                return {};
            const auto all_freed = memory::internal::fini(alloc);
            if (IS_PROXY(alloc))
                memory::proxy::remove(alloc);
            memory::internal::free(&t_system.slab_alloc, alloc);
            return all_freed;
        }

        usize os_alloc_granularity() {
            return t_system.os_alloc_granularity;
        }

        alloc_t* make(alloc_config_t* config) {
            BC_ASSERT_NOT_NULL(config);
            auto r = memory::internal::alloc(&t_system.slab_alloc, 0, 0);
            auto alloc = (alloc_t*) r.mem;
            memory::init(alloc, config);
            array::append(t_system.allocators, alloc);
            return alloc;
        }

        static u0 enqueue_naming_command(alloc_t* alloc,
                                         const s8* name,
                                         s32 len) {
            if (!alloc || !name)
                return;
            len = len == -1 ? strlen(name) : len;
            auto& cmd = array::append(t_system.naming_queue);
            cmd.alloc = alloc;
            cmd.name  = (s8*) memory::alloc(&t_system.main, len + 1);
            cmd.len   = len;
            std::strncpy(cmd.name, name, len);
            cmd.name[len] = '\0';
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

        status_t init(alloc_config_t* cfg) {
#ifdef _WIN32
            SYSTEM_INFO system_info;
            GetSystemInfo(&system_info);
            t_system.os_page_size         = system_info.dwPageSize;
            t_system.os_alloc_granularity = system_info.dwAllocationGranularity;
#else
            t_system.os_page_size         = sysconf(_SC_PAGE_SIZE);
            t_system.os_alloc_granularity = t_system.os_page_size;
#endif
            auto sys_cfg = (system_config_t*) cfg;
            BC_ASSERT_NOT_NULL(sys_cfg->main);
            memory::init(&t_system.main, sys_cfg->main);
            array::init(t_system.naming_queue, &t_system.main);
            enqueue_naming_command(&t_system.main, sys_cfg->main->name);
            meta::system::init(&t_system.main);
            meta::system::track(&t_system.main);

            if (sys_cfg->temp) {
                sys_cfg->temp->backing.alloc = &t_system.main;
                memory::init(&t_system.temp, sys_cfg->temp);
                meta::system::track(&t_system.temp);
            }

            if (sys_cfg->scratch) {
                sys_cfg->scratch->backing.alloc = &t_system.main;
                memory::init(&t_system.scratch, sys_cfg->scratch);
                meta::system::track(&t_system.scratch);
            }

            array::init(t_system.allocators, &t_system.main);

            slab_config_t slab_config{};
            slab_config.name          = "memory::alloc_slab";
            slab_config.buf_size      = sizeof(alloc_t);
            slab_config.buf_align     = alignof(alloc_t);
            slab_config.num_pages     = DEFAULT_NUM_PAGES;
            slab_config.backing.alloc = &t_system.main;
            memory::init(&t_system.slab_alloc, &slab_config);

            return status_t::ok;
        }
    }

    namespace internal {
        u32 fini(alloc_t* alloc) {
            BC_ASSERT_NOT_NULL(alloc);
            auto sys = alloc->system;
            if (!sys || !sys->fini)
                return 0;
            const auto size_freed = sys->fini(alloc);
            if (size_freed > alloc->total_allocated) {
                format::print(stderr,
                              "fini of {} allocator freed {} "
                              "bytes vs {} in total_allocated!",
                              type_name(alloc->system->type),
                              size_freed,
                              alloc->total_allocated);
            } else {
                alloc->total_allocated -= size_freed;
                BC_ASSERT_MSG(alloc->total_allocated == 0,
                              "allocator is leaking memory: {}",
                              alloc->total_allocated);
            }
            meta::system::untrack(alloc);
            alloc->backing = {};
            return size_freed;
        }

        u32 size(alloc_t* alloc, u0* mem) {
            BC_ASSERT_NOT_NULL(alloc && mem);
            auto sys = alloc->system;
            if (!sys || !sys->size)
                return 0;
            return sys->size(alloc, mem);
        }

        u32 free(alloc_t* alloc, u0* mem) {
            if (!alloc)
                return 0;
            auto sys = alloc->system;
            if (!mem || !sys || !sys->free)
                return 0;
            const auto size_freed = sys->free(alloc, mem);
            alloc->total_allocated -= size_freed;
            return size_freed;
        }

        mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
            BC_ASSERT_NOT_NULL(alloc);
            auto sys = alloc->system;
            if (!sys || !sys->alloc)
                return {};
            const auto r = sys->alloc(alloc, size, align);
            alloc->total_allocated += r.size;
            return r;
        }

        mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
            BC_ASSERT_NOT_NULL(alloc);
            auto sys = alloc->system;
            if (!sys || !sys->realloc)
                return {};
            const auto r = sys->realloc(alloc, mem, size, align);
            alloc->total_allocated += r.size;
            return r;
        }
    }

    const s8* name(alloc_t* alloc) {
        if (alloc->name == 0)
            return "(none)";
        auto rc = string::interned::get(alloc->name);
        return OK(rc.status) ? (const s8*) rc.slice.data : "(none)";
    }

    alloc_t* unwrap(alloc_t* alloc) {
        while (alloc && IS_PROXY(alloc))
            alloc = alloc->backing;
        return alloc;
    }

    const s8* type_name(alloc_type_t type) {
        return s_type_names[(u32) type];
    }

    const s8* status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    u0 set_name(alloc_t* alloc, const s8* name, s32 len) {
        if (!t_system.initialized) {
            if (t_system.naming_queue.alloc)
                system::enqueue_naming_command(alloc, name, len);
            return;
        }
        auto rc = string::interned::fold_for_result(name, len);
        if (!OK(rc.status))
            return;
        alloc->name = rc.id;
    }

    status_t init(alloc_t* alloc, alloc_config_t* config) {
        BC_ASSERT_NOT_NULL(alloc);
        BC_ASSERT_NOT_NULL(config);
        switch (config->type) {
            case alloc_type_t::none:        break;
            case alloc_type_t::bump:        alloc->system = bump::system();     break;
            case alloc_type_t::base:        alloc->system = base::system();     break;
            case alloc_type_t::slab:        alloc->system = slab::system();     break;
            case alloc_type_t::temp:        break;
            case alloc_type_t::page:        alloc->system = page::system();     break;
            case alloc_type_t::proxy:       alloc->system = proxy::system();    break;
            case alloc_type_t::trace:       alloc->system = trace::system();    break;
            case alloc_type_t::stack:       alloc->system = stack::system();    break;
            case alloc_type_t::buddy:       alloc->system = buddy::system();    break;
            case alloc_type_t::scratch:     alloc->system = scratch::system();  break;
            case alloc_type_t::dlmalloc:    alloc->system = dl::system();       break;
        }
        BC_ASSERT_NOT_NULL(alloc->system);
        alloc->backing         = {};
        alloc->total_allocated = {};
        if (alloc->system->init)
            alloc->system->init(alloc, config);
        if (config->name)
            memory::set_name(alloc, config->name);
        meta::system::track(alloc);
        return status_t::ok;
    }
}
