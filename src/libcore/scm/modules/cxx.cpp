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

#include <basecode/core/obj_pool.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/kernel.h>
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
        basecode::obj_type_t*   serializer_type;
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

    cxx_core::revision_t find_revision(const s8* name);

    cxx_core::integral_size_t find_integral_size(const s8* name);

    cxx_core::position_type_t find_position_type(const s8* name);

    obj_t* make_program() {
        auto pgm = obj_pool::make<cxx_core::program_t>(g_cxx_sys.storage);
        cxx_core::program::init(*pgm, g_cxx_sys.alloc);
        return make_user_ptr(g_cxx_sys.ctx, pgm);
    }

    u32 stmt_do(obj_t* scope_usr,
                u32 pred,
                u32 expr,
                u32 label) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::do_(*scope, pred, expr, label);
    }

    u32 stmt_case(obj_t* scope_usr,
                  u32 pred,
                  u32 expr,
                  u32 label) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::case_(*scope, pred, expr, label);
    }

    u32 stmt_while(obj_t* scope_usr,
                   u32 pred,
                   u32 expr,
                   u32 label) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::while_(*scope, pred, expr, label);
    }

    u32 stmt_switch(obj_t* scope_usr,
                    u32 pred,
                    u32 expr,
                    u32 label) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::switch_(*scope, pred, expr, label);
    }

    u32 stmt_if(obj_t* scope_usr,
                 u32 pred,
                 u32 true_expr,
                 u32 false_expr = 0,
                 u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::if_(*scope, pred, true_expr, false_expr, label);
    }

    u32 stmt_for(obj_t* scope_usr,
                 u32 pred,
                 u32 expr,
                 u32 init = 0,
                 u32 post = 0,
                 u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::for_(*scope, pred, expr, init, post, label);
    }

    u32 make_var(obj_t* scope_usr,
                 u32 type,
                 u32 ident,
                 u32 init = 0,
                 u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::var(*scope, type, ident, init, flags);
    }

    u32 serialize(obj_t* ser_usr) {
        auto ser = (cxx_core::serializer_t*) to_user_ptr(g_cxx_sys.ctx, ser_usr);
        return u32(cxx_core::serializer::serialize(*ser));
    }

    u0 pop_scope(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        cxx_core::scope::pop(*scope);
    }

    u32 push_scope(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::push(*scope);
    }

    u32 stmt_public(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::public_(*scope);
    }

    u32 type_enum_class(obj_t* scope_usr,
                        u32 block,
                        u32 ident,
                        u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::enum_class_(*scope, block, ident, flags);
    }

    u32 stmt_private(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::private_(*scope);
    }

    u32 type_func(obj_t* scope_usr,
                  u32 block,
                  u32 return_type,
                  u32 ident,
                  u32 params_list = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::func(*scope,
                                           block,
                                           return_type,
                                           ident,
                                           params_list);
    }

    u32 finalize_program(obj_t* pgm_usr) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        return u32(cxx_core::program::finalize(*pgm));
    }

    u32 stmt_protected(obj_t* scope_usr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::protected_(*scope);
    }

    u32 size_in_bits(str::slice_t* name) {
        return cxx_core::program::integral_size_in_bits(
            find_integral_size((const s8*)name->data));
    }

    u32 size_in_bytes(str::slice_t* name) {
        return cxx_core::program::integral_size_in_bytes(
            find_integral_size((const s8*)name->data));
    }

    u32 stmt_def(obj_t* scope_usr, u32 expr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::def(*scope, expr);
    }

    u32 lit_char(obj_t* scope_usr, s8 value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::lit::chr(*scope, value);
    }

    u32 type_ptr(obj_t* scope_usr, u32 type) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::ptr(*scope, type);
    }

    u32 type_ref(obj_t* scope_usr, u32 type) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::ref(*scope, type);
    }

    obj_t* get_scope(obj_t* mod_usr, u32 id) {
        auto module = (cxx_core::module_t*) to_user_ptr(g_cxx_sys.ctx, mod_usr);
        auto& scope = cxx_core::module::get_scope(*module, id - 1);
        return make_user_ptr(g_cxx_sys.ctx, &scope);
    }

    u32 type_u0(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::u0_(*scope, ident);
    }

    u32 type_s8(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::s8_(*scope, ident);
    }

    u32 type_u8(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::u8_(*scope, ident);
    }

    u32 type_b8(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::b8_(*scope, ident);
    }

    u32 type_s16(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::s16_(*scope, ident);
    }

    u32 type_u16(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::u16_(*scope, ident);
    }

    u32 type_s32(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::s32_(*scope, ident);
    }

    u32 type_u32(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::u32_(*scope, ident);
    }

    u32 type_s64(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::s64_(*scope, ident);
    }

    u32 type_u64(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::u64_(*scope, ident);
    }

    u32 type_f32(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::f32_(*scope, ident);
    }

    u32 type_f64(obj_t* scope_usr, u32 ident) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::f64_(*scope, ident);
    }

    obj_t* get_module(obj_t* pgm_usr, u32 id) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        auto& module = cxx_core::program::get_module(*pgm, id - 1);
        return make_user_ptr(g_cxx_sys.ctx, &module);
    }

    obj_t* add_module(obj_t* pgm_usr,
                      str::slice_t* file_name,
                      str::slice_t* revision) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        auto& module = cxx_core::program::add_module(
            *pgm,
            *file_name,
            find_revision((const s8*) revision->data));
        return make_user_ptr(g_cxx_sys.ctx, &module);
    }

    u32 unary_not(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::lnot(*scope, expr_id);
    }

    u32 stmt_break(obj_t* scope_usr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::break_(*scope, label);
    }

    u32 stmt_using_ns(obj_t* scope_usr, u32 expr) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::using_ns(*scope, expr);
    }

    u32 init_assign(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::init::direct(*scope, expr_id);
    }

    u32 unary_deref(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::deref(*scope, expr_id);
    }

    u32 stmt_empty(obj_t* scope_usr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::empty(*scope, label);
    }

    u32 unary_negate(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::neg(*scope, expr_id);
    }

    u32 binary_lt(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::lt(*scope, lhs, rhs);
    }

    u32 binary_eq(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::eq(*scope, lhs, rhs);
    }

    u32 binary_gt(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::gt(*scope, lhs, rhs);
    }

    u32 lit_str(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::lit::str(*scope, *value);
    }

    u32 assign_shl(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::shl(*scope, lhs, rhs);
    }

    u32 assign_shr(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::shr(*scope, lhs, rhs);
    }

    u32 stmt_continue(obj_t* scope_usr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::continue_(*scope, label);
    }

    cxx_core::revision_t find_revision(const s8* name) {
        for (u32 i = 0; s_revisions[i].name; ++i) {
            if (strcmp(s_revisions[i].name, name) == 0)
                return s_revisions[i].rev;
        }
        return cxx_core::revision_t::cpp20;
    }

    u32 assign_bor(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::bor(*scope, lhs, rhs);
    }

    u32 binary_mul(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::mul(*scope, lhs, rhs);
    }

    u32 binary_neq(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::neq(*scope, lhs, rhs);
    }

    u32 binary_add(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::add(*scope, lhs, rhs);
    }

    u32 binary_gte(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::gte(*scope, lhs, rhs);
    }

    u32 binary_lte(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::lte(*scope, lhs, rhs);
    }

    u32 binary_sub(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::sub(*scope, lhs, rhs);
    }

    u32 binary_div(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::div(*scope, lhs, rhs);
    }

    u32 binary_mod(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::mod(*scope, lhs, rhs);
    }

    u32 binary_shl(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::shl(*scope, lhs, rhs);
    }

    u32 binary_shr(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::shr(*scope, lhs, rhs);
    }

    u32 binary_bor(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::bor(*scope, lhs, rhs);
    }

    u32 binary_lor(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::lor(*scope, lhs, rhs);
    }

    u32 assign_sum(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::sum(*scope, lhs, rhs);
    }

    u32 assign_band(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::band(*scope, lhs, rhs);
    }

    u32 unary_address_of(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::addrof(*scope, expr_id);
    }

    u32 unary_binary_not(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::bnot(*scope, expr_id);
    }

    u32 binary_cast(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::cast(*scope, lhs, rhs);
    }

    u32 binary_band(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::band(*scope, lhs, rhs);
    }

    u32 assign_bxor(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::bxor(*scope, lhs, rhs);
    }

    u32 assign_diff(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::diff(*scope, lhs, rhs);
    }

    u32 binary_bxor(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::bxor(*scope, lhs, rhs);
    }

    u32 make_list(obj_t* scope_usr, rest_array_t* rest) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        u32 ids[rest->size];
        u32 i{};
        for (auto obj : *rest)
            ids[i++] = FIXNUM(obj);
        return cxx_core::scope::expr::list(*scope, ids, rest->size);
    }

    u32 binary_land(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::land(*scope, lhs, rhs);
    }

    u32 init_list(obj_t* scope_usr, rest_array_t* rest) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        u32 ids[rest->size];
        u32 i{};
        for (auto obj : *rest)
            ids[i++] = FIXNUM(obj);
        return cxx_core::scope::expr::init::list(*scope, ids, rest->size);
    }

    u32 binary_comma(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::comma(*scope, lhs, rhs);
    }

    u32 binary_range(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::range(*scope, lhs, rhs);
    }

    u32 stmt_raw(obj_t* scope_usr, str::slice_t* source) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::raw(*scope, *source);
    }

    u32 make_ident(obj_t* scope_usr, str::slice_t* name) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::ident(*scope, *name);
    }

    u32 make_raw(obj_t* scope_usr, str::slice_t* source) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::raw(*scope, *source);
    }

    u32 make_label(obj_t* scope_usr, str::slice_t* name) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::label(*scope, *name);
    }

    u32 binary_member(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::member(*scope, lhs, rhs);
    }

    u32 pragma_raw(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::pp::pragma(*scope, *value);
    }

    u32 assign_direct(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::direct(*scope, lhs, rhs);
    }

    u32 stmt_using(obj_t* scope_usr, u32 ident, u32 type) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::using_(*scope, ident, type);
    }

    u32 type_bit_field(obj_t* scope_usr, u32 type, u8 bits) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::bit_mask(*scope, type, bits);
    }

    u32 assign_product(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::product(*scope, lhs, rhs);
    }

    u32 assign_quotient(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::quotient(*scope, lhs, rhs);
    }

    u32 binary_subscript(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::subscript(*scope, lhs, rhs);
    }

    u32 binary_scope_res(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::binary::scope_res(*scope, lhs, rhs);
    }

    u32 type_array(obj_t* scope_usr, u32 type, u32 size = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::array(*scope, type, size);
    }

    u32 assign_remainder(obj_t* scope_usr, u32 lhs, u32 rhs) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::assign::remainder(*scope, lhs, rhs);
    }

    u32 stmt_expr(obj_t* scope_usr, u32 expr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::expr(*scope, expr, label);
    }

    u32 stmt_decl(obj_t* scope_usr, u32 expr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::decl(*scope, expr, label);
    }

    u32 stmt_goto(obj_t* scope_usr, u32 expr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::goto_(*scope, expr, label);
    }

    u32 stmt_return(obj_t* scope_usr, u32 expr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::return_(*scope, expr, label);
    }

    u32 stmt_default(obj_t* scope_usr, u32 expr, u32 label = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::default_(*scope, expr, label);
    }

    u32 unary_address_of_label(obj_t* scope_usr, u32 expr_id) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::addrof_label(*scope, expr_id);
    }

    cxx_core::integral_size_t find_integral_size(const s8* name) {
        for (u32 i = 0; s_sizes[i].name; ++i) {
            if (strcmp(s_sizes[i].name, name) == 0)
                return s_sizes[i].size;
        }
        return cxx_core::integral_size_t::zero;
    }

    u32 stmt_comment_line(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::comment::line(*scope, *value);
    }

    cxx_core::position_type_t find_position_type(const s8* name) {
        if (strcmp(name, "prefix") == 0) {
            return cxx_core::position_type_t::prefix;
        } else if (strcmp(name, "postfix") == 0) {
            return cxx_core::position_type_t::postfix;
        }
        return cxx_core::position_type_t::none;
    }

    u32 stmt_comment_block(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::comment::block(*scope, *value);
    }

    u32 lit_float(obj_t* scope_usr, f64 value, str::slice_t* size) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::lit::float_(*scope,
                                            value,
                                            find_integral_size((const s8*) size->data));
    }

    u32 pragma_include_local(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::pp::include_local(*scope, *value);
    }

    u32 pragma_include_system(obj_t* scope_usr, str::slice_t* value) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::stmt::pp::include_system(*scope, *value);
    }

    u32 type_enum(obj_t* scope_usr, u32 block, u32 ident = 0, u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::enum_(*scope, block, ident, flags);
    }

    u32 type_class(obj_t* scope_usr, u32 block, u32 ident = 0, u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::class_(*scope, block, ident, flags);
    }

    u32 type_union(obj_t* scope_usr, u32 block, u32 ident = 0, u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::union_(*scope, block, ident, flags);
    }

    u32 type_struct(obj_t* scope_usr, u32 block, u32 ident = 0, u8 flags = 0) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::type::struct_(*scope, block, ident, flags);
    }

    u32 unary_increment(obj_t* scope_usr, u32 expr_id, str::slice_t* placement) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::inc(
            *scope,
            expr_id,
            find_position_type((const s8*) placement->data));
    }

    obj_t* make_serializer(obj_t* pgm_usr, u16 margin = 160, u16 tab_width = 4) {
        auto pgm = (cxx_core::program_t*) to_user_ptr(g_cxx_sys.ctx, pgm_usr);
        auto ser = obj_pool::make<cxx_core::serializer_t>(g_cxx_sys.storage);
        cxx_core::serializer::init(*ser,
                                   *pgm,
                                   g_cxx_sys.alloc,
                                   margin,
                                   tab_width);
        return make_user_ptr(g_cxx_sys.ctx, ser);
    }

    u32 unary_decrement(obj_t* scope_usr, u32 expr_id, str::slice_t* placement) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::expr::unary::dec(
            *scope,
            expr_id,
            find_position_type((const s8*) placement->data));
    }

    u32 lit_signed(obj_t* scope_usr, f64 value, str::slice_t* size, u32 radix = 10) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::lit::signed_(*scope,
                                             value,
                                             find_integral_size((const s8*) size->data),
                                             radix);
    }

    u32 lit_unsigned(obj_t* scope_usr, f64 value, str::slice_t* size, u32 radix = 10) {
        auto scope = (cxx_core::scope_t*) to_user_ptr(g_cxx_sys.ctx, scope_usr);
        return cxx_core::scope::lit::unsigned_(*scope,
                                               value,
                                               find_integral_size((const s8*) size->data),
                                               radix);
    }

