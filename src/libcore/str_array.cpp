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

#include <basecode/core/str_array.h>
#include <basecode/core/memory/system/page.h>
#include <basecode/core/memory/system/bump.h>

namespace basecode::str_array {
    u0 free(str_array_t& array) {
        memory::system::free(array.bump_alloc);
        memory::system::free(array.page_alloc);
        memory::free(array.alloc, array.index);
        array.size     = {};
        array.index    = {};
        array.capacity = {};
    }

    u0 reset(str_array_t& array) {
        memory::bump::reset(array.bump_alloc);
        memory::page::reset(array.page_alloc);
        std::memset(array.index, 0, array.capacity * sizeof(str_idx_t));
        array.size = {};
    }

    b8 empty(const str_array_t& array) {
        return array.size == 0;
    }

    u0 init(str_array_t& array, alloc_t* alloc) {
        array.size     = {};
        array.alloc    = alloc;
        array.index    = {};
        array.capacity = {};

        page_config_t page_config{};
        page_config.name          = "strtab::page_alloc";
        page_config.num_pages     = DEFAULT_NUM_PAGES;
        page_config.backing.alloc = array.alloc;
        array.page_alloc          = memory::system::make(&page_config);

        bump_config_t bump_config{};
        bump_config.name          = "strtab::bump_alloc";
        bump_config.type          = bump_type_t::allocator;
        bump_config.backing.alloc = array.page_alloc;
        array.bump_alloc          = memory::system::make(&bump_config);
    }

    u0 grow(str_array_t& array, u32 new_capacity) {
        new_capacity = std::max(new_capacity, array.capacity);
        reserve(array, new_capacity * 2 + 8);
    }

    u0 reserve(str_array_t& array, u32 new_capacity) {
        if (new_capacity == 0) {
            memory::free(array.alloc, array.index);
            array.size     = {};
            array.index    = {};
            array.capacity = {};
            return;
        }
        if (new_capacity == array.capacity)
            return;
        new_capacity = std::max(array.size, new_capacity);
        array.index  = (str_idx_t*) memory::realloc(
            array.alloc,
            array.index,
            new_capacity * sizeof(str_idx_t),
            alignof(str_idx_t));
        const auto data          = array.index + array.size;
        const auto size_to_clear = new_capacity > array.capacity ?
                                   new_capacity - array.capacity : 0;
        std::memset(data, 0, size_to_clear * sizeof(str_idx_t));
        array.capacity = new_capacity;
    }

    u0 append(str_array_t& array, const s8* str, s32 len) {
        const auto length = len == - 1 ? strlen(str) : len;
        if (array.size + 1 > array.capacity)
            grow(array);
        auto& idx = array.index[array.size++];
        idx.buf = (u8*) memory::alloc(array.bump_alloc, length + 1);
        idx.len = length;
        std::memcpy(idx.buf, str, length);
        idx.buf[length] = '\0';
    }
}
