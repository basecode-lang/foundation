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

#include <basecode/core/slice.h>
#include <basecode/core/context.h>
#include <basecode/core/hash/murmur.h>

namespace basecode::locale {
    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    str::slice_t locale();

    locale_key_t make_key(u32 id);

    locale_key_t make_key(u32 id, str::slice_t locale);
}

