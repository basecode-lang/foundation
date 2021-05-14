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

#ifdef _WIN32
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#   include <shlobj.h>
#   include <KnownFolders.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <basecode/core/env.h>
#include <basecode/core/buf.h>
#include <basecode/core/utf.h>
#include <basecode/core/filesys.h>

namespace basecode::filesys {
    struct system_t final {
        alloc_t*                    alloc;
        env_t*                      xdg_env;
        env_t*                      root_env;
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
            if ((flags & GLOB_APPEND) == 0)
                reset(r);
            str_t temp{};
            str::init(temp, t_file_sys.alloc);
            defer(str::free(temp));
            str::reserve(temp, pattern.length);
            str::append(temp, pattern);
            auto rc = ::glob(str::c_str(temp), flags, nullptr, &r.buf);
            if (rc == 0) {
                for (u32 i = 0; i < r.buf.gl_pathc; ++i)
                    array::append(r.paths, (str_t) r.buf.gl_pathv[i]);
                return status_t::ok;
            }
            return status_t::glob_no_match;
        }
    }

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
        t_file_sys.alloc = alloc;
        t_file_sys.xdg_env  = {};
        t_file_sys.root_env = env::system::get_root();

        path_t config_path{};
        path::init(config_path, t_file_sys.alloc);
        defer(path::free(config_path));
        if (!OK(places::user::config(config_path)))
            return status_t::invalid_user_place;
        path::append(config_path, "user-dirs.dirs"_ss);
        if (OK(exists(config_path))) {
            t_file_sys.xdg_env = env::system::make("xdg"_ss,
                                                   t_file_sys.root_env);
            auto status = env::load(t_file_sys.xdg_env, config_path);
            if (!OK(status))
                return status_t::xdg_user_dirs_invalid;
        }
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
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Profile,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto home = env::get(t_file_sys.root_env, "HOME"_ss);
        if (!home)
            return status_t::invalid_user_place;
        path::set(path, home->kind.str);
#endif
        return status_t::ok;
    }

    status_t places::user::data(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_DATA_HOME", path, ".local/share"_ss);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t is_read_only(const path_t& path) {
        return ::access(path::c_str(path), W_OK) == 0 ? status_t::ok :
               status_t::file_writable;
    }

    status_t places::user::temp(path_t& path) {
#ifdef _WIN32
        wchar_t buf[MAX_PATH + 1];
        DWORD count = GetTempPathW(MAX_PATH + 1, buf);
        if (count == 0)
            return status_t::mkdtemp_failure;
        auto utf8_str = utf::utf16_to_utf8(buf);
        defer(utf::free(utf8_str));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto tmp_dir = env::get(t_file_sys.root_env, "TMPDIR"_ss);
        if (!tmp_dir) {
            path::set(path, "/tmp"_ss);
        } else {
            path::set(path, temp_dir->kind.str);
        }
#endif
        return status_t::ok;
    }

    status_t places::user::cache(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_CACHE_HOME"_ss, path, ".cache"_ss);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::music(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Music,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_MUSIC_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::config(path_t& path) {
#ifdef _WIN32
        auto status = places::user::home(path);
        if (!OK(status))
            return status;
        path::append(path, ".config"_ss);
#else
        auto status = get_xdg_path("XDG_CONFIG_HOME"_ss, path, ".config");
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::videos(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Videos,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_CACHE_HOME"_ss, path, ".cache");
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::desktop(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Desktop,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_DESKTOP_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::runtime(path_t& path) {
#ifdef _WIN32
        return places::user::temp(path);
#else
        auto status = get_xdg_path("XDG_RUNTIME_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::pictures(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_PICTURES_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::programs(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramFiles,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        // /usr/local, /opt/local?
#endif
        return status_t::ok;
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
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_DOCUMENTS_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::downloads(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_DOWNLOADS_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::user::templates(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Templates,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = get_xdg_path("XDG_TEMPLATES_DIR"_ss, path);
        if (!OK(status))
            return status_t::invalid_user_place;
#endif
        return status_t::ok;
    }

    status_t places::system::programs(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramFiles,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = path::set(path, "/opt"_ss);
        if (!OK(status))
            return status_t::invalid_dir;
#endif
        return status_t::ok;
    }

    status_t places::user::app_entries(path_t& path) {
#ifdef _WIN32
        PWSTR known_path{};
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_StartMenu,
                                          0,
                                          NULL,
                                          &known_path);
        if (!SUCCEEDED(hr))
            return status_t::invalid_user_place;
        auto utf8_str = utf::utf16_to_utf8(known_path);
        defer(utf::free(utf8_str);
              CoTaskMemFree(known_path));
        path::set(path, utf::c_str(utf8_str), utf::length(utf8_str));
#else
        auto status = places::user::data_dir(path);
        if (!OK(status))
            return status_t::invalid_user_place;
        path::append(path, "applications"_ss);
#endif
        return status_t::ok;
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

    status_t places::user::get_xdg_path(str::slice_t name,
                                        path_t& path,
                                        str::slice_t default_path) {
        auto value = env::get(t_file_sys.xdg_env, name);
        if (value) {
            str_t expanded{};
            str::init(expanded, t_file_sys.alloc);
            auto status = env::expand(t_file_sys.xdg_env, value, expanded);
            if (!OK(status))
                return status_t::invalid_user_place;
            path::set(path, expanded);
        } else {
            if (!OK(places::user::home(path)))
                return status_t::invalid_user_place;
            if (!slice::empty(default_path))
                path::append(path, default_path);
        }
        return status_t::ok;
    }

    status_t places::user::get_xdg_path_list(str::slice_t name,
                                             path_array_t& paths,
                                             const slice_array_t& default_list) {
        auto value = env::get(t_file_sys.xdg_env, name);
        for (const auto& field : value ? value->kind.list : default_list) {
            auto& path = array::append(paths);
            path::init(path, field, paths.alloc);
        }
        return status_t::ok;
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
