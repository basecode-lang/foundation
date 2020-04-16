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

#include <basecode/core/string/str.h>
#include <basecode/core/array/array.h>
#include "system.h"

namespace basecode::memory::proxy {
    using proxy_list_t = array_t<alloc_t*>;

    enum class status_t : u8 {
        ok,
    };

    u0 shutdown();

    u0 reset(b8 enforce = true);

    const proxy_list_t& active();

    u0 free(alloc_t* proxy, b8 enforce = true);

    alloc_t* make(alloc_t* backing, string::slice_t name);

    status_t initialize(alloc_t* alloc = context::top()->alloc);
}

