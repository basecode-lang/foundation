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

#include <basecode/core/str.h>
#include <basecode/core/str_array.h>

namespace basecode::str_array {
    u0 free(str_array_t& array) {
        memory::free(array.alloc, array.index);
        memory::free(array.alloc, array.buf.data);
        array.index    = {};
        array.size     = array.capacity     = {};
        array.buf.data = {};
        array.buf.size = array.buf.capacity = {};
    }

    u0 reset(str_array_t& array) {
        std::memset(array.index, 0, array.capacity * sizeof(str_idx_t));
        array.size     = {};
        array.buf.size = {};
    }

    b8 empty(const str_array_t& array) {
        return array.size == 0;
    }

    // XXX: need to implement
    u0 erase(str_array_t& array, u32 index) {
        UNUSED(array);
        UNUSED(index);
    }

    u0 init(str_array_t& array, alloc_t* alloc) {
        array.alloc    = alloc;
        array.index    = {};
        array.size     = array.capacity     = {};
        array.buf.data = {};
        array.buf.size = array.buf.capacity = {};
    }

    u0 grow_data(str_array_t& array, u32 new_capacity) {
        new_capacity = std::max(new_capacity, array.buf.capacity);
        reserve_data(array, new_capacity * 2 + 8);
    }

    u0 grow_index(str_array_t& array, u32 new_capacity) {
        new_capacity = std::max(new_capacity, array.capacity);
        reserve_index(array, new_capacity * 2 + 8);
    }

    u0 append(str_array_t& array, const s8* str, s32 len) {
        const auto length = len == - 1 ? strlen(str) + 1 : len + 1;
        if (array.size + 1 > array.capacity)                grow_index(array);
        if (array.buf.size + length > array.buf.capacity)   grow_data(array, length);
        auto& idx = array.index[array.size++];
        idx.offset = array.buf.size;
        idx.length = length - 1;
        std::memcpy(array.buf.data + idx.offset, str, length);
        array.buf.size += length;
    }

    u0 reserve_data(str_array_t& array, u32 new_capacity) {
        if (new_capacity == 0) {
            memory::free(array.alloc, array.buf.data);
            array.buf.data = {};
            array.buf.size = array.buf.capacity = {};
            return;
        }
        if (new_capacity == array.capacity)  return;
        new_capacity = std::max(array.buf.size, new_capacity);
        array.buf.data     = (u8*) memory::realloc(array.alloc, array.buf.data, new_capacity);
        array.buf.capacity = new_capacity;
    }

    u0 reserve_index(str_array_t& array, u32 new_capacity) {
        if (new_capacity == 0) {
            memory::free(array.alloc, array.index);
            array.index = {};
            array.size  = array.capacity = {};
            return;
        }
        if (new_capacity == array.capacity) return;
        new_capacity = std::max(array.size, new_capacity);
        array.index  = (str_idx_t*) memory::realloc(array.alloc, array.index, new_capacity * sizeof(str_idx_t), alignof(str_idx_t));
        const auto data          = array.index + array.size;
        const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
        std::memset(data, 0, size_to_clear * sizeof(str_idx_t));
        array.capacity = new_capacity;
    }
}
