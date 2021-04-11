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

    u32 length(obj_t* obj) {
        return scm::length(g_basic_sys.ctx, obj);
    }

    obj_t* intern_get(u32 id) {
        auto s = string::interned::get_slice(id);
        return s ? make_string(g_basic_sys.ctx, *s) : nil(g_basic_sys.ctx);
    }

    obj_t* reverse(obj_t* lst) {
        auto ctx = g_basic_sys.ctx;
        obj_t* res;
        for (res = ctx->nil; !IS_NIL(lst); lst = CDR(lst))
            res = cons(ctx, CAR(lst), res);
        return res;
    }

    obj_t* current_environment() {
        return g_basic_sys.ctx->env;
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

    obj_t* append(rest_array_t* rest) {
        auto ctx = g_basic_sys.ctx;
        obj_t* head = ctx->nil;
        obj_t* tail = head;
        for (auto lst : *rest) {
            if (IS_NIL(lst))
                continue;
            if (is_list(ctx, lst)) {
                while (!IS_NIL(lst)) {
                    auto r = CONS1(CAR(lst));
                    if (IS_NIL(tail)) {
                        head = r;
                        tail = head;
                    } else {
                        SET_CDR(tail, r);
                        tail = r;
                    }
                    lst = CDR(lst);
                }
            } else {
                auto r = CONS1(lst);
                if (IS_NIL(tail)) {
                    head = r;
                    tail = head;
                } else {
                    SET_CDR(tail, r);
                    tail = r;
                }
            }
        }
        return head;
    }

    obj_t* parent_environment(obj_t* env) {
        auto ctx = g_basic_sys.ctx;
        check_type(ctx, env, obj_type_t::environment);
        auto e = ENV(env);
        return e->parent;
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

                {"append"_ss, 1,
                    {
                        {(u0*) append, "append"_ss, type_decl::obj_ptr, 1,
                            {
                                {"rest"_ss, type_decl::obj_ptr, .is_rest = true},
                            }
                        }
                    }
                },

                {"length"_ss, 1,
                    {
                        {(u0*) length, "length"_ss, type_decl::u32_, 1,
                            {
                                {"obj"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },

                {"reverse"_ss, 1,
                    {
                        {(u0*) reverse, "reverse"_ss, type_decl::obj_ptr, 1,
                            {
                                {"lst"_ss, type_decl::list_ptr},
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

                {"current-environment"_ss, 1,
                    {
                        {(u0*) current_environment, "current_environment"_ss, type_decl::obj_ptr, 0}
                    }
                },

                {"parent-environment"_ss, 1,
                    {
                        {(u0*) parent_environment, "parent_environment"_ss, type_decl::obj_ptr, 1,
                            {
                                {"env"_ss, type_decl::obj_ptr},
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
