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

#include <unistd.h>
#include <sys/stat.h>
#include <basecode/core/filesys.h>

namespace basecode::filesys {
    struct system_t final {
        alloc_t*                alloc;
    };

    static str::slice_t s_status_names[] = {
        "ok"_ss,
        "not dir"_ss,
        "not file"_ss,
        "not exists"_ss,
        "invalid dir"_ss,
        "mkdir failure"_ss,
        "rename failure"_ss,
        "remove failure"_ss,
        "not implemented"_ss,
        "realpath failure"_ss,
        "cannot modify root"_ss,
        "unexpected empty path"_ss,
        "cannot rename to existing file"_ss,
    };

    thread_local system_t       t_filesys_system = {};

    u0 fini() {
    }

    status_t pwd(path_t& path) {
        s8 temp[PATH_MAX];
        if (getcwd(temp, PATH_MAX)) {
            path::set(path, temp);
            return status_t::ok;
        }
        return status_t::getcwd_failure;
    }

    status_t init(alloc_t* alloc) {
        // XXX: we should read the XDG user-dirs.dirs file and put it into
        //      an efficient memory structure here so we don't have to continually
        //      open/read/parse/close this file.
        //
        //      additionally, we should grab all of the env vars we need up front
        //      and put the results into a symtab_t
        //
        t_filesys_system.alloc = alloc;
        return status_t::ok;
    }

    status_t mkdir(const path_t& path) {
        if (path::empty(path))    return status_t::unexpected_empty_path;
        if (path.is_root)   return status_t::cannot_modify_root;
        return ::mkdir(str::c_str(const_cast<str_t&>(path.str)), S_IRWXU) == 0 ? status_t::ok : status_t::mkdir_failure;
    }

    status_t exists(const path_t& path) {
        if (path::empty(path)) return status_t::unexpected_empty_path;
        struct stat sb;
        return stat(str::c_str(const_cast<str_t&>(path.str)), &sb) == 0 ? status_t::ok : status_t::not_exists;
    }

    status_t is_dir(const path_t& path) {
        if (path::empty(path)) return status_t::unexpected_empty_path;
        struct stat sb;
        if (stat(str::c_str(const_cast<str_t&>(path.str)), &sb))
            return status_t::not_exists;
        return S_ISDIR(sb.st_mode) ? status_t::ok : status_t::not_dir;
    }

    status_t is_file(const path_t& path) {
        if (path::empty(path)) return status_t::unexpected_empty_path;
        struct stat sb;
        if (stat(str::c_str(const_cast<str_t&>(path.str)), &sb))
            return status_t::not_exists;
        return S_ISREG(sb.st_mode) ? status_t::ok : status_t::not_file;
    }

    status_t places::user::home(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::data(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    status_t places::user::temp(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::cache(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::music(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::config(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::videos(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::desktop(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::runtime(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::pictures(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::programs(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::documents(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::downloads(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::templates(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::system::programs(path_t& path) {
        auto status = path::set(path, "/opt"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::user::app_entries(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t places::user::public_share(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t remove(const path_t& path, b8 recursive) {
        if (path::empty(path))    return status_t::unexpected_empty_path;
        if (path.is_root)   return status_t::cannot_modify_root;
        if (recursive) {
            return status_t::not_implemented;
        } else {
            return std::remove(str::c_str(const_cast<str_t&>(path.str))) == 0 ? status_t::ok : status_t::remove_failure;
        }
    }

    status_t mktmpdir(str::slice_t name, path_t& path) {
        UNUSED(name); UNUSED(path);
        return status_t::not_implemented;
    }

    status_t file_size(const path_t& path, usize& size) {
        if (path::empty(path)) return status_t::unexpected_empty_path;
        struct stat sb;
        if (stat(str::c_str(const_cast<str_t&>(path.str)), &sb))
            return status_t::not_exists;
        size = sb.st_size;
        return status_t::ok;
    }

    status_t places::system::cache(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/cache"_ss : "/var/cache"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::config(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/usr/local/etc"_ss : "/etc"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t make_abs(const path_t& path, path_t& new_path) {
        if (path::empty(path)) return status_t::unexpected_empty_path;
        s8 temp[PATH_MAX];
        if (!realpath(str::c_str(const_cast<str_t&>(path.str)), temp))
            return status_t::realpath_failure;
        path::set(new_path, temp);
        return status_t::ok;
    }

    status_t places::system::runtime(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/run"_ss : "/var/run"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::app_entries(path_t& path, b8 local) {
        UNUSED(path); UNUSED(local);
        return status_t::not_implemented;
    }

    status_t places::system::mutable_data(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/lib"_ss : "/var/lib"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::immutable_data(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/usr/local/share"_ss : "/usr/share"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t rename(const path_t& old_filename, const path_t& new_filename) {
        if (path::empty(old_filename) || path::empty(new_filename))
            return status_t::unexpected_empty_path;
        if (old_filename.is_root)
            return status_t::cannot_modify_root;
        if (!OK(exists(old_filename)))
            return status_t::not_exists;
        if (OK(exists(new_filename)))
            return status_t::cannot_rename_to_existing_file;
        return std::rename(str::c_str(const_cast<str_t&>(old_filename.str)), str::c_str(const_cast<str_t&>(new_filename.str))) == 0 ? status_t::ok : status_t::rename_failure;
    }

    status_t glob(glob_result_t& r, str::slice_t pattern, u32 flags, alloc_t* alloc) {
        UNUSED(r); UNUSED(pattern); UNUSED(flags); UNUSED(alloc);
        return status_t::not_implemented;
    }
}
