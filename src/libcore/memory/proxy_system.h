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

#include "memory.h"
#include "../array.h"
#include "../hashtable.h"

namespace basecode {
    struct proxy_config_t : alloc_config_t {
        alloc_t*                backing;
    };

    namespace memory::proxy {
        using proxy_list_t      = array_t<alloc_t*>;
        using proxy_map_t       = hashtable_t<u32, alloc_t*>;

        enum class status_t : u8 {
            ok,
        };

        u0 shutdown();

        proxy_list_t active();

        u32 id(alloc_t* alloc);

        alloc_system_t* system();

        u0 reset(b8 enforce = true);

        u0 id(alloc_t* alloc, u32 id);

        string::slice_t name(alloc_t* alloc);

        u0 free(alloc_t* proxy, b8 enforce = true);

        u0 name(alloc_t* alloc, string::slice_t name);

        // XXX: proxy allocators + dynamically created allocators
        //      via system::make create a unique situation whereby
        //      the "owner" of the backing allocator isn't clear.
        //
        //      the calling code which creates the proxy and underlying
        //      dynamic allocator need to ensure that the backing allocator
        //      is properly free'd via system::free *before* system::shutdown
        //      otherwise bad things happen.
        alloc_t* make(alloc_t* backing, string::slice_t name);

        status_t initialize(alloc_t* alloc = context::top()->alloc);
    }
}
