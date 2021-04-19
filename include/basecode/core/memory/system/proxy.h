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

#pragma once

#include <basecode/core/array.h>
#include <basecode/core/symtab.h>
#include <basecode/core/memory.h>

namespace basecode {
    struct proxy_config_t : alloc_config_t {
        alloc_t*                backing;
        b8                      owner;
    };

    struct proxy_pair_t final {
        alloc_t*                alloc;
        u32                     name_id;
        u32                     pair_id;
    };

    namespace memory::proxy {
        using proxy_array_t     = array_t<proxy_pair_t*>;
        using proxy_symtab_t    = symtab_t<proxy_pair_t*>;

        enum class status_t : u8 {
            ok,
        };

        u0 fini();

        u0 reset();

        u0 free(alloc_t* proxy);

        alloc_system_t* system();

        b8 remove(alloc_t* proxy);

        u0 active(proxy_array_t& list);

        alloc_t* find(str::slice_t name);

        str::slice_t name(alloc_t* alloc);

        status_t init(alloc_t* alloc = context::top()->alloc);

        alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner = false);
    }
}

FORMAT_TYPE(basecode::proxy_pair_t,
            format_to(ctx.out(),
                      "[alloc: {}, name_id: {}, pair_id: {}]",
                      (basecode::u0*) data.alloc,
                      data.name_id,
                      data.pair_id));
