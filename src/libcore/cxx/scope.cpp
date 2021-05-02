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

#include <basecode/core/string.h>
#include <basecode/core/cxx/cxx.h>

namespace basecode::cxx::scope {
    static u32 make_init(scope_t& scope,
                         u32 expr_id,
                         initializer_type_t type);

    static u32 make_assign(scope_t& scope,
                           u32 lhs_id,
                           u32 rhs_id,
                           assignment_type_t type);

    static u32 make_binary(scope_t& scope,
                           u32 lhs_id,
                           u32 rhs_id,
                           binary_op_type_t type);

    static u32 make_integral(scope_t& scope,
                             u32 ident_id,
                             meta_type_t meta_type,
                             integral_size_t size);

    static u32 make_unary(scope_t& scope,
                          u32 lhs_id,
                          unary_op_type_t type,
                          position_type_t pos = position_type_t::none);

    static u32 make_integral_lit(scope_t& scope,
                                 meta_type_t meta_type,
                                 integral_size_t size,
                                 u64 lit,
                                 u32 radix = 10);

    static u32 make_aggregate(scope_t& scope,
                              aggregate_type_t type,
                              u32 block_id,
                              u32 ident_id,
                              u32 inheritance_list_id,
                              u8 flags);

    static u32 make_include(scope_t& scope, u32 intern_id, b8 local = false);

    static u32 make_comment(scope_t& scope, u32 intern_id, b8 block = false);

    namespace lit {
        u32 unsigned_(scope_t& scope,
                      u64 lit,
                      integral_size_t size,
                      u32 radix) {
            return make_integral_lit(scope,
                                     meta_type_t::unsigned_integer,
                                     size,
                                     lit,
                                     radix);
        }

