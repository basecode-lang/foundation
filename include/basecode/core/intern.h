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

#include <basecode/core/str.h>
#include <basecode/core/array.h>

namespace basecode::intern {
    u0 free(intern_t& pool);

    u0 reset(intern_t& pool);

    u0 init(intern_t& pool,
            alloc_t* alloc = context::top()->alloc.main,
            f32 load_factor = .75f);

    b8 remove(intern_t& pool, intern_id id);

    u0 reserve(intern_t& pool, u32 capacity);

    result_t get(intern_t& pool, intern_id id);

    str::slice_t* get_slice(intern_t& pool, intern_id id);

    intern_t make(alloc_t* alloc = context::top()->alloc.main);

    result_t fold(intern_t& pool, const s8* data, s32 len = -1);

    result_t fold(intern_t& pool, const String_Concept auto& value) {
        return fold(pool, (const s8*) value.data, value.length);
    }
}

