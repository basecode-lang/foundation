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

#include <basecode/core/obj_pool.h>
#include <basecode/core/scm/modules/cxx.h>

namespace basecode::scm::module::cxx {
    namespace cxx_core = basecode::cxx;

    struct revision_map_t final {
        const s8*               name;
        cxx_core::revision_t    rev;
    };

    struct integral_size_map_t final {
        const s8*               name;
        cxx_core::integral_size_t size;
    };

    struct system_t final {
        alloc_t*                alloc;
        scm::ctx_t*             ctx;
        obj_pool_t              storage;
        basecode::obj_type_t*   program_type;
    };

    system_t                    g_cxx_sys;

    static revision_map_t s_revisions[] = {
        {"c99",     cxx_core::revision_t::c99},
        {"c11",     cxx_core::revision_t::c11},
        {"cpp17",   cxx_core::revision_t::cpp17},
        {"cpp20",   cxx_core::revision_t::cpp20},
        {nullptr, cxx_core::revision_t::c11},
    };

    static integral_size_map_t s_sizes[] = {
        {"zero", cxx_core::integral_size_t::zero},
        {"byte", cxx_core::integral_size_t::byte},
        {"word", cxx_core::integral_size_t::word},
        {"dword", cxx_core::integral_size_t::dword},
        {"qword", cxx_core::integral_size_t::qword},
        {nullptr, cxx_core::integral_size_t::zero},
    };

    cxx_core::revision_t find_revision(const s8* name) {
        for (u32 i = 0; s_revisions[i].name; ++i) {
            if (strcmp(s_revisions[i].name, name) == 0)
                return s_revisions[i].rev;
        }
        return cxx_core::revision_t::cpp20;
    }

    cxx_core::position_type_t find_position_type(const s8* name) {
        if (strcmp(name, "prefix") == 0) {
            return cxx_core::position_type_t::prefix;
        } else if (strcmp(name, "postfix") == 0) {
            return cxx_core::position_type_t::postfix;
        }
        return cxx_core::position_type_t::none;
    }

    cxx_core::integral_size_t find_integral_size(const s8* name) {
        for (u32 i = 0; s_sizes[i].name; ++i) {
            if (strcmp(s_sizes[i].name, name) == 0)
                return s_sizes[i].size;
        }
        return cxx_core::integral_size_t::zero;
    }

    obj_t* make_program() {
        auto pgm = obj_pool::make<cxx_core::program_t>(g_cxx_sys.storage);
        cxx_core::program::init(*pgm, g_cxx_sys.alloc);
        return make_user_ptr(g_cxx_sys.ctx, pgm);
    }

    u32 size_in_bits(str::slice_t* name) {
        return cxx_core::program::integral_size_in_bits(
            find_integral_size((const s8*)name->data));
    }

    u32 size_in_bytes(str::slice_t* name) {
        return cxx_core::program::integral_size_in_bytes(
            find_integral_size((const s8*)name->data));
    }

    obj_t* get_scope(obj_t* mod_usr, u32 id) {
        auto module = (cxx_core::module_t*) to_user_ptr(g_cxx_sys.ctx, mod_usr);
        auto& scope = cxx_core::module::get_scope(*module, id - 1);
        return make_user_ptr(g_cxx_sys.ctx, &scope);
    }

