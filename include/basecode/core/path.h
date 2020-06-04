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
    struct path_t final {
        str_t                   str;
        array_t<u16>            marks;
        u16                     ext_mark:   10;
        u16                     is_abs:     1;
        u16                     is_root:    1;
        u16                     pad:        4;
    };
    static_assert(sizeof(path_t) <= 56, "path_t is now larger than 56 bytes!");

    namespace path {
        enum class status_t : u8 {
            ok,
            path_too_long,
            no_parent_path,
            unexpected_empty_path,
            expected_relative_path,
            unexpected_empty_extension,
        };

        u0 free(path_t& path);

        u0 tokenize(path_t& path);

        b8 empty(const path_t& path);

        u16 length(const path_t& path);

        const s8* c_str(const path_t& path);

        str::slice_t stem(const path_t& path);

        str::slice_t status_name(status_t status);

        str::slice_t filename(const path_t& path);

        str::slice_t directory(const path_t& path);

        str::slice_t extension(const path_t& path);

        status_t set(path_t& path, const s8* value);

        status_t append(path_t& lhs, const path_t& rhs);

        status_t parent_path(const path_t& path, path_t& new_path);

        status_t set(path_t& path, const String_Concept auto& value) {
            if (value.length > PATH_MAX)    return status_t::path_too_long;
            str::reset(path.str);
            str::append(path.str, value);
            tokenize(path);
            return status_t::ok;
        }

        status_t replace_extension(path_t& path, const String_Concept auto& ext) {
            if (ext.length == 0)    return status_t::unexpected_empty_extension;
            b8 ext_has_dot = ext[0] == '.';
            if (!path.ext_mark) {
                if (!ext_has_dot)   str::append(path.str, ".");
                str::append(path.str, ext);
            } else {
                if (path.ext_mark + ext.length > PATH_MAX)  return status_t::path_too_long;
                if (path.ext_mark + ext.length > path.str.capacity)
                    str::reserve(path.str, path.ext_mark + ext.length);
                std::memcpy(path.str.data + path.ext_mark + 1, ext.data + ext_has_dot, ext.length - ext_has_dot);
                const auto diff = s32(ext.length - (path.str.length - path.ext_mark));
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

