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

#include <basecode/core/memory.h>

namespace basecode {
    struct proxy_config_t : alloc_config_t {
        proxy_config_t() : alloc_config_t(alloc_type_t::proxy) {}

        b8                      owner;
    };

    namespace memory::proxy {
        enum class status_t : u8 {
            ok,
        };

        u0 fini();

        u0 reset();

        u0 free(alloc_t* alloc);

        alloc_system_t* system();

        b8 remove(alloc_t* alloc);

        const array_t<alloc_t*>& active();

        status_t init(alloc_t* alloc = context::top()->alloc.main);

        alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner = false);
    }
}