    obj_t* get_module(obj_t* pgm_usr, u32 id) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        auto& module = cxx_core::program::get_module(*pgm, id - 1);
        return make_user_ptr(g_cxx_sys.ctx, &module);
    }

    obj_t* add_module(obj_t* pgm_usr, str::slice_t* file_name, str::slice_t* revision) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        auto& module = cxx_core::program::add_module(
            *pgm,
            *file_name,
            find_revision((const s8*) revision->data));
        return make_user_ptr(g_cxx_sys.ctx, &module);
    }

    u0 pop_scope(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        cxx_core::scope::pop(*scope);
    }

    u32 push_scope(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::push(*scope);
    }

    u32 unary_not(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::lnot(*scope, expr_id);
    }

    u32 unary_deref(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::deref(*scope, expr_id);
    }

    u32 unary_negate(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::neg(*scope, expr_id);
    }

    u32 unary_address_of(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::addrof(*scope, expr_id);
    }

    u32 unary_binary_not(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::bnot(*scope, expr_id);
    }

    u32 make_label(obj_t* scope_usr, str::slice_t* name) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::label(*scope, *name);
    }

    u32 unary_address_of_label(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::addrof_label(*scope, expr_id);
    }

    u32 unary_increment(obj_t* scope_usr, u32 expr_id, str::slice_t* placement) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::inc(
            *scope,
            expr_id,
            find_position_type((const s8*) placement->data));
    }

    u32 unary_decrement(obj_t* scope_usr, u32 expr_id, str::slice_t* placement) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::dec(
            *scope,
            expr_id,
            find_position_type((const s8*) placement->data));
    }

    // XXX: move this to another module
    obj_t* intern_get(u32 id) {
        auto s = string::interned::get_slice(id);
        return s ? make_string(g_cxx_sys.ctx, *s) : nil(g_cxx_sys.ctx);
    }

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            static proc_export_t s_exports[] = {
                // XXX: move this to another module
                {"intern/get"_ss, 1,
                    {
                        {(u0*) intern_get, "intern_get"_ss, type_decl::obj_ptr, 1,
                            {
                                {"id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/make-program"_ss, 1,
                    {
                        {(u0*) make_program, "make_program"_ss, type_decl::obj_ptr, 0}
                    }
                },
                {"cxx/size-in-bits"_ss, 1,
                    {
                        {(u0*) size_in_bits, "size_in_bits"_ss, type_decl::u32_, 1,
                            {
                                {"name"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"cxx/size-in-bytes"_ss, 1,
                    {
                        {(u0*) size_in_bytes, "size_in_bytes"_ss, type_decl::u32_, 1,
                            {
                                {"name"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"cxx/get-scope"_ss, 1,
                    {
                        {(u0*) get_scope, "get_scope"_ss, type_decl::obj_ptr, 2,
                            {
                                {"mod_usr"_ss, type_decl::obj_ptr},
                                {"id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/get-module"_ss, 1,
                    {
                        {(u0*) get_module, "get_module"_ss, type_decl::obj_ptr, 2,
                            {
                                {"pgm_usr"_ss, type_decl::obj_ptr},
                                {"id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/add-module"_ss, 1,
                    {
                        {(u0*) add_module, "add_module"_ss, type_decl::obj_ptr, 3,
                            {
                                {"pgm_usr"_ss, type_decl::obj_ptr},
                                {"file_name"_ss, type_decl::slice_ptr},
                                {"revision"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"cxx/pop-scope"_ss, 1,
                    {
                        {(u0*) pop_scope, "pop_scope"_ss, type_decl::u0_, 1,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },
                {"cxx/push-scope"_ss, 1,
                    {
                        {(u0*) push_scope, "push_scope"_ss, type_decl::u32_, 1,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },
                {"cxx/make-label"_ss, 1,
                    {
                        {(u0*) make_label, "make-label"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"name"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"cxx/unary/!"_ss, 1,
                    {
                        {(u0*) unary_not, "unary_not"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/~"_ss, 1,
                    {
                        {(u0*) unary_binary_not, "unary_binary_not"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/*"_ss, 1,
                    {
                        {(u0*) unary_deref, "unary_deref"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/-"_ss, 1,
                    {
                        {(u0*) unary_negate, "unary_negate"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/&"_ss, 1,
                    {
                        {(u0*) unary_address_of, "unary_address_of"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/&&"_ss, 1,
                    {
                        {(u0*) unary_address_of_label, "unary_address_of_label"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"cxx/unary/++"_ss, 1,
                    {
                        {(u0*) unary_increment, "unary_increment"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                                {"placement"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"cxx/unary/--"_ss, 1,
                    {
                        {(u0*) unary_decrement, "unary_decrement"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                                {"placement"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {str::slice_t{}},
           };
        }

        u0 fini() {
            for (auto obj : g_cxx_sys.program_type->objects) {
                cxx_core::program::free(*((cxx_core::program_t*) obj));
            }
            obj_pool::free(g_cxx_sys.storage);
        }

        status_t init(ctx_t* ctx, alloc_t* alloc) {
            g_cxx_sys.alloc = alloc;
            g_cxx_sys.ctx   = ctx;
            obj_pool::init(g_cxx_sys.storage, g_cxx_sys.alloc);
            kernel::create_exports(g_cxx_sys.ctx, exports::s_exports);
            g_cxx_sys.program_type = obj_pool::register_type<cxx_core::program_t>(g_cxx_sys.storage);
            return status_t::ok;
        }
    }
}
