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

#include "../memory/proxy_system.h"
#include "cxx.h"

namespace basecode::cxx::serializer {
    static string::slice_t s_var_flags[] = {
        "const"_ss,
        "static"_ss,
        "volatile"_ss,
        "register"_ss,
        "constexpr"_ss,
    };

    static u0 newline(serializer_t& s, format::memory_buffer_t& buf);

    static status_t decl_var(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor);

    static status_t process_list(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor);

    static status_t process_expr(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor);

    static status_t process_stmt(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor);

    static status_t process_scope(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor);

    static u0 tab(serializer_t& s) {
        s.indent += s.tab_width - (s.column % s.tab_width);
    }

    static u0 untab(serializer_t& s) {
        s.indent -= s.tab_width - (s.column % s.tab_width);
    }

    static u0 newline(serializer_t& s, format::memory_buffer_t& buf) {
        ++s.line;
        s.column = {};
        format::format_to(buf, "\n");
    }

    template <typename... Args>
    static u0 at_indent(serializer_t& s, format::memory_buffer_t& buf, fmt::string_view format_str, const Args&... args) {
        if (likely(s.column == 0 && s.indent > 0))
            format::format_to(buf, "{:<{}}", " ", s.indent);
        auto start = buf.size();
        format::format_to(buf, format_str, args...);
        s.column += buf.size() - start;
        if (s.column > s.margin)
            newline(s, buf);
    }

    static u0 process_var_flags(serializer_t& s, format::memory_buffer_t& buf, u8 flags) {
        u8 mask = 1;
        for (u32 i = 0; i < 8; ++i) {
            if ((flags & mask) == mask)
                at_indent(s, buf, "{} ", s_var_flags[i]);
            mask <<= (u32) 1;
        }
    }

