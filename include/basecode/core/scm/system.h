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

#include <basecode/core/scm/scm.h>
#include <basecode/core/scm/kernel.h>

namespace basecode::scm::system {
    enum class status_t : u32 {
        ok,
        error,
        bad_input,
    };

    u0 fini();

    ctx_t* global_ctx();

    const path_t* current_eval_path();

    status_t eval(const path_t& path, obj_t** obj);

    status_t eval(const u8* source, u32 len, obj_t** obj);

    status_t eval(const String_Concept auto& source, obj_t** obj) {
        return eval(source.data, source.length, obj);
    }

    status_t init(u32 heap_size, alloc_t* alloc = context::top()->alloc.main);
}
