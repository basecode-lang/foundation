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

#include <basecode/core/path.h>

namespace basecode::path {
    static str::slice_t s_status_names[] = {
        "ok"_ss,
        "path too long"_ss,
        "no parent path"_ss,
        "unexpected empty path"_ss,
        "expected relative path"_ss,
        "expected empty extension"_ss,
    };

    u0 tokenize(path_t& path) {
        array::reset(path.marks);
        path.ext_mark = path.is_abs = path.is_root = {};
        if (path.str.length == 0) return;
        u16 last_ext_mark{};
        for (u16 i    = 0; i < path.str.length; ++i) {
            if (path.str[i] == '/') {
                array::append(path.marks, i);
            } else if (path.str[i] == '.') {
                last_ext_mark = i;
            }
        }
        if (array::empty(path.marks) || last_ext_mark > *array::back(path.marks)) {
            path.ext_mark = last_ext_mark;
        }
        path.is_abs   = path.str[0] == '/';
        path.is_root  = path.is_abs && path.str.length == 1;
    }

    u0 free(path_t& path) {
        str::free(path.str);
        array::free(path.marks);
        path.ext_mark = path.is_root = path.is_abs = {};
    }

    b8 empty(const path_t& path) {
        return path.str.length == 0;
    }

    u16 length(const path_t& path) {
        return path.str.length;
    }

    str::slice_t stem(const path_t& path) {
        if (empty(path)) return {};
        const auto last_idx = path.marks.size > 0 ? (*array::back(path.marks)) + 1 : 0;
        return str::slice_t{.data = path.str.data + last_idx, .length = u32(path.ext_mark ? path.ext_mark - last_idx : path.str.length)};
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    str::slice_t filename(const path_t& path) {
        if (empty(path)) return {};
        const auto last_idx = path.marks.size > 0 ? (*array::back(path.marks)) + 1 : 0;
        return str::slice_t{.data = path.str.data + last_idx, .length = u32(path.str.length - last_idx)};
    }

    str::slice_t directory(const path_t& path) {
        if (empty(path)) return {};
        const auto last_idx = path.marks.size > 0 ? *array::back(path.marks) : 0;
        return str::slice_t{.data = path.str.data, .length = u32(last_idx)};
    }

    str::slice_t extension(const path_t& path) {
        if (empty(path) || !path.ext_mark) return {};
        return str::slice_t{.data = path.str.data + path.ext_mark, .length = u32(path.str.length - path.ext_mark)};
    }

    status_t set(path_t& path, const s8* value) {
        return set(path, str::slice_t{.data = (const u8*) value, .length = u32(strlen(value))});
    }

    status_t append(path_t& lhs, const path_t& rhs) {
        if (empty(rhs))                                         return status_t::ok;
        if (rhs.is_abs)                                         return status_t::expected_relative_path;
        if ((lhs.str.length + rhs.str.length + 2) > PATH_MAX)   return status_t::path_too_long;
        if (lhs.str[lhs.str.length - 1] != '/') {
            str::append(lhs.str, '/');
        }
        str::append(lhs.str, rhs.str);
        tokenize(lhs);
        return status_t::ok;
    }

    status_t parent_path(const path_t& path, path_t& new_path) {
        if (empty(path))                                        return status_t::unexpected_empty_path;
        if (path.str.length == 1 || array::empty(path.marks))   return status_t::no_parent_path;
        const u16 len = std::max<u16>(*array::back(path.marks), 1);
        if (&path != &new_path) {
            str::reset(new_path.str);
            str::append(new_path.str, path.str.data, len);
        } else {
            str::resize(new_path.str, len);
        }
        tokenize(new_path);
        return status_t::ok;
    }
}
