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

#include <basecode/core/memory.h>

namespace basecode {
    struct str_t;

    struct str_idx_t final {
        u64                         status: 2;
        u64                         offset: 31;
        u64                         length: 31;
    };

    struct str_array_t final {
        alloc_t*                    alloc;
        str_idx_t*                  index;
        struct {
            u8*                     data;
            u32                     size;
            u32                     capacity;
        }                           buf;
        u32                         size;
        u32                         capacity;

        str::slice_t operator[](u32 i) const {
            const auto& idx = index[i];
            return slice::make(buf.data + idx.offset, idx.length);
        }
    };
    static_assert(sizeof(str_array_t) <= 40, "str_array_t is now greater than 40 bytes!");

    namespace str_array {
        u0 free(str_array_t& array);

        u0 reset(str_array_t& array);

        b8 empty(const str_array_t& array);

        u0 erase(str_array_t& array, u32 index);

        u0 reserve_data(str_array_t& array, u32 new_capacity);

        u0 reserve_index(str_array_t& array, u32 new_capacity);

        u0 grow_data(str_array_t& array, u32 new_capacity = 0);

        u0 grow_index(str_array_t& array, u32 new_capacity = 0);

        u0 append(str_array_t& array, const s8* str, s32 len = -1);

        u0 append(str_array_t& array, const String_Concept auto& str) {
            if (array.size + 1 > array.capacity)                        grow_index(array);
            if (array.buf.size + (str.length + 1) > array.buf.capacity) grow_data(array, str.length + 1);
            auto& idx = array.index[array.size++];
            idx.offset = array.buf.size;
            idx.length = str.length;
            std::memcpy(array.buf.data + idx.offset, str.data, str.length);
            array.buf.size += str.length;
            array.buf.data[array.buf.size++] = '\0';
        }

        u0 init(str_array_t& array, alloc_t* alloc = context::top()->alloc);
    }
}

FORMAT_TYPE(basecode::str_array_t,
    for (basecode::u32 i = 0; i < data.size; ++i) {
        if (i > 0) format_to(ctx.out(), ",");
        format_to(ctx.out(), "{}", data[i]);
    });