    static status_t decl_var(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        auto& intern = *s.intern;
        auto& store = *s.store;

        auto dict = bass::dict::make(cursor);
        process_var_flags(s, buf, bass::dict::get(dict, element::field::type));
        type_info_t type_info{};
        type_info.name = &s.scratch;
        expand_type(store, intern, bass::dict::get(dict, element::field::lhs), type_info);
        at_indent(s, buf, "{} ", s.scratch);
        auto rhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::rhs));
        if (unlikely(!rhs_cursor.ok))
            return status_t::rhs_not_found;
        auto status = process_expr(s, buf, rhs_cursor);
        if (unlikely(!OK(status)))
            return status;
        switch (type_info.meta_type) {
            case meta_type_t::array: {
                at_indent(s, buf, "[");
                if (likely(type_info.size))
                    at_indent(s, buf, "{}", type_info.size);
                at_indent(s, buf, "] ");
                break;
            }
            case meta_type_t::bit_mask: {
                at_indent(s, buf, ":{}", type_info.size);
                break;
            }
            default: break;
        }
        auto init_id = bass::dict::get(dict, element::field::init);
        if (init_id) {
            auto init_cursor = bass::get_header(store, init_id);
            if (unlikely(!init_cursor.ok))
                return status_t::element_not_found;
            status = process_expr(s, buf, init_cursor);
        }
        return status;
    }

    static status_t process_list(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        u32 id{};
        b8 first = true;
        auto& store = *s.store;
        while (bass::next_field(cursor, id, element::field::child)) {
            auto element_cursor = bass::get_header(store, id);
            if (unlikely(!element_cursor.ok))
                return status_t::element_not_found;
            switch (element_cursor.header->type) {
                case element::header::variable: {
                    if (!first) at_indent(s, buf, ", ");
                    auto status = decl_var(s, buf, element_cursor);
                    if (unlikely(!OK(status)))
                        return status;
                    break;
                }
                case element::header::statement: {
                    auto status = process_stmt(s, buf, element_cursor);
                    if (unlikely(!OK(status)))
                        return status;
                    break;
                }
                case element::header::expression: {
                    if (!first) at_indent(s, buf, ", ");
                    auto status = process_expr(s, buf, element_cursor);
                    if (unlikely(!OK(status)))
                        return status;
                    break;
                }
                case element::header::scope:    break;
                default:                        return status_t::invalid_list_entry;
            }
            first = false;
        }
        return status_t::ok;
    }

    static status_t process_expr(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        status_t status{};
        auto& intern = *s.intern;
        auto& store = *s.store;
        auto dict = bass::dict::make(cursor);
        switch (cursor.header->type) {
            case element::header::type: {
                auto ident_cursor = bass::get_header(store, bass::dict::get(dict, element::field::ident));
                return process_expr(s, buf, ident_cursor);
            }
            case element::header::ident:
            case element::header::num_lit: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                at_indent(s, buf, "{}", interned.slice);
                return status_t::ok;
            }
            case element::header::str_lit: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                at_indent(s, buf, "\"{}\"", interned.slice);
                return status_t::ok;
            }
            case element::header::char_lit: {
                at_indent(
                    s,
                    buf,
                    "'{}'",
                    (s8) bass::dict::get(dict, element::field::lit));
                return status_t::ok;
            }
            case element::header::variable: {
                auto ident_cursor = bass::get_header(store, bass::dict::get(dict, element::field::rhs));
                return process_expr(s, buf, ident_cursor);
            }
            case element::header::scope:        return process_scope(s, buf, cursor);
            case element::header::statement:    return process_stmt(s, buf, cursor);
            case element::header::expression:   break;
            default:                            return status_t::invalid_expr_element;
        }
        auto type_field = bass::dict::get(dict, element::field::type);
        auto type = (expression_type_t) BASE_TYPE(type_field);
        auto lhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
        if (unlikely(!lhs_cursor.ok))
            return status_t::lhs_not_found;
        switch (type) {
            case expression_type_t::raw: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                if (unlikely(!OK(interned.status)))
                    return status_t::intern_not_found;
                at_indent(s, buf, "{}", interned.slice);
                break;
            }
            case expression_type_t::unary: {
                auto op_type = (unary_op_type_t) SUB_TYPE(type_field);
                switch (op_type) {
                    case unary_op_type_t::neg:
                        at_indent(s, buf, "-");
                        break;
                    case unary_op_type_t::inc: {
                        auto pos_type = (position_type_t) POS_TYPE(type_field);
                        switch (pos_type) {
                            case position_type_t::none:
                                return status_t::invalid_pos_type;
                            case position_type_t::prefix: {
                                at_indent(s, buf, "++");
                                status = process_expr(s, buf, lhs_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                break;
                            }
                            case position_type_t::postfix: {
                                status = process_expr(s, buf, lhs_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                at_indent(s, buf, "++");
                                break;
                            }
                        }
                        break;
                    }
                    case unary_op_type_t::dec: {
                        auto pos_type = (position_type_t) POS_TYPE(type_field);
                        switch (pos_type) {
                            case position_type_t::none:
                                return status_t::invalid_pos_type;
                            case position_type_t::prefix: {
                                at_indent(s, buf, "--");
                                status = process_expr(s, buf, lhs_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                break;
                            }
                            case position_type_t::postfix: {
                                status = process_expr(s, buf, lhs_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                at_indent(s, buf, "--");
                                break;
                            }
                        }
                        break;
                    }
                    case unary_op_type_t::lnot:
                        at_indent(s, buf, "!");
                        break;
                    case unary_op_type_t::bnot:
                        at_indent(s, buf, "~");
                        break;
                    case unary_op_type_t::deref:
                        at_indent(s, buf, "*");
                        break;
                    case unary_op_type_t::addrof:
                        at_indent(s, buf, "&");
                        break;
                    case unary_op_type_t::addrof_label:
                        at_indent(s, buf, "&&");
                        break;
                }
                break;
            }
            case expression_type_t::binary: {
                auto lhs_dict = bass::dict::make(lhs_cursor);

                status = process_expr(s, buf, lhs_cursor);
                if (unlikely(!OK(status)))
                    return status;

                auto rhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::rhs));
                if (unlikely(!rhs_cursor.ok))
                    return status_t::rhs_not_found;

                auto op_type = (binary_op_type_t) SUB_TYPE(type_field);
                switch (op_type) {
                    case binary_op_type_t::eq:
                        at_indent(s, buf, " == ");
                        goto bin_op_done;
                    case binary_op_type_t::lt:
                        at_indent(s, buf, " < ");
                        goto bin_op_done;
                    case binary_op_type_t::gt:
                        at_indent(s, buf, " > ");
                        goto bin_op_done;
                    case binary_op_type_t::lor:
                        at_indent(s, buf, " || ");
                        goto bin_op_done;
                    case binary_op_type_t::add:
                        at_indent(s, buf, " + ");
                        goto bin_op_done;
                    case binary_op_type_t::sub:
                        at_indent(s, buf, " - ");
                        goto bin_op_done;
                    case binary_op_type_t::mul:
                        at_indent(s, buf, " * ");
                        goto bin_op_done;
                    case binary_op_type_t::div:
                        at_indent(s, buf, " / ");
                        goto bin_op_done;
                    case binary_op_type_t::mod:
                        at_indent(s, buf, " % ");
                        goto bin_op_done;
                    case binary_op_type_t::shl:
                        at_indent(s, buf, " << ");
                        goto bin_op_done;
                    case binary_op_type_t::shr:
                        at_indent(s, buf, " >> ");
                        goto bin_op_done;
                    case binary_op_type_t::bor:
                        at_indent(s, buf, " | ");
                        goto bin_op_done;
                    case binary_op_type_t::neq:
                        at_indent(s, buf, " != ");
                        goto bin_op_done;
                    case binary_op_type_t::lte:
                        at_indent(s, buf, " <= ");
                        goto bin_op_done;
                    case binary_op_type_t::gte:
                        at_indent(s, buf, " >= ");
                        goto bin_op_done;
                    case binary_op_type_t::band:
                        at_indent(s, buf, " & ");
                        goto bin_op_done;
                    case binary_op_type_t::bxor:
                        at_indent(s, buf, " ^ ");
                        goto bin_op_done;
                    case binary_op_type_t::land:
                        at_indent(s, buf, " && ");
                        goto bin_op_done;
                    case binary_op_type_t::comma:
                        at_indent(s, buf, ", ");
                        goto bin_op_done;
                    case binary_op_type_t::scope:
                        at_indent(s, buf, "::");
                        goto bin_op_done;
                    case binary_op_type_t::range:
                        at_indent(s, buf, " : ");
                        goto bin_op_done;
                    case binary_op_type_t::member: {
                        // XXX: REFACTOR THIS PILE OF SHIT
                        //      The majority of this logic should move to the var function when
                        //      the element is created.  Either a mask could be added to field::type
                        //      -or- a new field type created that encodes what type of member access, if any,
                        //      would be valid for the variable.
                        if (lhs_cursor.header->type != element::header::variable)
                            return status_t::error;
                        auto type_cursor = bass::get_header(store, bass::dict::get(lhs_dict, element::field::lhs));
                        u32 type_value{};
                        if (!type_cursor.ok
                        ||  !bass::next_field(type_cursor, type_value, element::field::type)) {
                            return status_t::error;
                        }
                        auto meta_type = (meta_type_t) BASE_TYPE(type_value);
                        switch (meta_type) {
                            case meta_type_t::alias: {
                                // XXX: need to find base
                                return status_t::error;
                            }
                            case meta_type_t::none:
                            case meta_type_t::void_:
                            case meta_type_t::array:
                            case meta_type_t::boolean:
                            case meta_type_t::bit_mask:
                            case meta_type_t::function:
                            case meta_type_t::signed_integer:
                            case meta_type_t::floating_point:
                            case meta_type_t::unsigned_integer:
                                return status_t::error;
                            case meta_type_t::pointer:
                                at_indent(s, buf, "->");
                                break;
                            case meta_type_t::aggregate:
                            case meta_type_t::reference:
                                at_indent(s, buf, ".");
                                break;
                        }
                        goto bin_op_done;
                    }
                    case binary_op_type_t::cast:
                        at_indent(s, buf, "(");
                        status = process_expr(s, buf, rhs_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        at_indent(s, buf, ")");
                        break;
                    case binary_op_type_t::subscript:
                        at_indent(s, buf, "[");
                        status = process_expr(s, buf, rhs_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        at_indent(s, buf, "]");
                        break;
                }

                break;

            bin_op_done:
                status = process_expr(s, buf, rhs_cursor);
                if (unlikely(!OK(status)))
                    return status;
                break;
            }
            case expression_type_t::assignment: {
                auto rhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::rhs));
                if (unlikely(!rhs_cursor.ok))
                    return status_t::rhs_not_found;

                status = process_expr(s, buf, lhs_cursor);
                if (unlikely(!OK(status)))
                    return status;

                auto op_type = (assignment_type_t) SUB_TYPE(type_field);
                switch (op_type) {
                    case assignment_type_t::direct:
                        at_indent(s, buf, " = ");
                        break;
                    case assignment_type_t::bor:
                        at_indent(s, buf, " |= ");
                        break;
                    case assignment_type_t::sum:
                        at_indent(s, buf, " += ");
                        break;
                    case assignment_type_t::shl:
                        at_indent(s, buf, " <<= ");
                        break;
                    case assignment_type_t::shr:
                        at_indent(s, buf, " >>= ");
                        break;
                    case assignment_type_t::diff:
                        at_indent(s, buf, " -= ");
                        break;
                    case assignment_type_t::band:
                        at_indent(s, buf, " &= ");
                        break;
                    case assignment_type_t::bxor:
                        at_indent(s, buf, " ^= ");
                        break;
                    case assignment_type_t::product:
                        at_indent(s, buf, " *= ");
                        break;
                    case assignment_type_t::quotient:
                        at_indent(s, buf, " /= ");
                        break;
                    case assignment_type_t::remainder:
                        at_indent(s, buf, " %= ");
                        break;
                }

                status = process_expr(s, buf, rhs_cursor);
                break;
            }
            case expression_type_t::initializer: {
                auto op_type = (initializer_type_t) SUB_TYPE(type_field);
                switch (op_type) {
                    case initializer_type_t::list:
                        at_indent(s, buf, "{{");
                        status = process_list(s, buf, lhs_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        at_indent(s, buf, "}}");
                        break;
                    case initializer_type_t::direct:
                        at_indent(s, buf, " = ");
                        status = process_expr(s, buf, lhs_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        break;
                }
                break;
            }
        }
        return status;
    }

    static status_t process_stmt(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        status_t status{};
        auto& intern = *s.intern;
        auto& store = *s.store;
        auto dict = bass::dict::make(cursor);
        auto type_field = bass::dict::get(dict, element::field::type);
        auto type = (statement_type_t) BASE_TYPE(type_field);
        auto label_id = bass::dict::get(dict, element::field::label);
        if (label_id) {
            auto label_cursor = bass::get_header(store, label_id);
            u32 intern_id{};
            if (!label_cursor.ok
            ||  !bass::next_field(label_cursor, intern_id, element::field::intern)) {
                return status_t::label_not_found;
            }
            auto interned = intern::get(intern, intern_id);
            at_indent(s, buf, "{}: ", interned.slice);
        }
        switch (type) {
            case statement_type_t::pp: {
                auto pp_type = (preprocessor_type_t) SUB_TYPE(type_field);
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                switch (pp_type) {
                    case preprocessor_type_t::pragma: {
                        at_indent(s, buf, "#pragma {}", interned.slice);
                        break;
                    }
                    case preprocessor_type_t::local_include: {
                        at_indent(s, buf, "#include \"{}\"", interned.slice);
                        break;
                    }
                    case preprocessor_type_t::system_include: {
                        at_indent(s, buf, "#include <{}>", interned.slice);
                        break;
                    }
                    default: {
                        return status_t::invalid_pp_type;
                    }
                }
                newline(s, buf);
                break;
            }
            case statement_type_t::raw: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                if (unlikely(!OK(interned.status)))
                    return status_t::intern_not_found;
                at_indent(s, buf, "{}", interned.slice);
                break;
            }
            case statement_type_t::if_: {
                at_indent(s, buf, "if (");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                auto true_branch_id = bass::dict::get(dict, element::field::tbranch);
                if (true_branch_id) {
                    auto branch_cursor = bass::get_header(store, true_branch_id);
                    status = process_expr(s, buf, branch_cursor);
                }
                auto false_branch_id = bass::dict::get(dict, element::field::fbranch);
                if (false_branch_id) {
                    at_indent(s, buf, " else ");
                    auto branch_cursor = bass::get_header(store, false_branch_id);
                    status = process_expr(s, buf, branch_cursor);
                }
                newline(s, buf);
                break;
            }
            case statement_type_t::do_: {
                at_indent(s, buf, "do ");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                status = process_expr(s, buf, branch_cursor);
                at_indent(s, buf, "while (");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, "); ");
                break;
            }
            case statement_type_t::for_: {
                at_indent(s, buf, "for (");
                auto init_cursor = bass::get_header(store, bass::dict::get(dict, element::field::init));
                if (unlikely(init_cursor.ok))
                    status = process_expr(s, buf, init_cursor);
                auto lhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (lhs_cursor.ok) {
                    at_indent(s, buf, "; ");
                    status = process_expr(s, buf, lhs_cursor);
                }
                auto rhs_cursor = bass::get_header(store, bass::dict::get(dict, element::field::rhs));
                if (rhs_cursor.ok) {
                    at_indent(s, buf, "; ");
                    status = process_expr(s, buf, rhs_cursor);
                }
                at_indent(s, buf, ") ");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::expr: {
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::decl: {
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                switch (expr_cursor.header->type) {
                    case element::header::type: {
                        auto type_dict = bass::dict::make(expr_cursor);
                        const auto type_flags = bass::dict::get(type_dict, element::field::type);
                        auto meta_type = (meta_type_t) BASE_TYPE(type_flags);
                        switch (meta_type) {
                            case meta_type_t::aggregate: {
                                auto agg_type = (aggregate_type_t) SUB_TYPE(type_flags);
                                switch (agg_type) {
                                    case aggregate_type_t::enum_:
                                        at_indent(s, buf, "enum ");
                                        break;
                                    case aggregate_type_t::union_:
                                        at_indent(s, buf, "union ");
                                        break;
                                    case aggregate_type_t::class_:
                                        at_indent(s, buf, "class ");
                                        break;
                                    case aggregate_type_t::struct_:
                                        at_indent(s, buf, "struct ");
                                        break;
                                    case aggregate_type_t::enum_class:
                                        at_indent(s, buf, "enum class ");
                                        break;
                                }
                                status = process_expr(s, buf, expr_cursor);
                                break;
                            }
                            case meta_type_t::function: {
                                auto lhs_cursor = bass::get_header(store, bass::dict::get(type_dict, element::field::lhs));
                                if (!lhs_cursor.ok || lhs_cursor.header->type != element::header::type)
                                    return status_t::lhs_not_found;
                                status = process_expr(s, buf, lhs_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                at_indent(s, buf, " ");
                                status = process_expr(s, buf, expr_cursor);
                                if (unlikely(!OK(status)))
                                    return status;
                                auto rhs_cursor = bass::get_header(store, bass::dict::get(type_dict, element::field::rhs));
                                if (!rhs_cursor.ok || rhs_cursor.header->type != element::header::list)
                                    return status_t::rhs_not_found;
                                at_indent(s, buf, "(");
                                status = process_list(s, buf, rhs_cursor);
                                at_indent(s, buf, ")");
                                break;
                            }
                            default:
                                return status_t::invalid_decl_element;
                        }
                        break;
                    }
                    case element::header::variable: {
                        status = decl_var(s, buf, expr_cursor);
                        break;
                    }
                    default:    return status_t::invalid_decl_element;
                }
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::empty: {
                newline(s, buf);
                break;
            }
            case statement_type_t::case_: {
                at_indent(s, buf, "case ");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ":");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::goto_: {
                at_indent(s, buf, "goto ");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::break_: {
                at_indent(s, buf, "break;");
                newline(s, buf);
                break;
            }
            case statement_type_t::while_: {
                at_indent(s, buf, "while (");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                if (branch_cursor.ok)
                    status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::using_: {
                at_indent(s, buf, "using ");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::return_: {
                at_indent(s, buf, "return ");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::public_: {
                at_indent(s, buf, "public:");
                newline(s, buf);
                break;
            }
            case statement_type_t::switch_: {
                at_indent(s, buf, "switch (");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                if (branch_cursor.ok)
                    status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::default_: {
                at_indent(s, buf, "default: ");
                auto branch_cursor = bass::get_header(store, bass::dict::get(dict, element::field::tbranch));
                if (branch_cursor.ok)
                    status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::private_: {
                at_indent(s, buf, "private:");
                newline(s, buf);
                break;
            }
            case statement_type_t::continue_: {
                at_indent(s, buf, "continue;");
                newline(s, buf);
                break;
            }
            case statement_type_t::using_ns_:{
                at_indent(s, buf, "using namespace ");
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (unlikely(!expr_cursor.ok))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::protected_: {
                at_indent(s, buf, "protected:");
                newline(s, buf);
                break;
            }
            case statement_type_t::definition: {
                auto expr_cursor = bass::get_header(store, bass::dict::get(dict, element::field::lhs));
                if (!expr_cursor.ok || expr_cursor.header->type != element::header::type)
                    return status_t::lhs_not_found;
                const auto type_dict = bass::dict::make(expr_cursor);
                const auto type_flags = bass::dict::get(type_dict, element::field::type);
                auto meta_type = (meta_type_t) BASE_TYPE(type_flags);
                switch (meta_type) {
                    case meta_type_t::function: {
                        auto lhs_cursor = bass::get_header(store, bass::dict::get(type_dict, element::field::lhs));
                        if (!lhs_cursor.ok || lhs_cursor.header->type != element::header::type)
                            return status_t::lhs_not_found;
                        status = process_expr(s, buf, lhs_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        at_indent(s, buf, " ");
                        status = process_expr(s, buf, expr_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        auto rhs_cursor = bass::get_header(store, bass::dict::get(type_dict, element::field::rhs));
                        if (!rhs_cursor.ok || rhs_cursor.header->type != element::header::list)
                            return status_t::rhs_not_found;
                        at_indent(s, buf, "(");
                        status = process_list(s, buf, rhs_cursor);
                        at_indent(s, buf, ") ");
                        auto branch_cursor = bass::get_header(store, bass::dict::get(type_dict, element::field::tbranch));
                        status = process_expr(s, buf, branch_cursor);
                        break;
                    }
                    case meta_type_t::aggregate: {
                        const auto flags = bass::dict::get(type_dict, element::field::lhs);
                        auto agg_type = (aggregate_type_t) SUB_TYPE(type_flags);
                        switch (agg_type) {
                            case aggregate_type_t::enum_:
                                at_indent(s, buf, "enum ");
                                break;
                            case aggregate_type_t::union_:
                                at_indent(s, buf, "union ");
                                break;
                            case aggregate_type_t::class_:
                                at_indent(s, buf, "class ");
                                break;
                            case aggregate_type_t::struct_:
                                at_indent(s, buf, "struct ");
                                break;
                            case aggregate_type_t::enum_class:
                                at_indent(s, buf, "enum class ");
                                break;
                        }
                        status = process_expr(s, buf, expr_cursor);
                        if (unlikely(!OK(status)))
                            return status;
                        if ((flags & aggregate::final_) == aggregate::final_)
                            at_indent(s, buf, " final ");
                        const auto inheritance_list_id = bass::dict::get(type_dict, element::field::rhs);
                        if (inheritance_list_id) {
                            auto list_cursor = bass::get_header(store, inheritance_list_id);
                            status = process_list(s, buf, list_cursor);
                            if (unlikely(!OK(status)))
                                return status;
                        }
                        auto block_id = bass::dict::get(dict, element::field::tbranch);
                        if (block_id) {
                            auto block_cursor = bass::get_header(store, block_id);
                            if (!block_cursor.ok)
                                return status_t::scope_not_found;
                            status = process_expr(s, buf, block_cursor);
                        }
                        at_indent(s, buf, ";");
                        newline(s, buf);
                        break;
                    }
                    default:    return status_t::invalid_def_element;
                }
                break;
            }
            case statement_type_t::line_comment: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                at_indent(s, buf, "//{}", interned.slice);
                newline(s, buf);
                break;
            }
            case statement_type_t::block_comment: {
                auto interned = intern::get(intern, bass::dict::get(dict, element::field::intern));
                at_indent(s, buf, "/*{}*/", interned.slice);
                break;
            }
        }
        return status;
    }

    static status_t process_scope(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        auto& store = *s.store;

        u32 parent_scope_id{};
        if (unlikely(!bass::next_field(cursor, parent_scope_id, element::field::scope)))
            return status_t::scope_not_found;

        u32 id{};
        if (unlikely(!bass::next_field(cursor, id, element::field::list)))
            return status_t::list_not_found;

        if (likely(parent_scope_id)) {
            at_indent(s, buf, "{{");
            newline(s, buf);
            tab(s);
        }

        auto list_cursor = bass::get_header(store, id);
        auto status = process_list(s, buf, list_cursor);
        if (unlikely(!OK(status)))
            return status;

        if (likely(parent_scope_id)) {
            untab(s);
            at_indent(s, buf, "}}");
        }

        return status_t::ok;
    }

    static status_t unsupported(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        return status_t::unsupported_revision;
    }

    static status_t c99(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        return status_t::not_implemented;
    }

    static status_t c11(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        return status_t::not_implemented;
    }

    static status_t cpp17(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        return status_t::not_implemented;
    }

    static status_t cpp20(serializer_t& s, format::memory_buffer_t& buf, cursor_t& cursor) {
        auto& store = *s.store;
        u32 id{};
        if (unlikely(!bass::next_field(cursor, id, element::field::child)))
            return status_t::child_not_found;
        auto scope_cursor = bass::get_header(store, id);
        if (unlikely(!scope_cursor.ok))
            return status_t::element_not_found;
        auto status = process_scope(s, buf, scope_cursor);
        if (unlikely(!OK(status)))
            return status;
        format::print("{}\n", format::to_string(buf));
        return status_t::ok;
    }

    using revision_handler_t = status_t (*)(serializer_t&, format::memory_buffer_t& buf, cursor_t& cursor);

    static revision_handler_t s_revision_handlers[] = {
        unsupported,
        c99,
        c11,
        cpp17,
        cpp20
    };

    static status_t serialize_module(serializer_t& s, u32 id, alloc_t* allocator) {
        auto& store = *s.store;
        auto cursor = bass::get_header(store, id);
        if (unlikely(!cursor.ok)) return status_t::element_not_found;
        auto dict = bass::dict::make(cursor);
        auto revision = (revision_t) bass::dict::get(dict, element::field::revision);
        format::memory_buffer_t buf(format::allocator_t{allocator});
        return (*s_revision_handlers[(u32) revision])(s, buf, cursor);
    }

    u0 free(serializer_t& s) {
        string::free(s.scratch);
    }

    status_t serialize(serializer_t& s) {
        auto& store = *s.store;
        u32 value{};
        auto pgm_cursor = bass::first_header(store);
        if (unlikely(!pgm_cursor.ok)) return status_t::pgm_not_found;
        if (unlikely(!bass::next_field(pgm_cursor, value, element::field::list)))
            return status_t::list_not_found;
        auto list_cursor = bass::get_header(store, value);
        while (bass::next_field(list_cursor, value, element::field::child)) {
            auto status = serialize_module(s, value, s.alloc);
            if (unlikely(!OK(status)))
                return status;
        }
        return status_t::ok;
    }

    u0 init(serializer_t& s, program_t& pgm, alloc_t* alloc, u16 margin, u16 tab_width) {
        s.line = {};
        s.indent = {};
        s.alloc = alloc;
        s.margin = margin;
        s.store = &pgm.storage;
        s.intern = &pgm.intern;
        s.tab_width = tab_width;
        string::init(s.scratch, s.alloc);
        string::reserve(s.scratch, 32);
    }

    status_t expand_type(bass_t& storage, intern_t& intern, u32 type_id, type_info_t& type_info) {
        u8 suffix[8];
        s32 suffix_len{};

        type_info.meta_type = meta_type_t::none;
        type_info.size_type = integral_size_t::zero;
        type_info.size = 0;
        string::reset(*type_info.name);

        auto type_cursor = bass::get_header(storage, type_id);
        while (type_cursor.ok) {
            const auto type_dict = bass::dict::make(type_cursor);
            const auto type_field = bass::dict::get(type_dict, element::field::type);
            const auto meta_type = (meta_type_t) BASE_TYPE(type_field);
            const auto size_type = (integral_size_t) SUB_TYPE(type_field);
            switch (meta_type) {
                case meta_type_t::array: {
                    type_cursor = bass::get_header(
                        storage,
                        bass::dict::get(type_dict, element::field::lhs));
                    type_info.size_type = size_type;
                    type_info.meta_type = meta_type;
                    type_info.size = bass::dict::get(type_dict, element::field::rhs);
                    break;
                }
                case meta_type_t::aggregate: {
                    type_info.size_type = size_type;
                    type_info.meta_type = meta_type;
                    type_info.size = bass::dict::get(type_dict, element::field::rhs);
                    // N.B fallthrough intentional
                }
                case meta_type_t::void_:
                case meta_type_t::alias:
                case meta_type_t::boolean:
                case meta_type_t::signed_integer:
                case meta_type_t::floating_point:
                case meta_type_t::unsigned_integer: {
                    if (type_info.meta_type == meta_type_t::none) {
                        type_info.meta_type = meta_type;
                        type_info.size_type = size_type;
                        type_info.size = program::integral_size_in_bytes(size_type);
                    }
                    auto ident_cursor = bass::get_header(
                        storage,
                        bass::dict::get(type_dict, element::field::ident));
                    u32 intern_id{};
                    if (!bass::next_field(ident_cursor, intern_id, element::field::intern)) {
                        return status_t::intern_not_found;
                    }
                    auto interned = intern::get(intern, intern_id);
                    string::append(*type_info.name, interned.slice);
                    if (suffix_len > 0) {
                        string::append(*type_info.name, suffix, suffix_len);
                        suffix_len = {};
                    }
                    return status_t::ok;
                }
                case meta_type_t::pointer: {
                    type_cursor = bass::get_header(
                        storage,
                        bass::dict::get(type_dict, element::field::lhs));
                    suffix[suffix_len++] = '*';
                    type_info.size_type = size_type;
                    type_info.meta_type = meta_type;
                    type_info.size = program::integral_size_in_bytes(size_type);
                    break;
                }
                case meta_type_t::function: {
                    type_info.size_type = size_type;
                    type_info.meta_type = meta_type;
                    type_info.size = program::integral_size_in_bytes(size_type);
                    // XXX: finish
                    break;
                }
                case meta_type_t::bit_mask: {
                    type_cursor = bass::get_header(
                        storage,
                        bass::dict::get(type_dict, element::field::lhs));
                    type_info.size_type = integral_size_t::byte;
                    type_info.meta_type = meta_type;
                    type_info.size = SUB_TYPE(type_field);
                    break;
                }
                case meta_type_t::reference: {
                    type_cursor = bass::get_header(
                        storage,
                        bass::dict::get(type_dict, element::field::lhs));
                    suffix[suffix_len++] = '&';
                    type_info.size_type = size_type;
                    type_info.meta_type = meta_type;
                    type_info.size = program::integral_size_in_bytes(size_type);
                    break;
                }
                default:    return status_t::invalid_meta_type;
            }
        }

        return status_t::element_not_found;
    }
}
