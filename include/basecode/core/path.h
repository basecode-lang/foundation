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

#ifndef _WIN32
#   include <zconf.h>
#endif
#include <basecode/core/str.h>
#include <basecode/core/array.h>

namespace basecode {
    namespace path {
        u0 free(path_t& path);

        u0 reset(path_t& path);

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

        template <String_Concept S>
        status_t set(path_t& path, const S& value) {
            if (value.length > PATH_MAX)
                return status_t::path_too_long;
            str::reset(path.str);
            str::append(path.str, value);
            tokenize(path);
            return status_t::ok;
        }

        template <String_Concept S>
        status_t append(path_t& lhs, const S& rhs) {
            if (rhs.length == 0)
                return status_t::ok;
            if (rhs.length >= 2
            && (rhs[0] == '/' ||  rhs[1] == ':')) {
                return status_t::expected_relative_path;
            }
            if ((lhs.str.length + rhs.length + 2) > PATH_MAX)
                return status_t::path_too_long;
            const auto ch = lhs.str[lhs.str.length - 1];
            if (ch != '/' && ch != '\\')
                str::append(lhs.str, path_sep);
            str::append(lhs.str, rhs);
            tokenize(lhs);
            return status_t::ok;
        }

        status_t append(path_t& lhs, const path_t& rhs);

        s32 find_mark_index(const path_t& path, u8 type);

        status_t set(path_t& path, const path_t& new_path);

        s32 find_last_mark_index(const path_t& path, u8 type);

        template <String_Concept S>
        status_t replace_extension(path_t& path, const S& ext) {
            if (ext.length == 0)
                return status_t::unexpected_empty_extension;
            const auto ext_idx = find_mark_index(path,
                                                 path::marks::extension);
            b8 ext_has_dot = ext[0] == '.';
            if (ext_idx == -1) {
                if (!ext_has_dot)
                    str::append(path.str, ".");
                str::append(path.str, ext);
            } else {
                if (ext_idx + ext.length > PATH_MAX)
                    return status_t::path_too_long;
                if (ext_idx + ext.length > path.str.capacity)
                    str::reserve(path.str, ext_idx + ext.length);
                std::memcpy(path.str.data + ext_idx + 1,
                            ext.data + ext_has_dot,
                            ext.length - ext_has_dot);
                const auto diff = s32(ext.length - (path.str.length - ext_idx));
                path.str.length += diff;
            }
            tokenize(path);
            return status_t::ok;
        }

        status_t set(path_t& path, const s8* value, s32 len = -1);

        status_t parent_path(const path_t& path, path_t& new_path);

        status_t init(path_t& path,
                      alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept S>
        status_t init(path_t& path,
                      const S& value,
                      alloc_t* alloc = context::top()->alloc.main) {
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

