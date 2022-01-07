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
    struct slab_t final {
        u0*                     page;
        u0*                     free_list;
        slab_t*                 prev;
        slab_t*                 next;
        u32                     buf_count;
        u0*                     pad[3];
    };
    static_assert(sizeof(slab_t) <= 64, "slab_t is now larger than 64 bytes!");

    constexpr u32 slab_size     = sizeof(slab_t);

    struct slab_config_t : alloc_config_t {
        slab_config_t() : alloc_config_t(alloc_type_t::slab) {}

        u32                     buf_size;
        u8                      buf_align;
        u8                      num_pages = 1;
    };

    namespace memory::slab {
        u0 reset(alloc_t* alloc);

        alloc_system_t* system();
    }
}

