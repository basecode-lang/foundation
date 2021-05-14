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

#include <basecode/core/str.h>
#include <basecode/core/intern.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/memory/system/trace.h>

namespace basecode::memory::trace {
    enum class alloc_event_type_t : u8 {
        init,
        fini,
    };

    enum class memory_event_type_t : u8 {
        free,
        alloc,
        realloc
    };

    struct alloc_event_t final {
        // XXX:
        str::slice_t                call_site;
        u64                         ticks;
        alloc_event_type_t          type;
    };

    struct memory_event_t final {
        str::slice_t                call_site;
        u64                         ticks;
        u32                         size;
        memory_event_type_t         type;
    };

    using alloc_event_map_t         = hashtab_t<u0*, alloc_event_t>;
    using memory_event_map_t        = hashtab_t<u0*, memory_event_t>;

    struct trace_system_t final {
        intern_t                    intern;
        alloc_event_map_t           alloc_events;
        memory_event_map_t          memory_events;
    };

    static u32 fini(alloc_t* alloc) {
        UNUSED(alloc);
        return 0;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        UNUSED(mem);
        return 0;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        UNUSED(mem);
        return 0;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        UNUSED(alloc);
        UNUSED(config);
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        UNUSED(alloc);
        UNUSED(size);
        UNUSED(align);
        return {};
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        UNUSED(alloc);
        UNUSED(mem);
        UNUSED(size);
        UNUSED(align);
        return {};
    }

    alloc_system_t g_system{
        .size       = size,
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = realloc,
        .type       = alloc_type_t::trace,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    alloc_t* make(alloc_t* backing, str::slice_t name) {
        UNUSED(backing);
        UNUSED(name);
        return nullptr;
    }
}
