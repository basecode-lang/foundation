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

#include <basecode/core/array.h>
#include <basecode/core/symtab.h>
#include <basecode/core/memory.h>

namespace basecode {
    struct proxy_config_t : alloc_config_t {
        alloc_t*                backing;
        b8                      owner;
    };

    constexpr u32 max_proxy_name_len = 55;
    struct proxy_pair_t final {
        alloc_t*                alloc;
        u32                     id;
        u8                      name[max_proxy_name_len];
        u8                      length;
    };

    namespace memory::proxy {
        using proxy_map_t   = symtab_t<proxy_pair_t*>;
        using proxy_list_t  = array_t<proxy_pair_t*>;

        enum class status_t : u8 {
            ok,
        };

        u0 fini();

        proxy_list_t active();

        alloc_system_t* system();

        u0 reset(b8 enforce = true);

        str::slice_t name(alloc_t* alloc);

        u0 free(alloc_t* proxy, b8 enforce = true);

        u0 name(alloc_t* alloc, str::slice_t name);

        status_t init(alloc_t* alloc = context::top()->alloc);

        alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner = false);
    }
}
