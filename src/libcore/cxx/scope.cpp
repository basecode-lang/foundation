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

#include <basecode/core/cxx/cxx.h>

namespace basecode::cxx::scope {
    static u32 make_unary(
            scope_t& scope,
            u32 lhs_id,
            unary_op_type_t type,
            position_type_t pos = position_type_t::none) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, lhs_id);
        auto type_value = MAKE_TYPE(expression_type_t::unary, type);
        switch (pos) {
            case position_type_t::prefix:   type_value = PREFIX(type_value); break;
            case position_type_t::postfix:  type_value = POSTFIX(type_value); break;
            default: break;
        }
        bass::write_field(c, element::field::type, type_value);
        return c.id;
    }

    static u32 make_aggregate(
            scope_t& scope,
            aggregate_type_t type,
            u32 block_id,
            u32 ident_id,
            u32 inheritance_list_id,
            u8 flags) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 6);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, flags);
        bass::write_field(c, element::field::rhs, inheritance_list_id);
        bass::write_field(c, element::field::ident, ident_id);
        bass::write_field(c, element::field::tbranch, block_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::aggregate, type));
        array::append(scope.types, c.id);
        return c.id;
    }

    static u32 make_integral_lit(
            scope_t& scope,
            meta_type_t meta_type,
            integral_size_t size,
            u64 lit,
            u32 radix = 10) {
        switch (size) {
            case integral_size_t::zero:
                break;
            case integral_size_t::byte:
                meta_type == meta_type_t::signed_integer ? format::to_radix(scope.pgm->scratch, s8(lit), radix) : format::to_radix(scope.pgm->scratch, u8(lit), radix);
                break;
            case integral_size_t::word:
                meta_type == meta_type_t::signed_integer ? format::to_radix(scope.pgm->scratch, s16(lit), radix) : format::to_radix(scope.pgm->scratch, u16(lit), radix);
                break;
            case integral_size_t::dword:
                meta_type == meta_type_t::signed_integer ? format::to_radix(scope.pgm->scratch, s32(lit), radix) : format::to_radix(scope.pgm->scratch, u32(lit), radix);
                break;
            case integral_size_t::qword:
                meta_type == meta_type_t::signed_integer ? format::to_radix(scope.pgm->scratch, s64(lit), radix) : format::to_radix(scope.pgm->scratch, u64(lit), radix);
                break;
        }
        auto r = string::interned::fold_for_result(scope.pgm->scratch);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::num_lit, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::radix, radix);
        bass::write_field(c, element::field::intern, r.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type, size));
        return c.id;
    }

    static u32 make_comment(scope_t& scope, u32 intern_id, b8 block = false) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::intern, intern_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(
            c,
            element::field::type,
            MAKE_TYPE(
                (block ? statement_type_t::block_comment : statement_type_t::line_comment),
                0));
        return c.id;
    }

    static u32 make_include(scope_t& scope, u32 intern_id, b8 local = false) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::intern, intern_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(
            c,
            element::field::type,
            MAKE_TYPE(
                statement_type_t::pp,
                (local ? preprocessor_type_t::local_include : preprocessor_type_t::system_include)));
        return c.id;
    }

    static u32 make_init(scope_t& scope, u32 expr_id, initializer_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 3);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(expression_type_t::initializer, type));
        return c.id;
    }

    static u32 make_binary(scope_t& scope, u32 lhs_id, u32 rhs_id, binary_op_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, lhs_id);
        bass::write_field(c, element::field::rhs, rhs_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(expression_type_t::binary, type));
        return c.id;
    }

    static u32 make_assign(scope_t& scope, u32 lhs_id, u32 rhs_id, assignment_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 4);
        bass::write_field(c, element::field::lhs, lhs_id);
        bass::write_field(c, element::field::rhs, rhs_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(expression_type_t::assignment, type));
        return c.id;
    }

    static u32 make_integral(scope_t& scope, u32 ident_id, meta_type_t meta_type, integral_size_t size) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::ident, ident_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type, size));
        array::append(scope.types, c.id);
        return c.id;
    }

    u0 pop(scope_t& scope) {
        stack::pop(scope.stack);
    }

    u0 free(scope_t& scope) {
        stack::free(scope.stack);
        array::free(scope.types);
        symtab::free(scope.labels);
        array::free(scope.children);
        array::free(scope.statements);
        symtab::free(scope.identifiers);
    }

    u32 push(scope_t& scope) {
        auto& module = program::get_module(*scope.pgm, scope.module_idx);
        auto& new_scope = array::append(module.scopes);
        new_scope.idx = module.scopes.size - 1;
        init(scope.pgm, &module, new_scope, &scope, scope.children.alloc);
        stack::push(scope.stack, new_scope.idx);
        return *stack::top(scope.stack);
    }

    u32 stmt::public_(scope_t& scope) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 2);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::public_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    status_t finalize(scope_t& scope) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::list, scope.statements.size + scope.children.size + 1);
        bass::write_field(c, element::field::parent, scope.id);
        for (auto id : scope.statements)
            bass::write_field(c, element::field::child, id);
        for (auto child : scope.children)
            bass::write_field(c, element::field::child, child);
        u32 value{};
        cursor_t scope_cursor{};
        if (!bass::seek_record(scope.pgm->storage, scope.id, scope_cursor))
            return status_t::scope_not_found;
        if (!bass::next_field(scope_cursor, value, element::field::list))
            return status_t::list_not_found;
        bass::write_field(scope_cursor, element::field::list, c.id);
        return status_t::ok;
    }

    u32 stmt::private_(scope_t& scope) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 2);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::private_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::protected_(scope_t& scope) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 2);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::protected_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 lit::chr(scope_t& scope, s8 lit) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::char_lit, 1);
        bass::write_field(c, element::field::lit, lit);
        return c.id;
    }

    u32 type::ref(scope_t& scope, u32 id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 3);
        bass::write_field(c, element::field::lhs, id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::reference, integral_size_t::qword));
        array::append(scope.types, c.id);
        return c.id;
    }

    u32 stmt::def(scope_t& scope, u32 expr_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::label, 0);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::definition, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 type::ptr(scope_t& scope, u32 type_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 3);
        bass::write_field(c, element::field::lhs, type_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::pointer, integral_size_t::qword));
        array::append(scope.types, c.id);
        return c.id;
    }

    u32 type::b8_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::boolean, integral_size_t::byte);
    }

    u32 type::u0_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::void_, integral_size_t::zero);
    }

    u32 type::s8_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::signed_integer, integral_size_t::byte);
    }

    u32 type::u8_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::unsigned_integer, integral_size_t::byte);
    }

    u32 type::s16_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::signed_integer, integral_size_t::word);
    }

    u32 type::s32_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::signed_integer, integral_size_t::dword);
    }

    u32 type::s64_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::signed_integer, integral_size_t::qword);
    }

    u32 type::u16_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::unsigned_integer, integral_size_t::word);
    }

    u32 type::u32_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::unsigned_integer, integral_size_t::dword);
    }

    u32 type::u64_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::unsigned_integer, integral_size_t::qword);
    }

    u32 type::f32_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::floating_point, integral_size_t::dword);
    }

    u32 type::f64_(scope_t& scope, u32 ident_id) {
        return make_integral(scope, ident_id, meta_type_t::floating_point, integral_size_t::qword);
    }

    u32 label(scope_t& scope, str::slice_t name) {
        ident_t ident{};
        if (symtab::find(scope.labels, name, ident))
            return ident.record_id;
        auto     r = string::interned::fold_for_result(name);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::label, 1);
        bass::write_field(c, element::field::intern, r.id);
        ident_t* new_ident{};
        if (symtab::emplace(scope.labels, name, &new_ident)) {
            new_ident->record_id = c.id;
            new_ident->intern_id = r.id;
        }
        return c.id;
    }

    u32 stmt::empty(scope_t& scope, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::empty, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 lit::str(scope_t& scope, str::slice_t lit) {
        auto r = string::interned::fold_for_result(lit);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::str_lit, 1);
        bass::write_field(c, element::field::intern, r.id);
        return c.id;
    }

    u32 stmt::break_(scope_t& scope, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::break_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::using_ns(scope_t& scope, u32 expr_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::using_ns_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::continue_(scope_t& scope, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::continue_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 expr::unary::neg(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::neg);
    }

    u32 expr::unary::bnot(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::bnot);
    }

    u32 expr::raw(scope_t& scope, str::slice_t source) {
        auto r = string::interned::fold_for_result(source);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::intern, r.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(expression_type_t::raw, 0));
        return c.id;
    }

    u32 stmt::raw(scope_t& scope, str::slice_t source) {
        auto r = string::interned::fold_for_result(source);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::intern, r.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::raw, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 expr::unary::lnot(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::lnot);
    }

    u32 expr::ident(scope_t& scope, str::slice_t name) {
        ident_t ident{};
        if (symtab::find(scope.identifiers, name, ident))
            return ident.record_id;
        auto r = string::interned::fold_for_result(name);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::ident, 2);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::intern, r.id);
        ident_t* new_ident{};
        if (symtab::emplace(scope.identifiers, name, &new_ident)) {
            new_ident->record_id = c.id;
            new_ident->intern_id = r.id;
        }
        return c.id;
    }

    u32 expr::unary::deref(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::deref);
    }

    u32 expr::init::direct(scope_t& scope, u32 expr_id) {
        return make_init(scope, expr_id, initializer_type_t::direct);
    }

    u32 expr::unary::addrof(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::addrof);
    }

    u32 type::array(scope_t& scope, u32 type_id, u32 size) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, type_id);
        bass::write_field(c, element::field::rhs, size);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::array, integral_size_t::qword));
        array::append(scope.types, c.id);
        return c.id;
    }

    u32 expr::list(scope_t& scope, u32 id_list[], u32 size) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::list, size + 1);
        bass::write_field(c, element::field::scope, scope.id);
        for (u32 i = 0; i < size; ++i)
            bass::write_field(c, element::field::child, id_list[i]);
        return c.id;
    }

    u32 stmt::pp::pragma(scope_t &scope, str::slice_t expr) {
        auto r = string::interned::fold_for_result(expr);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::intern, r.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::pp, preprocessor_type_t::pragma));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 type::bit_mask(scope_t& scope, u32 type_id, u8 bits) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 3);
        bass::write_field(c, element::field::lhs, type_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::bit_mask, bits));
        array::append(scope.types, c.id);
        return c.id;
    }

    u32 stmt::decl(scope_t& scope, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::decl, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::expr(scope_t& scope, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::expr, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 expr::unary::addrof_label(scope_t& scope, u32 expr_id) {
        return make_unary(scope, expr_id, unary_op_type_t::addrof_label);
    }

    u32 stmt::goto_(scope_t& scope, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::goto_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::comment::line(scope_t& scope, str::slice_t value) {
        auto r = string::interned::fold_for_result(value);
        auto id = make_comment(scope, r.id);
        array::append(scope.statements, id);
        return id;
    }

    u32 stmt::using_(scope_t& scope, u32 ident_id, u32 type_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, type_id);
        bass::write_field(c, element::field::rhs, ident_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::using_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::return_(scope_t& scope, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::return_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::default_(scope_t& scope, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::default_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::comment::block(scope_t& scope, str::slice_t value) {
        auto r = string::interned::fold_for_result(value);
        auto id = make_comment(scope, r.id, true);
        array::append(scope.statements, id);
        return id;
    }

    u32 expr::binary::eq(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::eq);
    }

    u32 expr::binary::lt(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::lt);
    }

    u32 expr::binary::gt(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::gt);
    }

    u32 expr::binary::mul(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::mul);
    }

    u32 expr::binary::neq(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::neq);
    }

    u32 expr::binary::lte(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::lte);
    }

    u32 expr::binary::gte(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::gte);
    }

    u32 expr::binary::add(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::add);
    }

    u32 expr::binary::sub(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::sub);
    }

    u32 expr::binary::div(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::div);
    }

    u32 expr::binary::mod(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::mod);
    }

    u32 expr::binary::shl(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::shl);
    }

    u32 expr::binary::shr(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::shr);
    }

    u32 expr::binary::bor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::bor);
    }

    u32 expr::binary::lor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::lor);
    }

    u32 expr::assign::sum(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::sum);
    }

    u32 expr::assign::shl(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::shl);
    }

    u32 expr::assign::shr(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::shr);
    }

    u32 expr::assign::bor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::bor);
    }

    u32 expr::binary::cast(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::cast);
    }

    u32 expr::binary::band(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::band);
    }

    u32 expr::binary::bxor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::bxor);
    }

    u32 expr::binary::land(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::land);
    }

    u32 expr::init::list(scope_t& scope, u32 expr_ids[], u32 size) {
        return make_init(scope, expr::list(scope, expr_ids, size), initializer_type_t::list);
    }

    u32 expr::assign::band(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::band);
    }

    u32 stmt::pp::include_local(scope_t &scope, str::slice_t path) {
        auto r = string::interned::fold_for_result(path);
        auto id = make_include(scope, r.id, true);
        array::append(scope.statements, id);
        return id;
    }

    u32 lit::float_(scope_t& scope, f64 lit, integral_size_t size) {
        str_buf_t buf(&scope.pgm->scratch);
        format::format_to(buf, "{}", lit);
        auto r = string::interned::fold_for_result(scope.pgm->scratch);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::num_lit, 3);
        bass::write_field(c, element::field::lit, r.id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::floating_point, size));
        return c.id;
    }

    u32 expr::assign::bxor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::bxor);
    }

    u32 expr::assign::diff(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::diff);
    }

    u32 expr::binary::comma(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::comma);
    }

    u32 expr::binary::range(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::range);
    }

    u32 stmt::pp::include_system(scope_t &scope, str::slice_t path) {
        auto r = string::interned::fold_for_result(path);
        auto id = make_include(scope, r.id);
        array::append(scope.statements, id);
        return id;
    }

    u32 expr::binary::member(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::member);
    }

    u32 expr::assign::direct(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::direct);
    }

    u32 expr::assign::product(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::product);
    }

    u32 expr::assign::quotient(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::quotient);
    }

    u32 expr::assign::remainder(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_assign(scope, lhs_id, rhs_id, assignment_type_t::remainder);
    }

    u32 expr::binary::scope_res(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::scope);
    }

    u32 expr::binary::subscript(scope_t& scope, u32 lhs_id, u32 rhs_id) {
        return make_binary(scope, lhs_id, rhs_id, binary_op_type_t::subscript);
    }

    u32 type::enum_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
        return make_aggregate(scope, aggregate_type_t::enum_, block_id, ident_id, 0, flags);
    }

    u32 expr::unary::inc(scope_t& scope, u32 expr_id, position_type_t pos) {
        return make_unary(scope, expr_id, unary_op_type_t::inc, pos);
    }

    u32 type::class_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
        return make_aggregate(scope, aggregate_type_t::class_, block_id, ident_id, 0, flags);
    }

    u32 type::union_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
        return make_aggregate(scope, aggregate_type_t::union_, block_id, ident_id, 0, flags);
    }

    u32 expr::unary::dec(scope_t& scope, u32 expr_id, position_type_t pos) {
        return make_unary(scope, expr_id, unary_op_type_t::dec, pos);
    }

    u32 type::struct_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
        return make_aggregate(scope, aggregate_type_t::struct_, block_id, ident_id, 0, flags);
    }

    u32 lit::signed_(scope_t& scope, u64 lit, integral_size_t size, u32 radix) {
        return make_integral_lit(scope, meta_type_t::signed_integer, size, lit, radix);
    }

    u32 stmt::do_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 5);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::do_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 type::enum_class_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
        return make_aggregate(scope, aggregate_type_t::enum_class, block_id, ident_id, 0, flags);
    }

    u32 lit::unsigned_(scope_t& scope, u64 lit, integral_size_t size, u32 radix) {
        return make_integral_lit(scope, meta_type_t::unsigned_integer, size, lit, radix);
    }

    u32 stmt::case_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 5);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::case_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::while_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 5);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::while_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 stmt::switch_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 5);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::switch_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 expr::var(scope_t& scope, u32 type_id, u32 ident_id, u32 init_expr_id, u8 flags) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::variable, 5);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, type_id);
        bass::write_field(c, element::field::rhs, ident_id);
        bass::write_field(c, element::field::init, init_expr_id);
        bass::write_field(c, element::field::type, flags);
        return c.id;
    }

    u0 init(program_t* pgm, module_t* module, scope_t& scope, scope_t* parent, alloc_t* alloc) {
        cursor_t c{};
        bass::seek_current(pgm->storage, c);
        bass::new_record(c, element::header::scope, 3);
        scope.id = c.id;
        scope.pgm = pgm;
        scope.module_idx = module->idx;
        if (parent) {
            bass::write_field(c, element::field::scope, parent->id);
            bass::write_field(c, element::field::parent, parent->id);
            array::append(parent->children, scope.id);
            scope.parent_idx = parent->idx;
        } else {
            bass::write_field(c, element::field::scope, 0);
            bass::write_field(c, element::field::parent, module->id);
            scope.parent_idx = 0;
        }
        bass::write_field(c, element::field::list, 0);
        stack::init(scope.stack, alloc);
        array::init(scope.types, alloc);
        symtab::init(scope.labels, alloc);
        array::init(scope.children, alloc);
        array::init(scope.statements, alloc);
        symtab::init(scope.identifiers, alloc);
    }


    u32 stmt::if_(scope_t& scope, u32 predicate_id, u32 true_expr_id, u32 false_expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 6);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::tbranch, true_expr_id);
        bass::write_field(c, element::field::fbranch, false_expr_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::if_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }

    u32 type::func(scope_t& scope, u32 block_id, u32 return_type_id, u32 ident_id, u32 params_list_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 6);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::ident, ident_id);
        bass::write_field(c, element::field::lhs, return_type_id);
        bass::write_field(c, element::field::rhs, params_list_id);
        bass::write_field(c, element::field::tbranch, block_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(meta_type_t::function, integral_size_t::qword));
        return c.id;
    }

    u32 stmt::for_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 init_expr_id, u32 post_expr_id, u32 label_id) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 7);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::label, label_id);
        bass::write_field(c, element::field::lhs, predicate_id);
        bass::write_field(c, element::field::rhs, post_expr_id);
        bass::write_field(c, element::field::init, init_expr_id);
        bass::write_field(c, element::field::tbranch, expr_id);
        bass::write_field(c, element::field::type, MAKE_TYPE(statement_type_t::for_, 0));
        array::append(scope.statements, c.id);
        return c.id;
    }
}
