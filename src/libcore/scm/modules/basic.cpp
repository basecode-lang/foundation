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

#include <basecode/core/filesys.h>
#include <basecode/core/obj_pool.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/basic.h>

namespace basecode::scm::module::basic {
    struct system_t final {
        scm::ctx_t*             ctx;
        alloc_t*                alloc;
        obj_pool_t              storage;
    };

    system_t                    g_basic_sys;

    b8 adjust_load_path(path_t& path, str::slice_t file_name);

    obj_t* intern_get(u32 id) {
        auto s = string::interned::get_slice(id);
        return s ? make_string(g_basic_sys.ctx, *s) : nil(g_basic_sys.ctx);
    }

    obj_t* load(rest_array_t* rest) {
        obj_t* obj{};
        path_t load_path{};
        defer(path::free(load_path));
        for (auto path : *rest) {
            check_type(g_basic_sys.ctx, path, obj_type_t::string);
            const auto& file_name = *string::interned::get_slice(STRING_ID(path));
            if (!adjust_load_path(load_path, file_name)) {
                // XXX:
                error(g_basic_sys.ctx,
                      "load cannot resolve file path: {}",
                      file_name);
            }
            auto status = scm::system::eval(load_path, &obj);
            if (!OK(status)) {
                // XXX:
                error(g_basic_sys.ctx, "load eval failed");
            }
        }
        return obj;
    }

    b8 adjust_load_path(path_t& path, str::slice_t file_name) {
        path_t tmp_path{};
        path::init(tmp_path, file_name, g_basic_sys.alloc);
        defer(path::free(tmp_path));
        if (!path::absolute(tmp_path)) {
            auto eval_path = scm::system::current_eval_path();
            if (eval_path) {
                path::init(path, eval_path->str);
                path::parent_path(path, path);
                path::append(path, tmp_path);
                if (!OK(filesys::mkabs(path, path)))
                    return false;
            } else {
                if (!OK(filesys::bin_rel_path(path, tmp_path)))
                    return false;
            }
            if (!OK(filesys::exists(path)))
                return false;
        } else {
            path::init(path, file_name, g_basic_sys.alloc);
        }
        return true;
    }

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            static proc_export_t s_exports[] = {
                {"load"_ss, 1,
                    {
                        {(u0*) load, "load"_ss, type_decl::obj_ptr, 1,
                            {
                                {"rest"_ss, type_decl::obj_ptr, .is_rest = true},
                            }
                        }
                    }
                },

                {"intern/get"_ss, 1,
                    {
                        {(u0*) intern_get, "intern_get"_ss, type_decl::obj_ptr, 1,
                            {
                                {"id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {str::slice_t{}},
            };
        }

        u0 fini() {
            obj_pool::free(g_basic_sys.storage);
        }

        status_t init(scm::ctx_t* ctx, alloc_t* alloc) {
            g_basic_sys.ctx   = ctx;
            g_basic_sys.alloc = alloc;
            obj_pool::init(g_basic_sys.storage, g_basic_sys.alloc);
            kernel::create_exports(g_basic_sys.ctx, exports::s_exports);
            return status_t::ok;
        }
    }
}
