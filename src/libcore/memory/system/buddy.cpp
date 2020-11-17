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

#include <basecode/core/memory/system/buddy.h>

namespace basecode::memory::buddy {
    [[maybe_unused]] constexpr u32 max_levels    = 32;
    [[maybe_unused]] constexpr u32 leaf_size     = 16;

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        return nullptr;
    }

    alloc_system_t g_system{
        .init       = init,
        .fini       = fini,
        .free       = {},
        .alloc      = alloc,
        .realloc    = {},
        .type       = alloc_type_t::buddy,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}