        u32 chr(scope_t& scope, s8 lit) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::char_lit, 1);
            bass::write_field(c, element::field::lit, lit);
            return c.id;
        }

        u32 str(scope_t& scope, str::slice_t lit) {
            auto r = string::interned::fold_for_result(lit);
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::str_lit, 1);
            bass::write_field(c, element::field::intern, r.id);
            return c.id;
        }

        u32 float_(scope_t& scope, f64 lit, integral_size_t size) {
            str_t scratch{};
            str::init(scratch, context::top()->alloc.temp); {
                str_buf_t buf(&scratch);
                format::format_to(buf, "{}", lit);
            }
            auto r = string::interned::fold_for_result(scratch);
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::num_lit, 3);
            bass::write_field(c, element::field::lit, r.id);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::floating_point,
                                        size));
            return c.id;
        }

        u32 signed_(scope_t& scope, u64 lit, integral_size_t size, u32 radix) {
            return make_integral_lit(scope,
                                     meta_type_t::signed_integer,
                                     size,
                                     lit,
                                     radix);
        }
    }

    namespace expr {
        namespace init {
            u32 direct(scope_t& scope, u32 expr_id) {
                return make_init(scope, expr_id,
                                 initializer_type_t::direct);
            }

            u32 list(scope_t& scope, u32 expr_ids[], u32 size) {
                return make_init(scope,
                                 expr::list(scope, expr_ids, size),
                                 initializer_type_t::list);
            }
        }

        namespace unary {
            u32 neg(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::neg);
            }

            u32 bnot(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::bnot);
            }

            u32 lnot(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::lnot);
            }

            u32 deref(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::deref);
            }

            u32 addrof(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::addrof);
            }

            u32 addrof_label(scope_t& scope, u32 expr_id) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::addrof_label);
            }

            u32 inc(scope_t& scope, u32 expr_id, position_type_t pos) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::inc, pos);
            }

            u32 dec(scope_t& scope, u32 expr_id, position_type_t pos) {
                return make_unary(scope,
                                  expr_id,
                                  unary_op_type_t::dec, pos);
            }
        }

        namespace binary {
            u32 comma(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::comma);
            }

            u32 range(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::range);
            }

            u32 eq(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::eq);
            }

            u32 lt(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::lt);
            }

            u32 gt(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::gt);
            }

            u32 mul(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::mul);
            }

            u32 neq(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::neq);
            }

            u32 lte(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::lte);
            }

            u32 gte(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::gte);
            }

            u32 add(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::add);
            }

            u32 sub(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::sub);
            }

            u32 div(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::div);
            }

            u32 mod(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::mod);
            }

            u32 shl(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::shl);
            }

            u32 shr(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::shr);
            }

            u32 bor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::bor);
            }

            u32 lor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::lor);
            }

            u32 bxor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::bxor);
            }

            u32 land(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::land);
            }

            u32 band(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::band);
            }

            u32 cast(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::cast);
            }

            u32 member(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::member);
            }

            u32 scope_res(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::scope);
            }

            u32 subscript(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_binary(scope,
                                   lhs_id,
                                   rhs_id,
                                   binary_op_type_t::subscript);
            }
        }

        namespace assign {
            u32 sum(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::sum);
            }

            u32 shr(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::shr);
            }

            u32 bor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::bor);
            }

            u32 shl(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::shl);
            }

            u32 diff(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::diff);
            }

            u32 bxor(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::bxor);
            }

            u32 band(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::band);
            }

            u32 direct(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::direct);
            }

            u32 product(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::product);
            }

            u32 quotient(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::quotient);
            }

            u32 remainder(scope_t& scope, u32 lhs_id, u32 rhs_id) {
                return make_assign(scope,
                                   lhs_id,
                                   rhs_id,
                                   assignment_type_t::remainder);
            }
        }

        u32 var(scope_t& scope,
                u32 type_id,
                u32 ident_id,
                u32 init_expr_id,
                u8 flags) {
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

        u32 ident(scope_t& scope, str::slice_t name) {
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


        u32 raw(scope_t& scope, str::slice_t source) {
            auto r = string::interned::fold_for_result(source);
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::expression, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::intern, r.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(expression_type_t::raw, 0));
            return c.id;
        }

        u32 list(scope_t& scope, u32 id_list[], u32 size) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::list, size + 1);
            bass::write_field(c, element::field::scope, scope.id);
            for (u32 i = 0; i < size; ++i)
                bass::write_field(c, element::field::child, id_list[i]);
            return c.id;
        }
    }

    namespace type {
        u32 func(scope_t& scope,
                 u32 block_id,
                 u32 return_type_id,
                 u32 ident_id,
                 u32 params_list_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::type, 6);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::ident, ident_id);
            bass::write_field(c, element::field::lhs, return_type_id);
            bass::write_field(c, element::field::rhs, params_list_id);
            bass::write_field(c, element::field::tbranch, block_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::function,
                                        integral_size_t::qword));
            return c.id;
        }

        u32 ref(scope_t& scope, u32 id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::type, 3);
            bass::write_field(c, element::field::lhs, id);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::reference,
                                        integral_size_t::qword));
            array::append(scope.types, c.id);
            return c.id;
        }

        u32 ptr(scope_t& scope, u32 type_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::type, 3);
            bass::write_field(c, element::field::lhs, type_id);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::pointer,
                                        integral_size_t::qword));
            array::append(scope.types, c.id);
            return c.id;
        }

        u32 b8_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::boolean,
                                 integral_size_t::byte);
        }

        u32 u0_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::void_,
                                 integral_size_t::zero);
        }

        u32 s8_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::signed_integer,
                                 integral_size_t::byte);
        }

        u32 u8_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::unsigned_integer,
                                 integral_size_t::byte);
        }

        u32 s16_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::signed_integer,
                                 integral_size_t::word);
        }

        u32 s32_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::signed_integer,
                                 integral_size_t::dword);
        }

        u32 s64_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::signed_integer,
                                 integral_size_t::qword);
        }

        u32 u16_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::unsigned_integer,
                                 integral_size_t::word);
        }

        u32 u32_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::unsigned_integer,
                                 integral_size_t::dword);
        }

        u32 u64_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::unsigned_integer,
                                 integral_size_t::qword);
        }

        u32 f32_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::floating_point,
                                 integral_size_t::dword);
        }

        u32 f64_(scope_t& scope, u32 ident_id) {
            return make_integral(scope,
                                 ident_id,
                                 meta_type_t::floating_point,
                                 integral_size_t::qword);
        }

        u32 array(scope_t& scope, u32 type_id, u32 size) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::type, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, type_id);
            bass::write_field(c, element::field::rhs, size);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::array,
                                        integral_size_t::qword));
            array::append(scope.types, c.id);
            return c.id;
        }

        u32 bit_mask(scope_t& scope, u32 type_id, u8 bits) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::type, 3);
            bass::write_field(c, element::field::lhs, type_id);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(meta_type_t::bit_mask, bits));
            array::append(scope.types, c.id);
            return c.id;
        }

        u32 enum_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
            return make_aggregate(scope,
                                  aggregate_type_t::enum_,
                                  block_id,
                                  ident_id,
                                  0,
                                  flags);
        }

        u32 union_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
            return make_aggregate(scope,
                                  aggregate_type_t::union_,
                                  block_id,
                                  ident_id,
                                  0,
                                  flags);
        }

        u32 class_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
            return make_aggregate(scope,
                                  aggregate_type_t::class_,
                                  block_id,
                                  ident_id,
                                  0,
                                  flags);
        }

        u32 struct_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
            return make_aggregate(scope,
                                  aggregate_type_t::struct_,
                                  block_id,
                                  ident_id,
                                  0,
                                  flags);
        }

        u32 enum_class_(scope_t& scope, u32 block_id, u32 ident_id, u8 flags) {
            return make_aggregate(scope,
                                  aggregate_type_t::enum_class,
                                  block_id,
                                  ident_id,
                                  0,
                                  flags);
        }
    }

    namespace stmt {
        namespace pp {
            u32 pragma(scope_t &scope, str::slice_t expr) {
                auto r = string::interned::fold_for_result(expr);
                cursor_t c{};
                bass::seek_current(scope.pgm->storage, c);
                bass::new_record(c, element::header::statement, 3);
                bass::write_field(c, element::field::scope, scope.id);
                bass::write_field(c, element::field::intern, r.id);
                bass::write_field(c,
                                  element::field::type,
                                  MAKE_TYPE(statement_type_t::pp,
                                            preprocessor_type_t::pragma));
                array::append(scope.statements, c.id);
                return c.id;
            }

            u32 include_local(scope_t &scope, str::slice_t path) {
                auto r = string::interned::fold_for_result(path);
                auto id = make_include(scope, r.id, true);
                array::append(scope.statements, id);
                return id;
            }

            u32 include_system(scope_t &scope, str::slice_t path) {
                auto r = string::interned::fold_for_result(path);
                auto id = make_include(scope, r.id);
                array::append(scope.statements, id);
                return id;
            }
        }

        namespace comment {
            u32 line(scope_t& scope, str::slice_t value) {
                auto r = string::interned::fold_for_result(value);
                auto id = make_comment(scope, r.id);
                array::append(scope.statements, id);
                return id;
            }

            u32 block(scope_t& scope, str::slice_t value) {
                auto r = string::interned::fold_for_result(value);
                auto id = make_comment(scope, r.id, true);
                array::append(scope.statements, id);
                return id;
            }
        }

        u32 if_(scope_t& scope,
                u32 predicate_id,
                u32 true_expr_id,
                u32 false_expr_id,
                u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 6);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c, element::field::tbranch, true_expr_id);
            bass::write_field(c, element::field::fbranch, false_expr_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::if_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 for_(scope_t& scope,
                 u32 predicate_id,
                 u32 expr_id,
                 u32 init_expr_id,
                 u32 post_expr_id,
                 u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 7);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::rhs, post_expr_id);
            bass::write_field(c, element::field::init, init_expr_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::for_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 switch_(scope_t& scope,
                    u32 predicate_id,
                    u32 expr_id,
                    u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 5);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::switch_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 public_(scope_t& scope) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 2);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::public_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 private_(scope_t& scope) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 2);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::private_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 protected_(scope_t& scope) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 2);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::protected_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 empty(scope_t& scope, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::empty, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 def(scope_t& scope, u32 expr_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c, element::field::label, 0);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::definition, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 break_(scope_t& scope, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::break_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 using_ns(scope_t& scope, u32 expr_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::using_ns_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 continue_(scope_t& scope, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::continue_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 raw(scope_t& scope, str::slice_t source) {
            auto r = string::interned::fold_for_result(source);
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 3);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::intern, r.id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::raw, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 decl(scope_t& scope, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::decl, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 expr(scope_t& scope, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::expr, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 goto_(scope_t& scope, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::goto_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 using_(scope_t& scope, u32 ident_id, u32 type_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, type_id);
            bass::write_field(c, element::field::rhs, ident_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::using_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 return_(scope_t& scope, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::return_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 default_(scope_t& scope, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 4);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::default_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 do_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 5);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::do_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 case_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 5);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::case_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }

        u32 while_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id) {
            cursor_t c{};
            bass::seek_current(scope.pgm->storage, c);
            bass::new_record(c, element::header::statement, 5);
            bass::write_field(c, element::field::scope, scope.id);
            bass::write_field(c, element::field::lhs, predicate_id);
            bass::write_field(c, element::field::tbranch, expr_id);
            bass::write_field(c, element::field::label, label_id);
            bass::write_field(c,
                              element::field::type,
                              MAKE_TYPE(statement_type_t::while_, 0));
            array::append(scope.statements, c.id);
            return c.id;
        }
    }

    u0 pop(scope_t& scope) {
        stack::pop(scope.stack);
    }

    u0 init(program_t* pgm,
            module_t* module,
            scope_t& scope,
            scope_t* parent,
            alloc_t* alloc) {
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

    status_t finalize(scope_t& scope) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c,
                         element::header::list,
                         scope.statements.size + scope.children.size + 1);
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

    static u32 make_aggregate(scope_t& scope,
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
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(meta_type_t::aggregate, type));
        array::append(scope.types, c.id);
        return c.id;
    }

    static u32 make_integral_lit(scope_t& scope,
                                 meta_type_t meta_type,
                                 integral_size_t size,
                                 u64 lit,
                                 u32 radix) {
        str_t scratch{};
        str::init(scratch, context::top()->alloc.temp);
        switch (size) {
            case integral_size_t::zero:
                break;
            case integral_size_t::byte:
                meta_type == meta_type_t::signed_integer ?
                    format::to_radix(scratch, s8(lit), radix) :
                    format::to_radix(scratch, u8(lit), radix);
                break;
            case integral_size_t::word:
                meta_type == meta_type_t::signed_integer ?
                    format::to_radix(scratch, s16(lit), radix) :
                    format::to_radix(scratch, u16(lit), radix);
                break;
            case integral_size_t::dword:
                meta_type == meta_type_t::signed_integer ?
                    format::to_radix(scratch, s32(lit), radix) :
                    format::to_radix(scratch, u32(lit), radix);
                break;
            case integral_size_t::qword:
                meta_type == meta_type_t::signed_integer ?
                    format::to_radix(scratch, s64(lit), radix) :
                    format::to_radix(scratch, u64(lit), radix);
                break;
        }
        auto r = string::interned::fold_for_result(scratch);
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::num_lit, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::radix, radix);
        bass::write_field(c, element::field::intern, r.id);
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(meta_type, size));
        return c.id;
    }

    static u32 make_unary(scope_t& scope,
                          u32 lhs_id,
                          unary_op_type_t type,
                          position_type_t pos) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, lhs_id);
        auto type_value = MAKE_TYPE(expression_type_t::unary, type);
        switch (pos) {
            case position_type_t::prefix:
                type_value = PREFIX(type_value);
                break;
            case position_type_t::postfix:
                type_value = POSTFIX(type_value);
                break;
            default:
                break;
        }
        bass::write_field(c, element::field::type, type_value);
        return c.id;
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

    static u32 make_init(scope_t& scope,
                         u32 expr_id,
                         initializer_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 3);
        bass::write_field(c, element::field::lhs, expr_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(expression_type_t::initializer, type));
        return c.id;
    }

    static u32 make_binary(scope_t& scope,
                           u32 lhs_id,
                           u32 rhs_id,
                           binary_op_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 4);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::lhs, lhs_id);
        bass::write_field(c, element::field::rhs, rhs_id);
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(expression_type_t::binary, type));
        return c.id;
    }

    static u32 make_integral(scope_t& scope,
                             u32 ident_id,
                             meta_type_t meta_type,
                             integral_size_t size) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::type, 3);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c, element::field::ident, ident_id);
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(meta_type, size));
        array::append(scope.types, c.id);
        return c.id;
    }

    static u32 make_assign(scope_t& scope,
                           u32 lhs_id,
                           u32 rhs_id,
                           assignment_type_t type) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::expression, 4);
        bass::write_field(c, element::field::lhs, lhs_id);
        bass::write_field(c, element::field::rhs, rhs_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(c,
                          element::field::type,
                          MAKE_TYPE(expression_type_t::assignment, type));
        return c.id;
    }

    static u32 make_comment(scope_t& scope, u32 intern_id, b8 block) {
        cursor_t c{};
        bass::seek_current(scope.pgm->storage, c);
        bass::new_record(c, element::header::statement, 3);
        bass::write_field(c, element::field::intern, intern_id);
        bass::write_field(c, element::field::scope, scope.id);
        bass::write_field(
            c,
            element::field::type,
            MAKE_TYPE(
                (block ? statement_type_t::block_comment :
                 statement_type_t::line_comment),
                0));
        return c.id;
    }

    static u32 make_include(scope_t& scope, u32 intern_id, b8 local) {
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
                (local ? preprocessor_type_t::local_include :
                 preprocessor_type_t::system_include)));
        return c.id;
    }
}
