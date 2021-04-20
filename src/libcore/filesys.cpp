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

#include <sys/stat.h>
#include <sys/types.h>
#include <basecode/core/symtab.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>

namespace basecode::filesys {
    struct system_t final {
        alloc_t*                    alloc;
        symtab_t<str_t>             env;
    };

    thread_local system_t           t_file_sys = {};

    namespace glob {
        u0 free(glob_result_t& r) {
            ::globfree(&r.buf);
            for (auto& s : r.paths)
                str::free(s);
            array::free(r.paths);
        }

        u0 reset(glob_result_t& r) {
            for (auto& s : r.paths)
                str::free(s);
            array::reset(r.paths);
        }

        u0 init(glob_result_t& r, alloc_t* alloc) {
            array::init(r.paths, alloc);
            ZERO_MEM(&r.buf, glob_t);
        }

        status_t find(glob_result_t& r, str::slice_t pattern, u32 flags) {
            if ((flags & GLOB_APPEND) == 0) reset(r);
            WITH_SLICE_AS_CSTR(pattern,
                               ::glob(pattern, flags, nullptr, &r.buf));
            for (u32 i = r.buf.gl_offs; i < r.buf.gl_pathc; ++i)
                array::append(r.paths, (str_t) r.buf.gl_pathv[i]);
            return status_t::ok;
        }
    }

    u0 fini() {
        symtab::free(t_file_sys.env);
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
        //      an efficient memory structure here so we don't have to
        //      continually open/read/parse/close this file.
        //
        //      additionally, we should grab all of the env vars we need up
        //      front and put the results into a symtab_t
        //
        t_file_sys.alloc = alloc;
        symtab::init(t_file_sys.env, t_file_sys.alloc);
        return status_t::ok;
    }

    status_t cwd(const path_t& path) {
        return ::chdir(path::c_str(path)) == 0 ? status_t::ok :
               status_t::chdir_failure;
    }

