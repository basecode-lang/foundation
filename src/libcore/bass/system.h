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

#include <basecode/core/types.h>
#include <basecode/core/hashtable/hashtable.h>

namespace basecode {
    namespace kind {
        [[maybe_unused]] static constexpr u8 none       = 0b000;
        [[maybe_unused]] static constexpr u8 blob       = 0b001;
        [[maybe_unused]] static constexpr u8 field      = 0b010;
        [[maybe_unused]] static constexpr u8 header     = 0b100;

        string::slice_t name(u8);
    }

    namespace field {
        [[maybe_unused]] static constexpr u8 none       = 0b00000;
        [[maybe_unused]] static constexpr u8 id         = 0b00001;
    }

    struct field_t final {
        u32                     kind:3;
        u32                     type:5;
        u32                     value:24;
    };

    struct field_index_t final {
        u8*                     page;
        u16                     offset;
    };

    struct field_dict_t final {
        u32                     values[32];
    };

    using header_index_t = hashtable_t<u32, field_index_t>;

    struct bass_t final {
        alloc_t*                alloc;
        header_index_t          index;
        alloc_t*                page_alloc;
        alloc_t*                bump_alloc;
        u32                     id;
    };

    struct cursor_t final {
        u8*                     page;
        field_t*                field;
        field_t*                header;
        bass_t*                 storage;
        u32                     id;
        u16                     offset;
        u16                     end_offset;
        u16                     last_offset;
        u16                     start_offset;
        b8                      ok;
    };

    namespace bass {
        namespace dict {
            field_dict_t make(cursor_t& cursor);

            u32 get(const field_dict_t& dict, u8 key);

            u0 set(field_dict_t& dict, u8 key, u32 value);
        }

        u0 free(bass_t& storage);

        b8 next_header(cursor_t& cursor);

        u0 reset_cursor(cursor_t& cursor);

        cursor_t make_cursor(bass_t& storage);

        cursor_t first_header(bass_t& storage);

        b8 move_next(cursor_t& cursor, u8 type = 0);

        cursor_t get_header(bass_t& storage, u32 id);

        cursor_t make_cursor(const cursor_t& cursor);

        b8 write_field(cursor_t& cursor, u8 type, u32 value);

        b8 next_field(cursor_t& cursor, u32& value, u8 type = 0);

        cursor_t write_header(bass_t& storage, u8 type, u32 size = 0);

        u0 init(bass_t& storage, alloc_t* alloc = context::top()->alloc);
    }
}

