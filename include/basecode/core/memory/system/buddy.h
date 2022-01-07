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
    struct buddy_block_t final {
        buddy_block_t*          next;
        buddy_block_t*          prev;
    };

    struct buddy_config_t : alloc_config_t {
        buddy_config_t() : alloc_config_t(alloc_type_t::buddy) {}

        u0*                     metadata;
        u32                     heap_size;
    };

    namespace memory::buddy {
        alloc_system_t* system();

        u32 available(alloc_t* alloc);

        u32 largest_available(alloc_t* alloc);
    }
}
