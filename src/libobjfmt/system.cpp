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

#include <basecode/core/error.h>
#include <basecode/core/string.h>
#include <basecode/core/config.h>
#include <basecode/objfmt/types.h>
#include <basecode/core/filesys.h>

namespace basecode::objfmt {
    struct system_t final {
        alloc_t*                alloc;
    };

    system_t                    g_objfmt_sys;

    namespace system {
        u0 fini() {
            container::fini();
        }

        status_t init(alloc_t* alloc) {
            g_objfmt_sys.alloc = alloc;
            container::init(g_objfmt_sys.alloc);
            auto objfmt_config_path = "../etc/objfmt.fe"_path;
            path_t config_path{};
            filesys::bin_rel_path(config_path, objfmt_config_path);
            defer({
                path::free(config_path);
                path::free(objfmt_config_path);
            });
            fe_Object* result{};
            auto status = config::eval(config_path, &result);
            if (!OK(status)) {
                return status_t::config_eval_error;
            }
            return status_t::ok;
        }
    }

    status_t read(container::type_t type, obj_file_t& file) {
        return status_t::read_error;
    }

    status_t write(container::type_t type, obj_file_t& file) {
        return status_t::write_error;
    }
}
