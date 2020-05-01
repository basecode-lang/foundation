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

#include <basecode/core/str.h>
#include <basecode/core/intern.h>
#include <basecode/core/hashtable.h>
#include "trace_system.h"

namespace basecode::memory::trace {
    enum class alloc_event_type_t : u8 {
        init,
        release,
    };

    enum class memory_event_type_t : u8 {
        free,
        alloc,
        realloc
    };

    struct alloc_event_t final {
        // XXX:
        string::slice_t             call_site;
        u64                         ticks;
        alloc_event_type_t          type;
    };

    struct memory_event_t final {
        string::slice_t             call_site;
        u64                         ticks;
        u32                         size;
        memory_event_type_t         type;
    };

    using alloc_event_map_t         = hashtable_t<u0*, alloc_event_t>;
    using memory_event_map_t        = hashtable_t<u0*, memory_event_t>;

    struct trace_system_t final {
        intern_t                    intern;
        alloc_event_map_t           alloc_events;
        memory_event_map_t          memory_events;
    };

    static u0 release(alloc_t* alloc) {
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        freed_size = 0;
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        allocated_size = 0;
        return {};
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size) {
        old_size = new_size = {};
        return {};
    }

    alloc_system_t g_system{
        .init       = init,
        .type       = alloc_type_t::trace,
        .free       = free,
        .alloc      = alloc,
        .release    = release,
        .realloc    = realloc,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    alloc_t* make(alloc_t* backing, string::slice_t name) {
        return nullptr;
    }
}
