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

namespace basecode::gap_buf {
    u0 free(gap_buf_t& buf);

    u8 caret_curr(gap_buf_t& buf);

    u8 caret_next(gap_buf_t& buf);

    u8 caret_prev(gap_buf_t& buf);

    u0 print(const gap_buf_t& buf);

    u0 gap_to_caret(gap_buf_t& buf);

    u32 caret_offset(gap_buf_t& buf);

    u32 gap_size(const gap_buf_t& buf);

    u0 caret_put(gap_buf_t& buf, u8 value);

    u0 caret_insert(gap_buf_t& buf, u8 value);

    u0 caret_move(gap_buf_t& buf, u32 offset);

    u0 caret_delete(gap_buf_t& buf, u32 size);

    u0 grow(gap_buf_t& buf, u32 new_capacity);

    u0 caret_replace(gap_buf_t& buf, u8 value);

    u0 gap_expand(gap_buf_t& buf, u32 new_size);

    u0 reserve(gap_buf_t& buf, u32 new_capacity);

    status_t init(gap_buf_t& buf,
                  u32 gap_size = 16,
                  alloc_t* alloc = context::top()->alloc.main);

    u0 caret_insert(gap_buf_t& buf, const String_Concept auto& value) {
        gap_to_caret(buf);
        auto length = value.length;
        if (length > gap_size(buf))
            gap_expand(buf, length);
        u32 i{};
        while (i < length)
            caret_put(buf, value[i++]);
    }
}