    status_t exists(const path_t& path) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        struct stat sb{};
        const auto path_str = path::c_str(path);
        auto rc = stat(path_str, &sb);
        return rc == 0 ? status_t::ok : status_t::not_exists;
    }

    status_t is_dir(const path_t& path) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        struct stat sb{};
        if (stat(path::c_str(path), &sb))
            return status_t::not_exists;
        return S_ISDIR(sb.st_mode) ? status_t::ok : status_t::not_dir;
    }

    status_t is_file(const path_t& path) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        struct stat sb{};
        if (stat(path::c_str(path), &sb))
            return status_t::not_exists;
        return S_ISREG(sb.st_mode) ? status_t::ok : status_t::not_file;
    }

    status_t places::user::home(path_t& path) {
#ifdef _WIN32
        auto home_drive = getenv("HOMEDRIVE");
        auto home_path  = getenv("HOMEPATH");
        if (!home_drive || !home_path)
            return status_t::no_home_path;
        path::set(path, format::format("{}{}", home_drive, home_path));
#else
        auto home = getenv("HOME");
        path::set(path, home);
#endif
        return status_t::ok;
    }

    status_t places::user::data(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t is_read_only(const path_t& path) {
        return ::access(path::c_str(path), W_OK) == 0 ? status_t::ok :
               status_t::file_writable;
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
        auto status = places::user::home(path);
        if (!OK(status))
            return status;
        auto config_path = ".config"_path;
        path::append(path, config_path);
        path::free(config_path);
        return status_t::ok;
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

    status_t rm(const path_t& path, b8 recursive) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        if (path::is_only_root(path))
            return status_t::cannot_modify_root;
        if (recursive) {
            return status_t::not_implemented;
        } else {
            return std::remove(path::c_str(path)) == 0 ? status_t::ok :
                   status_t::remove_failure;
        }
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

    status_t mkdir(const path_t& path, b8 recursive) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        if (path::is_only_root(path))
            return status_t::cannot_modify_root;
        if (!recursive) {
            s32 rc;
#if _WIN32
            rc = ::mkdir(path::c_str(path));
#else
            rc = ::mkdir(path::c_str(path), S_IRWXU);
#endif
            return rc == 0 ? status_t::ok : status_t::mkdir_failure;
        }

        s8 temp[PATH_MAX];
        std::memcpy(temp, path.str.data, path.str.length);
        temp[path.str.length] = '\0';

        for (auto p = temp + 1; *p; ++p) {
            if (*p == '/') {
                *p = '\0';
                s32 rc;
#if _WIN32
                rc = ::mkdir(temp);
#else
                rc = ::mkdir(temp, S_IRWXU);
#endif
                if (rc != 0) {
                    if (errno != EEXIST)
                        return status_t::mkdir_failure;
                }
                *p = '/';
            }
        }

        s32 rc;
#if _WIN32
        rc = ::mkdir(temp);
#else
        rc = ::mkdir(temp, S_IRWXU);
#endif
        if (rc != 0) {
            if (errno != EEXIST)
                return status_t::mkdir_failure;
        }

        return status_t::ok;
    }

    status_t places::user::public_share(path_t& path) {
        UNUSED(path);
        return status_t::not_implemented;
    }

    status_t mktmpdir(str::slice_t name, path_t& path) {
        if (!path::empty(path))
            return status_t::unexpected_path;
        s8 temp[PATH_MAX];
        std::memcpy(temp, name.data, name.length);
        std::memset(temp + name.length, 'X', 6);
        temp[name.length + 6] = '\0';
#ifndef _WIN32
        auto temp_str = ::mkdtemp(temp);
        if (!temp_str) return status_t::mkdtemp_failure;
        str::append(path.str, temp_str);
#else
        // XXX: FIXME
#endif
        return status_t::ok;
    }

    status_t file_size(const path_t& path, usize& size) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        struct stat sb{};
        if (stat(path::c_str(path), &sb))
            return status_t::not_exists;
        size = sb.st_size;
        return status_t::ok;
    }

    status_t mkabs(const path_t& path, path_t& new_path) {
        if (path::empty(path))
            return status_t::unexpected_empty_path;
        s8 temp[PATH_MAX];
#if _WIN32
        if (!_fullpath(temp, path::c_str(path), PATH_MAX))
            return status_t::realpath_failure;
#else
        if (!realpath(path::c_str(path), temp))
            return status_t::realpath_failure;
#endif
        path::set(new_path, temp);
        return status_t::ok;
    }

    status_t places::system::cache(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/cache"_ss :
                                      "/var/cache"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::config(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/usr/local/etc"_ss :
                                      "/etc"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::runtime(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/run"_ss :
                                      "/var/run"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t places::system::app_entries(path_t& path, b8 local) {
        UNUSED(path); UNUSED(local);
        return status_t::not_implemented;
    }

    status_t places::system::mutable_data(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/var/local/lib"_ss :
                                      "/var/lib"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t equivalent(const path_t& path1, const path_t& path2) {
        if (path::empty(path1) || path::empty(path2))
            return status_t::unexpected_empty_path;
        struct stat sb1{};
        struct stat sb2{};
        if (stat(path::c_str(path1), &sb1))
            return status_t::not_exists;
        if (stat(path::c_str(path2), &sb2))
            return status_t::not_exists;
        return sb1.st_dev == sb2.st_dev && sb1.st_ino == sb2.st_ino ?
               status_t::ok : status_t::not_equivalent;
    }

    status_t places::system::immutable_data(path_t& path, b8 local) {
        auto status = path::set(path, local ? "/usr/local/share"_ss :
                                      "/usr/share"_ss);
        return OK(status) ? status_t::ok : status_t::invalid_dir;
    }

    status_t bin_rel_path(path_t& abs_path, const path_t& rel_path) {
        path::init(abs_path, slice::make(context::top()->argv[0]));
        path::parent_path(abs_path, abs_path);
        path::append(abs_path, rel_path);
        return filesys::mkabs(abs_path, abs_path);
    }

    status_t mv(const path_t& old_filename, const path_t& new_filename) {
        if (path::empty(old_filename) || path::empty(new_filename))
            return status_t::unexpected_empty_path;
        if (path::is_only_root(old_filename))
            return status_t::cannot_modify_root;
        if (!OK(exists(old_filename)))
            return status_t::not_exists;
        if (OK(exists(new_filename)))
            return status_t::cannot_rename_to_existing_file;
        return std::rename(path::c_str(old_filename),
                           path::c_str(new_filename)) == 0 ? status_t::ok :
                           status_t::rename_failure;
    }
}
