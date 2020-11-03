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

#ifndef _WIN32
#   include <zconf.h>
#endif
#include <basecode/core/str.h>
#include <basecode/core/array.h>

namespace basecode {
    struct path_mark_t final {
        u16                     type:       4;
        u16                     value:      12;
    };

    using path_mark_list_t      = array_t<path_mark_t>;

    struct path_t final {
        str_t                   str;
        path_mark_list_t        marks;
    };
    static_assert(sizeof(path_t) <= 48, "path_t is now larger than 48 bytes!");

    namespace path {
        enum class status_t : u8 {
            ok                          = 0,
            path_too_long               = 159,
            no_parent_path              = 160,
            unexpected_empty_path       = 161,
            expected_relative_path      = 162,
            unexpected_empty_extension  = 163,
        };

        namespace marks {
            [[maybe_unused]] constexpr u16 none         = 0;
#ifdef _WIN32
            [[maybe_unused]] constexpr u16 drive_name   = 1;
#endif
            [[maybe_unused]] constexpr u16 root_part    = 2;
            [[maybe_unused]] constexpr u16 path_part    = 3;
            [[maybe_unused]] constexpr u16 extension    = 4;
            [[maybe_unused]] constexpr u16 parent_dir   = 5;
            [[maybe_unused]] constexpr u16 current_dir  = 6;
        }

        u0 free(path_t& path);

        u0 tokenize(path_t& path);

        b8 empty(const path_t& path);

        u16 length(const path_t& path);

        b8 absolute(const path_t& path);

        b8 is_only_root(const path_t& path);

        const s8* c_str(const path_t& path);

        b8 has_extension(const path_t& path);

        str::slice_t stem(const path_t& path);

        str::slice_t filename(const path_t& path);

        str::slice_t directory(const path_t& path);

        str::slice_t extension(const path_t& path);

#ifdef _WIN32
        str::slice_t drive_name(const path_t& path);
#endif

        status_t append(path_t& lhs, const path_t& rhs);

        s32 find_mark_index(const path_t& path, u8 type);

        status_t set(path_t& path, const path_t& new_path);

        s32 find_last_mark_index(const path_t& path, u8 type);

        status_t set(path_t& path, const s8* value, s32 len = -1);

        status_t parent_path(const path_t& path, path_t& new_path);

        status_t set(path_t& path, const String_Concept auto& value) {
            if (value.length > PATH_MAX)    return status_t::path_too_long;
            str::reset(path.str);
            str::append(path.str, value);
            tokenize(path);
            return status_t::ok;
        }

        status_t init(path_t& path, alloc_t* alloc = context::top()->alloc);

        status_t replace_extension(path_t& path, const String_Concept auto& ext) {
            if (ext.length == 0)    return status_t::unexpected_empty_extension;
            const auto ext_idx = find_mark_index(path, path::marks::extension);
            b8 ext_has_dot = ext[0] == '.';
            if (ext_idx == -1) {
                if (!ext_has_dot)   str::append(path.str, ".");
                str::append(path.str, ext);
            } else {
                if (ext_idx + ext.length > PATH_MAX)  return status_t::path_too_long;
                if (ext_idx + ext.length > path.str.capacity)
                    str::reserve(path.str, ext_idx + ext.length);
                std::memcpy(path.str.data + ext_idx + 1, ext.data + ext_has_dot, ext.length - ext_has_dot);
                const auto diff = s32(ext.length - (path.str.length - ext_idx));
                path.str.length += diff;
            }
            tokenize(path);
            return status_t::ok;
        }

        status_t init(path_t& path, const String_Concept auto& value, alloc_t* alloc = context::top()->alloc) {
            str::init(path.str, alloc);
            array::init(path.marks, alloc);
            return set(path, value);
        }
    }

    namespace slice {
        inline str::slice_t make(const path_t& path) {
            return slice::make(path.str);
        }
    }

    inline path_t operator "" _path(const s8* value) {
        path_t path{};
        path::init(path, slice::make(value));
        return path;
    }

    inline path_t operator "" _path(const s8* value, std::size_t length) {
        path_t path{};
        path::init(path, slice::make(value, length));
        return path;
    }
}

FORMAT_TYPE(basecode::path_t, format_to(ctx.out(), "{}", data.str));

