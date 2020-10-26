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
#include <basecode/core/string.h>

namespace basecode::path {
#ifdef _WIN32
    constexpr s8 path_sep = '\\';
#else
    constexpr s8 path_sep = '/';
#endif

    u0 tokenize(path_t& path) {
        array::reset(path.marks);
        if (path.str.length == 0) return;
        b8 first_path_part = true;
        for (u32 i = 0; i < path.str.length; ++i) {
            const auto ch = path.str[i];
#ifdef _WIN32
            if (isalpha(ch)) {
                if (i + 1 < path.str.length
                && path.str[i + 1] == ':') {
                    path_mark_t mark{};
                    mark.type = path::marks::drive_name;
                    mark.value = i + 1;
                    array::append(path.marks, mark);
                    ++i;
                }
            } else
#endif
            if (ch == '/' || ch == '\\') {
                path_mark_t mark{};
                if (first_path_part) {
                    mark.type = path::marks::root_part;
                } else {
                    mark.type = path::marks::path_part;
                }
                mark.value = i;
                array::append(path.marks, mark);
            } else if (path.str[i] == '.') {
                path_mark_t mark{};
                if (i + 1 < path.str.length) {
                    switch (path.str[i + 1]) {
                        case '.':
                            mark.type = path::marks::parent_dir;
                            mark.value = i + 1;
                            ++i;
                            break;
                        case '/':
                            mark.type = path::marks::current_dir;
                            mark.value = i;
                            break;
                        default:
                            mark.type = path::marks::extension;
                            mark.value = i;
                            break;
                    }
                }
                array::append(path.marks, mark);
            }
            if (first_path_part) {
                first_path_part = false;
            }
        }
    }

    u0 free(path_t& path) {
        str::free(path.str);
        array::free(path.marks);
    }

    b8 empty(const path_t& path) {
        return path.str.length == 0;
    }

    u16 length(const path_t& path) {
        return path.str.length;
    }

    b8 absolute(const path_t& path) {
        auto abs = find_mark_index(path, path::marks::root_part) != -1;
#ifdef _WIN32
        abs = abs || find_mark_index(path, path::marks::drive_name) != -1;
#endif
        return abs;
    }

    b8 is_only_root(const path_t& path) {
#ifdef _WIN32
        return path.marks.size == 1
            && (path.marks[0].type == path::marks::drive_name || path.marks[0].type == path::marks::root_part);
#else
        return path.marks.size == 1 && path.marks[0].type == path::marks::root_part;
#endif
    }

    const s8* c_str(const path_t& path) {
        return str::c_str(const_cast<str_t&>(path.str));
    }

    b8 has_extension(const path_t& path) {
        return find_mark_index(path, path::marks::extension) != -1;
    }

    str::slice_t stem(const path_t& path) {
        if (empty(path)) return {};
        const auto ext_idx = find_mark_index(path, path::marks::extension);
        auto last_path_idx = find_last_mark_index(path, path::marks::path_part);
        if (last_path_idx == -1)
            last_path_idx = 0;
        else
            ++last_path_idx;
        auto temp = slice::make(path.str.data + last_path_idx,
                                u32(ext_idx != -1 ? ext_idx - last_path_idx : path.str.length));
        return temp;
    }

    str::slice_t filename(const path_t& path) {
        if (empty(path)) return {};
        auto last_path_idx = find_last_mark_index(path, path::marks::path_part);
        if (last_path_idx == -1)
            last_path_idx = 0;
        else
            ++last_path_idx;
        auto temp = slice::make(path.str.data + last_path_idx, u32(path.str.length - last_path_idx));
        return temp;
    }

    str::slice_t directory(const path_t& path) {
        if (empty(path)) return {};
        const auto last_path_idx = find_last_mark_index(path, path::marks::path_part);
        if (last_path_idx == -1)
            return {};
        return str::slice_t{.data = path.str.data, .length = u32(last_path_idx)};
    }

    str::slice_t extension(const path_t& path) {
        if (empty(path))    return {};
        auto ext_idx = find_mark_index(path, path::marks::extension);
        if (ext_idx == -1)  return {};
        return str::slice_t{.data = path.str.data + ext_idx, .length = u32(path.str.length - ext_idx)};
    }

    status_t set(path_t& path, const s8* value) {
        return set(path, str::slice_t{.data = (const u8*) value, .length = u32(strlen(value))});
    }
    status_t init(path_t& path, alloc_t* alloc) {
        str::init(path.str, alloc);
        array::init(path.marks, alloc);
        return status_t::ok;
    }
#ifdef _WIN32

    str::slice_t drive_name(const path_t& path) {
        if (empty(path))            return {};
        auto drive_name_idx = find_mark_index(path, path::marks::drive_name);
        if (drive_name_idx == -1)
            return {};
        return slice::make(path.str.data, drive_name_idx);
    }

#endif

    status_t append(path_t& lhs, const path_t& rhs) {
        if (empty(rhs))                                         return status_t::ok;
        if (absolute(rhs))                                      return status_t::expected_relative_path;
        if ((lhs.str.length + rhs.str.length + 2) > PATH_MAX)   return status_t::path_too_long;
        const auto ch = lhs.str[lhs.str.length - 1];
        if (ch != '/' && ch != '\\') {
            str::append(lhs.str, path_sep);
        }
        str::append(lhs.str, rhs.str);
        tokenize(lhs);
        return status_t::ok;
    }

    s32 find_mark_index(const path_t& path, u8 type) {
        for (const auto& mark : path.marks) {
            if (mark.type == type)
                return mark.value;
        }
        return -1;
    }

    s32 find_last_mark_index(const path_t& path, u8 type) {
        for (s32 i = path.marks.size - 1; i >= 0; --i) {
            if (path.marks[i].type == type)
                return path.marks[i].value;
        }
        return -1;
    }

    status_t parent_path(const path_t& path, path_t& new_path) {
        if (empty(path))                                        return status_t::unexpected_empty_path;
        if (path.str.length == 1 || array::empty(path.marks))   return status_t::no_parent_path;
        auto len = find_last_mark_index(path, path::marks::path_part);
        if (len == -1)
            return status_t::no_parent_path;
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
