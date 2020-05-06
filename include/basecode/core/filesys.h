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

#include <basecode/core/path.h>

namespace basecode {
    struct glob_result_t final {
        alloc_t*            alloc;
        u32                 length;
    };
    static_assert(sizeof(glob_result_t) <= 16, "glob_result_t is now bigger than 16 bytes!");

    namespace filesys {
        enum class status_t : u8 {
            ok,
            not_dir,
            not_file,
            not_exists,
            invalid_dir,
            mkdir_failure,
            getcwd_failure,
            rename_failure,
            remove_failure,
            not_implemented,
            realpath_failure,
            cannot_modify_root,
            unexpected_empty_path,
            cannot_rename_to_existing_file,
        };

        namespace places {
            namespace user {
                status_t home(path_t& path);

                status_t data(path_t& path);

                status_t temp(path_t& path);

                status_t cache(path_t& path);

                status_t music(path_t& path);

                status_t config(path_t& path);

                status_t videos(path_t& path);

                status_t desktop(path_t& path);

                status_t runtime(path_t& path);

                status_t pictures(path_t& path);

                status_t programs(path_t& path);

                status_t documents(path_t& path);

                status_t downloads(path_t& path);

                status_t templates(path_t& path);

                status_t app_entries(path_t& path);

                status_t public_share(path_t& path);
            }

            namespace system {
                status_t programs(path_t& path);

                status_t cache(path_t& path, b8 local = true);

                status_t config(path_t& path, b8 local = true);

                status_t runtime(path_t& path, b8 local = true);

                status_t app_entries(path_t& path, b8 local = true);

                status_t mutable_data(path_t& path, b8 local = true);

                status_t immutable_data(path_t& path, b8 local = true);
            }
        }

        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc);

        status_t pwd(path_t& path);

        status_t mkdir(const path_t& path);

        status_t exists(const path_t& path);

        status_t is_dir(const path_t& path);

        status_t is_file(const path_t& path);

        str::slice_t status_name(status_t status);

        status_t mktmpdir(str::slice_t name, path_t& path);

        status_t file_size(const path_t& path, usize& size);

        status_t remove(const path_t& path, b8 recursive = {});

        status_t make_abs(const path_t& path, path_t& new_path);

        status_t rename(const path_t& old_filename, const path_t& new_filename);

        status_t glob(glob_result_t& r, str::slice_t pattern, u32 flags = {}, alloc_t* alloc = context::top()->alloc);
    }
}
