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
#include <basecode/core/scm/types.h>
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
        return cxx_core::scope::type::func(*scope, block, return_type, ident, params_list);
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

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            static proc_export_t s_exports[] = {
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

                {"cxx/type/*"_ss, 1,
                    {
                        {(u0*) type_ptr, "type_ptr"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"type_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/&"_ss, 1,
                    {
                        {(u0*) type_ref, "type_ref"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"type_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/u0"_ss, 1,
                    {
                        {(u0*) type_u0, "type_u0"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/u8"_ss, 1,
                    {
                        {(u0*) type_u8, "type_u8"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/s8"_ss, 1,
                    {
                        {(u0*) type_s8, "type_s8"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/b8"_ss, 1,
                    {
                        {(u0*) type_b8, "type_b8"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/u16"_ss, 1,
                    {
                        {(u0*) type_u16, "type_u16"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/s16"_ss, 1,
                    {
                        {(u0*) type_s16, "type_s16"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/u32"_ss, 1,
                    {
                        {(u0*) type_u32, "type_u32"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/s32"_ss, 1,
                    {
                        {(u0*) type_s32, "type_s32"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/u64"_ss, 1,
                    {
                        {(u0*) type_u64, "type_u64"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/s64"_ss, 1,
                    {
                        {(u0*) type_s64, "type_s64"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/f32"_ss, 1,
                    {
                        {(u0*) type_f32, "type_f32"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/f64"_ss, 1,
                    {
                        {(u0*) type_f64, "type_f64"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/bit-field"_ss, 1,
                    {
                        {(u0*) type_bit_field, "type_bit_field"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"type_id"_ss, type_decl::u32_},
                                {"size"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/array"_ss, 1,
                    {
                        {(u0*) type_array, "type_array"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"type_id"_ss, type_decl::u32_},
                                {"size"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/type/enum"_ss, 1,
                    {
                        {(u0*) type_enum, "type_enum"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"block_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/type/class"_ss, 1,
                    {
                        {(u0*) type_class, "type_class"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"block_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/type/union"_ss, 1,
                    {
                        {(u0*) type_union, "type_union"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"block_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/type/struct"_ss, 1,
                    {
                        {(u0*) type_struct, "type_struct"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"block_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/type/enum-class"_ss, 1,
                    {
                        {(u0*) type_enum_class, "type_enum_class"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"block_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/type/func"_ss, 1,
                    {
                        {(u0*) type_func, "type_func"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ret_type_id"_ss, type_decl::u32_},
                                {"ident_id"_ss, type_decl::u32_},
                                {"params_list_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
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

                {"cxx/init/="_ss, 1,
                    {
                        {(u0*) init_assign, "init_assign"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/init/{}"_ss, 1,
                    {
                        {(u0*) init_list, "init_list"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"rest"_ss, type_decl::obj_ptr, .is_rest = true},
                            }
                        }
                    }
                },

                {"cxx/assign/="_ss, 1,
                    {
                        {(u0*) assign_direct, "assign_direct"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/+="_ss, 1,
                    {
                        {(u0*) assign_sum, "assign_sum"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/<<="_ss, 1,
                    {
                        {(u0*) assign_shl, "assign_shl"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/>>="_ss, 1,
                    {
                        {(u0*) assign_shl, "assign_shr"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/|="_ss, 1,
                    {
                        {(u0*) assign_bor, "assign_bor"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/^="_ss, 1,
                    {
                        {(u0*) assign_bor, "assign_xor"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/-="_ss, 1,
                    {
                        {(u0*) assign_diff, "assign_diff"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/*="_ss, 1,
                    {
                        {(u0*) assign_product, "assign_product"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign//="_ss, 1,
                    {
                        {(u0*) assign_quotient, "assign_quotient"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/assign/%="_ss, 1,
                    {
                        {(u0*) assign_remainder, "assign_remainder"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/expr/raw"_ss, 1,
                    {
                        {(u0*) make_raw, "make_raw"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"source"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/expr/var"_ss, 1,
                    {
                        {(u0*) make_var, "make_var"_ss, type_decl::u32_, 5,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"type"_ss, type_decl::u32_},
                                {"ident"_ss, type_decl::u32_},
                                {"init"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"flags"_ss, type_decl::u8_, .default_value.b = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/expr/ident"_ss, 1,
                    {
                        {(u0*) make_ident, "make_ident"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"source"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/expr/list"_ss, 1,
                    {
                        {(u0*) make_list, "make_list"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"rest"_ss, type_decl::obj_ptr, .is_rest = true},
                            }
                        }
                    }
                },

                {"cxx/binary/=="_ss, 1,
                    {
                        {(u0*) binary_eq, "binary_eq"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/!="_ss, 1,
                    {
                        {(u0*) binary_neq, "binary_neq"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/<"_ss, 1,
                    {
                        {(u0*) binary_lt, "binary_lt"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/<="_ss, 1,
                    {
                        {(u0*) binary_lte, "binary_lte"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/>"_ss, 1,
                    {
                        {(u0*) binary_gt, "binary_gt"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/>="_ss, 1,
                    {
                        {(u0*) binary_gte, "binary_gte"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/+"_ss, 1,
                    {
                        {(u0*) binary_add, "binary_add"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/-"_ss, 1,
                    {
                        {(u0*) binary_sub, "binary_sub"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/*"_ss, 1,
                    {
                        {(u0*) binary_mul, "binary_mul"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary//"_ss, 1,
                    {
                        {(u0*) binary_div, "binary_div"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/%"_ss, 1,
                    {
                        {(u0*) binary_mod, "binary_mod"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/^"_ss, 1,
                    {
                        {(u0*) binary_bxor, "binary_bxor"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/<<"_ss, 1,
                    {
                        {(u0*) binary_shl, "binary_shl"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/>>"_ss, 1,
                    {
                        {(u0*) binary_shr, "binary_shr"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/|"_ss, 1,
                    {
                        {(u0*) binary_bor, "binary_bor"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/||"_ss, 1,
                    {
                        {(u0*) binary_lor, "binary_lor"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/&"_ss, 1,
                    {
                        {(u0*) binary_band, "binary_band"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/&&"_ss, 1,
                    {
                        {(u0*) binary_land, "binary_land"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/:"_ss, 1,
                    {
                        {(u0*) binary_range, "binary_range"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/::"_ss, 1,
                    {
                        {(u0*) binary_scope_res, "binary_scope_res"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/."_ss, 1,
                    {
                        {(u0*) binary_member, "binary_member"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/,"_ss, 1,
                    {
                        {(u0*) binary_comma, "binary_comma"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/binary/subscript"_ss, 1,
                    {
                        {(u0*) binary_subscript, "binary_subscript"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"lhs"_ss, type_decl::u32_},
                                {"rhs"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/lit/char"_ss, 1,
                    {
                        {(u0*) lit_char, "lit_char"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::u8_},
                            }
                        }
                    }
                },

                {"cxx/lit/string"_ss, 1,
                    {
                        {(u0*) lit_str, "lit_str"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/lit/float"_ss, 1,
                    {
                        {(u0*) lit_float, "lit_f32"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::f32_},
                                {"size"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/lit/signed"_ss, 1,
                    {
                        {(u0*) lit_signed, "lit_signed"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::u32_},
                                {"size"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/lit/unsigned"_ss, 1,
                    {
                        {(u0*) lit_unsigned, "lit_unsigned"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::u32_},
                                {"size"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/raw-pragma"_ss, 1,
                    {
                        {(u0*) pragma_raw, "pragma_raw"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/include"_ss, 1,
                    {
                        {(u0*) pragma_include_local, "pragma_include_local"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"path"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/include<>"_ss, 1,
                    {
                        {(u0*) pragma_include_system, "pragma_include_system"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"path"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/raw"_ss, 1,
                    {
                        {(u0*) stmt_raw, "stmt_raw"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"source"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/public"_ss, 1,
                    {
                        {(u0*) stmt_public, "stmt_public"_ss, type_decl::u32_, 1,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/private"_ss, 1,
                    {
                        {(u0*) stmt_private, "stmt_private"_ss, type_decl::u32_, 1,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/protected"_ss, 1,
                    {
                        {(u0*) stmt_protected, "stmt_protected"_ss, type_decl::u32_, 1,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/line-comment"_ss, 1,
                    {
                        {(u0*) stmt_comment_line, "stmt_comment_line"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/block-comment"_ss, 1,
                    {
                        {(u0*) stmt_comment_block, "stmt_comment_block"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },

                {"cxx/stmt/if"_ss, 1,
                    {
                        {(u0*) stmt_if, "stmt_if"_ss, type_decl::u32_, 5,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"true_id"_ss, type_decl::u32_},
                                {"false_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/for"_ss, 1,
                    {
                        {(u0*) stmt_for, "stmt_for"_ss, type_decl::u32_, 6,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"expr_id"_ss, type_decl::u32_},
                                {"init_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"post_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/do"_ss, 1,
                    {
                        {(u0*) stmt_do, "stmt_do"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/case"_ss, 1,
                    {
                        {(u0*) stmt_case, "stmt_case"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/while"_ss, 1,
                    {
                        {(u0*) stmt_while, "stmt_while"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/switch"_ss, 1,
                    {
                        {(u0*) stmt_switch, "stmt_switch"_ss, type_decl::u32_, 4,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"predicate_id"_ss, type_decl::u32_},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/expr"_ss, 1,
                    {
                        {(u0*) stmt_expr, "stmt_expr"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/decl"_ss, 1,
                    {
                        {(u0*) stmt_decl, "stmt_decl"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/goto"_ss, 1,
                    {
                        {(u0*) stmt_goto, "stmt_goto"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/return"_ss, 1,
                    {
                        {(u0*) stmt_goto, "stmt_goto"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/default"_ss, 1,
                    {
                        {(u0*) stmt_default, "stmt_default"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/define"_ss, 1,
                    {
                        {(u0*) stmt_def, "stmt_def"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/stmt/using"_ss, 1,
                    {
                        {(u0*) stmt_using, "stmt_using"_ss, type_decl::u32_, 3,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"ident_id"_ss, type_decl::u32_},
                                {"type_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/stmt/using-namespace"_ss, 1,
                    {
                        {(u0*) stmt_using_ns, "stmt_using_ns"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"expr_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },

                {"cxx/stmt/empty"_ss, 1,
                    {
                        {(u0*) stmt_empty, "stmt_empty"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/break"_ss, 1,
                    {
                        {(u0*) stmt_break, "stmt_break"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
                            }
                        }
                    }
                },

                {"cxx/stmt/continue"_ss, 1,
                    {
                        {(u0*) stmt_continue, "stmt_continue"_ss, type_decl::u32_, 2,
                            {
                                {"scope_usr"_ss, type_decl::obj_ptr},
                                {"label_id"_ss, type_decl::u32_, .default_value.dw = 0, .has_default = true},
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