#define EXPORTS EXPORT_PROC("cxx/make-program",                                 \
                    OVERLOAD(make_program, obj_ptr))                            \
                                                                                \
                EXPORT_PROC("cxx/serialize",                                    \
                    OVERLOAD(serialize, u32,                                    \
                        REQ("ser_usr", obj_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/finalize-program",                             \
                    OVERLOAD(finalize_program, obj_ptr,                         \
                        REQ("pgm_usr", obj_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/make-serializer",                              \
                    OVERLOAD(make_serializer, obj_ptr,                          \
                        REQ("pgm_usr", obj_ptr),                                \
                        OPT("width", u16, 160),                                 \
                        OPT("tab-width", u16, 4)))                              \
                                                                                \
                EXPORT_PROC("size-in-bits",                                     \
                    OVERLOAD(size_in_bits, u32,                                 \
                        REQ("name", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("size-in-bytes",                                    \
                    OVERLOAD(size_in_bytes, u32,                                \
                        REQ("name", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("cxx/get-scope",                                    \
                    OVERLOAD(get_scope, obj_ptr,                                \
                        REQ("mod_usr", obj_ptr),                                \
                        REQ("id", u32)))                                        \
                                                                                \
                EXPORT_PROC("cxx/get-module",                                   \
                    OVERLOAD(get_module, obj_ptr,                               \
                        REQ("pgm_usr", obj_ptr),                                \
                        REQ("id", u32)))                                        \
                                                                                \
                EXPORT_PROC("cxx/add-module",                                   \
                    OVERLOAD(add_module, obj_ptr,                               \
                        REQ("pgm_usr", obj_ptr),                                \
                        REQ("file_name", slice_ptr),                            \
                        REQ("revision", slice_ptr)))                            \
                                                                                \
                EXPORT_PROC("cxx/pop-scope",                                    \
                    OVERLOAD(pop_scope, u0,                                     \
                        REQ("scope_usr", obj_ptr)))                             \
                                                                                \
                EXPORT_PROC("cxx/push-scope",                                   \
                    OVERLOAD(push_scope, u32,                                   \
                        REQ("scope_usr", obj_ptr)))                             \
                                                                                \
                EXPORT_PROC("cxx/make-label",                                   \
                    OVERLOAD(make_label, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("name", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("cxx/type/*",                                       \
                    OVERLOAD(type_ptr, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("type_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/&",                                       \
                    OVERLOAD(type_ref, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("type_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/u0",                                      \
                    OVERLOAD(type_u0, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/u8",                                      \
                    OVERLOAD(type_u8, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/s8",                                      \
                    OVERLOAD(type_s8, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/b8",                                      \
                    OVERLOAD(type_b8, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/u16",                                     \
                    OVERLOAD(type_u16, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/s16",                                     \
                    OVERLOAD(type_s16, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/u32",                                     \
                    OVERLOAD(type_u32, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/s32",                                     \
                    OVERLOAD(type_s32, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/u64",                                     \
                    OVERLOAD(type_u64, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/s64",                                     \
                    OVERLOAD(type_s64, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/f32",                                     \
                    OVERLOAD(type_f32, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/f64",                                     \
                    OVERLOAD(type_f64, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32)))                                  \
                                                                                \
                EXPORT_PROC("cxx/type/bit-field",                               \
                    OVERLOAD(type_bit_field, u32,                               \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("type_id", u32),                                    \
                        REQ("size", u32)))                                      \
                                                                                \
                EXPORT_PROC("cxx/type/array",                                   \
                    OVERLOAD(type_array, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("type_id", u32),                                    \
                        REQ("size", u32)))                                      \
                                                                                \
                EXPORT_PROC("cxx/type/enum",                                    \
                    OVERLOAD(type_enum, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("block_id", u32),                                   \
                        REQ("ident_id", u32),                                   \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/class",                                   \
                    OVERLOAD(type_class, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("block_id", u32),                                   \
                        REQ("ident_id", u32),                                   \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/union",                                   \
                    OVERLOAD(type_union, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("block_id", u32),                                   \
                        REQ("ident_id", u32),                                   \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/struct",                                  \
                    OVERLOAD(type_struct, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("block_id", u32),                                   \
                        REQ("ident_id", u32),                                   \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/enum-class",                              \
                    OVERLOAD(type_enum_class, u32,                              \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("block_id", u32),                                   \
                        REQ("ident_id", u32),                                   \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/type/function",                                \
                    OVERLOAD(type_func, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ret_type_id", u32),                                \
                        REQ("ident_id", u32),                                   \
                        OPT("params_list_id", u32, 0)))                         \
                                                                                \
                EXPORT_PROC("cxx/unary/!",                                      \
                    OVERLOAD(unary_not, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/~",                                      \
                    OVERLOAD(unary_binary_not, u32,                             \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/*",                                      \
                    OVERLOAD(unary_deref, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/-",                                      \
                    OVERLOAD(unary_negate, u32,                                 \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/&",                                      \
                    OVERLOAD(unary_address_of, u32,                             \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/&&",                                     \
                    OVERLOAD(unary_address_of_label, u32,                       \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/unary/++",                                     \
                    OVERLOAD(unary_increment, u32,                              \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        REQ("placement", slice_ptr)))                           \
                                                                                \
                EXPORT_PROC("cxx/unary/--",                                     \
                    OVERLOAD(unary_decrement, u32,                              \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        REQ("placement", slice_ptr)))                           \
                                                                                \
                EXPORT_PROC("cxx/init/=",                                       \
                    OVERLOAD(init_assign, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/init/{}",                                      \
                    OVERLOAD(init_list, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REST("rest", obj_ptr)))                                 \
                                                                                \
                EXPORT_PROC("cxx/assign/=",                                     \
                    OVERLOAD(assign_direct, u32,                                \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/+=",                                    \
                    OVERLOAD(assign_sum, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/<<=",                                   \
                    OVERLOAD(assign_shl, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/>>=",                                   \
                    OVERLOAD(assign_shr, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/|=",                                    \
                    OVERLOAD(assign_bor, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/&=",                                    \
                    OVERLOAD(assign_band, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/^=",                                    \
                    OVERLOAD(assign_bxor, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/-=",                                    \
                    OVERLOAD(assign_diff, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/*=",                                    \
                    OVERLOAD(assign_product, u32,                               \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign//=",                                    \
                    OVERLOAD(assign_quotient, u32,                              \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/assign/%=",                                    \
                    OVERLOAD(assign_remainder, u32,                             \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/==",                                    \
                    OVERLOAD(binary_eq, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/!=",                                    \
                    OVERLOAD(binary_neq, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/<",                                     \
                    OVERLOAD(binary_lt, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/<=",                                    \
                    OVERLOAD(binary_lte, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/>",                                     \
                    OVERLOAD(binary_gt, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/>=",                                    \
                    OVERLOAD(binary_gte, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/+",                                     \
                    OVERLOAD(binary_add, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/-",                                     \
                    OVERLOAD(binary_sub, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/*",                                     \
                    OVERLOAD(binary_mul, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary//",                                     \
                    OVERLOAD(binary_div, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/%",                                     \
                    OVERLOAD(binary_mod, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/^",                                     \
                    OVERLOAD(binary_bxor, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/<<",                                    \
                    OVERLOAD(binary_shl, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/>>",                                    \
                    OVERLOAD(binary_shr, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/|",                                     \
                    OVERLOAD(binary_bor, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/&",                                     \
                    OVERLOAD(binary_band, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/&&",                                    \
                    OVERLOAD(binary_land, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/||",                                    \
                    OVERLOAD(binary_lor, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/:",                                     \
                    OVERLOAD(binary_range, u32,                                 \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/::",                                    \
                    OVERLOAD(binary_scope_res, u32,                             \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/.",                                     \
                    OVERLOAD(binary_member, u32,                                \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/,",                                     \
                    OVERLOAD(binary_comma, u32,                                 \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/cast",                                  \
                    OVERLOAD(binary_cast, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/binary/subscript",                             \
                    OVERLOAD(binary_subscript, u32,                             \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("lhs", u32),                                        \
                        REQ("rhs", u32)))                                       \
                                                                                \
                EXPORT_PROC("cxx/lit/char",                                     \
                    OVERLOAD(lit_char, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", u8)))                                      \
                                                                                \
                EXPORT_PROC("cxx/lit/string",                                   \
                    OVERLOAD(lit_str, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/lit/float",                                    \
                    OVERLOAD(lit_float, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", f32),                                      \
                        REQ("size", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("cxx/lit/signed",                                   \
                    OVERLOAD(lit_signed, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", u32),                                      \
                        REQ("size", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("cxx/lit/unsigned",                                 \
                    OVERLOAD(lit_unsigned, u32,                                 \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", u32),                                      \
                        REQ("size", slice_ptr)))                                \
                                                                                \
                EXPORT_PROC("cxx/stmt/raw-pragma",                              \
                    OVERLOAD(pragma_raw, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/include",                                 \
                    OVERLOAD(pragma_include_local, u32,                         \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/include<>",                               \
                    OVERLOAD(pragma_include_system, u32,                        \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/raw",                                     \
                    OVERLOAD(stmt_raw, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("source", slice_ptr)))                              \
                                                                                \
                EXPORT_PROC("cxx/stmt/public",                                  \
                    OVERLOAD(stmt_public, u32,                                  \
                        REQ("scope_usr", obj_ptr)))                             \
                                                                                \
                EXPORT_PROC("cxx/stmt/private",                                 \
                    OVERLOAD(stmt_private, u32,                                 \
                        REQ("scope_usr", obj_ptr)))                             \
                                                                                \
                EXPORT_PROC("cxx/stmt/protected",                               \
                    OVERLOAD(stmt_protected, u32,                               \
                        REQ("scope_usr", obj_ptr)))                             \
                                                                                \
                EXPORT_PROC("cxx/stmt/line-comment",                            \
                    OVERLOAD(stmt_comment_line, u32,                            \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/block-comment",                           \
                    OVERLOAD(stmt_comment_block, u32,                           \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("value", slice_ptr)))                               \
                                                                                \
                EXPORT_PROC("cxx/expr/raw",                                     \
                    OVERLOAD(make_raw, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("source", slice_ptr)))                              \
                                                                                \
                EXPORT_PROC("cxx/expr/var",                                     \
                    OVERLOAD(make_var, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("type_id", u32),                                    \
                        REQ("ident_id", u32),                                   \
                        OPT("init", u32, 0),                                    \
                        OPT("flags", u8, 0)))                                   \
                                                                                \
                EXPORT_PROC("cxx/expr/ident",                                   \
                    OVERLOAD(make_ident, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("source", slice_ptr)))                              \
                                                                                \
                EXPORT_PROC("cxx/expr/list",                                    \
                    OVERLOAD(make_list, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REST("rest", obj_ptr)))                                 \
                                                                                \
                EXPORT_PROC("cxx/stmt/if",                                      \
                    OVERLOAD(stmt_if, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("true_id", u32),                                    \
                        OPT("false_id", u32, 0),                                \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/for",                                     \
                    OVERLOAD(stmt_for, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("expr_id", u32),                                    \
                        OPT("init_id", u32, 0),                                 \
                        OPT("post_id", u32, 0),                                 \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/do",                                      \
                    OVERLOAD(stmt_do, u32,                                      \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/case",                                    \
                    OVERLOAD(stmt_case, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/while",                                   \
                    OVERLOAD(stmt_while, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/switch",                                  \
                    OVERLOAD(stmt_switch, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("predicate_id", u32),                               \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/expr",                                    \
                    OVERLOAD(stmt_expr, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/decl",                                    \
                    OVERLOAD(stmt_decl, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/goto",                                    \
                    OVERLOAD(stmt_goto, u32,                                    \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/return",                                  \
                    OVERLOAD(stmt_return, u32,                                  \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/default",                                 \
                    OVERLOAD(stmt_default, u32,                                 \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32),                                    \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/define",                                  \
                    OVERLOAD(stmt_def, u32,                                     \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/stmt/using",                                   \
                    OVERLOAD(stmt_using, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("ident_id", u32),                                   \
                        REQ("type_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/stmt/using-namespace",                         \
                    OVERLOAD(stmt_using_ns, u32,                                \
                        REQ("scope_usr", obj_ptr),                              \
                        REQ("expr_id", u32)))                                   \
                                                                                \
                EXPORT_PROC("cxx/stmt/empty",                                   \
                    OVERLOAD(stmt_empty, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/break",                                   \
                    OVERLOAD(stmt_break, u32,                                   \
                        REQ("scope_usr", obj_ptr),                              \
                        OPT("label_id", u32, 0)))                               \
                                                                                \
                EXPORT_PROC("cxx/stmt/continue",                                \
                    OVERLOAD(stmt_continue, u32,                                \
                        REQ("scope_usr", obj_ptr),                              \
                        OPT("label_id", u32, 0)))

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            static proc_export_t s_exports[] = {
#define EXPORT_PROC(n, ...)    basecode::scm::kernel::proc_export_t{    \
    n##_ss, \
    u32(VA_COUNT(__VA_ARGS__)), \
    __VA_ARGS__},
                EXPORTS
#undef EXPORT_PROC
                {str::slice_t{}},
           };
        }

        u0 fini() {
            for (auto obj : g_cxx_sys.serializer_type->objects)
                cxx_core::serializer::free(*((cxx_core::serializer_t*) obj));
            for (auto obj : g_cxx_sys.program_type->objects)
                cxx_core::program::free(*((cxx_core::program_t*) obj));
            obj_pool::free(g_cxx_sys.storage);
        }

        status_t init(ctx_t* ctx, alloc_t* alloc) {
            g_cxx_sys.alloc = alloc;
            g_cxx_sys.ctx   = ctx;
            obj_pool::init(g_cxx_sys.storage, g_cxx_sys.alloc);
            kernel::create_exports(g_cxx_sys.ctx, exports::s_exports);
            g_cxx_sys.program_type = obj_pool::register_type<cxx_core::program_t>(g_cxx_sys.storage);
            g_cxx_sys.serializer_type = obj_pool::register_type<cxx_core::serializer_t>(g_cxx_sys.storage);
            return status_t::ok;
        }
    }
}
