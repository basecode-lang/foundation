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

#include <basecode/core/context.h>

namespace basecode {
    inline str::slice_t str_array_t::operator[](u32 i) const {
        const auto& idx = index[i];
        return slice::make(idx.buf, idx.len);
    }

    namespace str_array {
        u0 free(str_array_t& array);

        u0 reset(str_array_t& array);

        b8 empty(const str_array_t& array);

        u0 reserve(str_array_t& array, u32 new_capacity);

        u0 grow(str_array_t& array, u32 new_capacity = 8);

        u0 init(str_array_t& array,
                alloc_t* alloc = context::top()->alloc.main);

        u0 append(str_array_t& array, const s8* str, s32 len = -1);

        u0 append(str_array_t& array, const String_Concept auto& str) {
            append(array, (const s8*) str.data, str.length);
        }
    }
}
