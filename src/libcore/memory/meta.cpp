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

#include <basecode/core/memory/meta.h>

// XXX: rework
namespace basecode::memory::meta {
    struct system_t final {
        b8                      init{};
    };

    thread_local system_t       t_meta_system{};

    u0 fini() {
        t_meta_system.init = {};
    }

    u0 init(alloc_t* alloc) {
        t_meta_system.init = true;
        track(alloc);
    }

    u0 track(alloc_t* alloc) {
        if (!t_meta_system.init) return;
        UNUSED(alloc);
    }

    u0 untrack(alloc_t* alloc) {
        if (!t_meta_system.init) return;
        UNUSED(alloc);
    }
}

