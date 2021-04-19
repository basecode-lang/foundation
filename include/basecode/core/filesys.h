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

#include <glob.h>
#include <unistd.h>
#include <basecode/core/path.h>

namespace basecode {
    struct glob_result_t final {
        glob_t                  buf;
        array_t<str_t>          paths;
    };
    static_assert(sizeof(glob_result_t) <= 112, "glob_result_t is now bigger than 112 bytes!");

    namespace filesys {
        enum class status_t : u8 {
            ok                              = 0,
            not_dir                         = 116,
            not_file                        = 117,
            not_exists                      = 118,
            invalid_dir                     = 119,
            chdir_failure                   = 120,
            file_writable                   = 121,
            mkdir_failure                   = 122,
            getcwd_failure                  = 123,
            rename_failure                  = 124,
            remove_failure                  = 125,
            not_equivalent                  = 126,
            mkdtemp_failure                 = 127,
            not_implemented                 = 128,
            unexpected_path                 = 129,
            realpath_failure                = 130,
            cannot_modify_root              = 131,
            unexpected_empty_path           = 132,
            cannot_rename_to_existing_file  = 133,
        };

        namespace glob {
            u0 free(glob_result_t& r);

            u0 reset(glob_result_t& r);

            inline u32 size(const glob_result_t& r) {
                return r.paths.size;
            }

            u0 init(glob_result_t& r, alloc_t* alloc = context::top()->alloc);

            status_t find(glob_result_t& r, str::slice_t pattern, u32 flags = {});
        }

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

        status_t pwd(path_t& path);

        status_t cwd(const path_t& path);

        status_t exists(const path_t& path);

        status_t is_dir(const path_t& path);

        status_t is_file(const path_t& path);

        status_t is_read_only(const path_t& path);

        status_t mktmpdir(str::slice_t name, path_t& path);

        status_t rm(const path_t& path, b8 recursive = {});

        status_t file_size(const path_t& path, usize& size);

        status_t mkabs(const path_t& path, path_t& new_path);

        status_t mkdir(const path_t& path, b8 recursive = {});

        status_t init(alloc_t* alloc = context::top()->alloc);

        status_t equivalent(const path_t& path1, const path_t& path2);

        status_t bin_rel_path(path_t& abs_path, const path_t& rel_path);

        status_t mv(const path_t& old_filename, const path_t& new_filename);
    }
}
