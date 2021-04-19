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

#include <basecode/core/cxx/cxx.h>
#include <basecode/core/memory/system/proxy.h>

#define GET_INTERN(x)           (string::interned::get((x)))

namespace basecode::cxx::serializer {
    using revision_handler_t    = status_t (*)(serializer_t&,
                                               str_buf_t&,
                                               cursor_t&);

    static str::slice_t s_var_flags[] = {
        "const"_ss,
        "static"_ss,
        "volatile"_ss,
        "register"_ss,
        "constexpr"_ss
    };

    static str::slice_t s_bin_op_tokens[]     = {
        [(u32) binary_op_type_t::eq]                = "=="_ss,
        [(u32) binary_op_type_t::lt]                = "<"_ss,
        [(u32) binary_op_type_t::gt]                = ">"_ss,
        [(u32) binary_op_type_t::lor]               = "||"_ss,
        [(u32) binary_op_type_t::add]               = "+"_ss,
        [(u32) binary_op_type_t::sub]               = "-"_ss,
        [(u32) binary_op_type_t::mul]               = "*"_ss,
        [(u32) binary_op_type_t::div]               = "/"_ss,
        [(u32) binary_op_type_t::mod]               = "%"_ss,
        [(u32) binary_op_type_t::shl]               = "<<"_ss,
        [(u32) binary_op_type_t::shr]               = ">>"_ss,
        [(u32) binary_op_type_t::bor]               = "|"_ss,
        [(u32) binary_op_type_t::neq]               = "!="_ss,
        [(u32) binary_op_type_t::lte]               = "<="_ss,
        [(u32) binary_op_type_t::gte]               = ">="_ss,
        [(u32) binary_op_type_t::band]              = "&"_ss,
        [(u32) binary_op_type_t::bxor]              = "^"_ss,
        [(u32) binary_op_type_t::land]              = "&&"_ss,
        [(u32) binary_op_type_t::cast]              = ""_ss,
        [(u32) binary_op_type_t::comma]             = ","_ss,
        [(u32) binary_op_type_t::scope]             = "::"_ss,
        [(u32) binary_op_type_t::range]             = ":"_ss,
        [(u32) binary_op_type_t::member]            = ""_ss,
        [(u32) binary_op_type_t::subscript]         = ""_ss,
    };

    static str::slice_t s_unary_op_tokens[]   = {
        [(u32) unary_op_type_t::neg]                = "-"_ss,
        [(u32) unary_op_type_t::inc]                = "++"_ss,
        [(u32) unary_op_type_t::dec]                = "--"_ss,
        [(u32) unary_op_type_t::lnot]               = "!"_ss,
        [(u32) unary_op_type_t::bnot]               = "~"_ss,
        [(u32) unary_op_type_t::deref]              = "*"_ss,
        [(u32) unary_op_type_t::addrof]             = "&"_ss,
        [(u32) unary_op_type_t::addrof_label]       = "&&"_ss,
    };

    static str::slice_t s_assignment_tokens[] = {
        [(u32) assignment_type_t::direct]           = "="_ss,
        [(u32) assignment_type_t::bor]              = "|="_ss,
        [(u32) assignment_type_t::sum]              = "+="_ss,
        [(u32) assignment_type_t::shl]              = "<<="_ss,
        [(u32) assignment_type_t::shr]              = ">>="_ss,
        [(u32) assignment_type_t::diff]             = "-="_ss,
        [(u32) assignment_type_t::band]             = "&="_ss,
        [(u32) assignment_type_t::bxor]             = "^="_ss,
        [(u32) assignment_type_t::product]          = "*="_ss,
        [(u32) assignment_type_t::quotient]         = "/="_ss,
        [(u32) assignment_type_t::remainder]        = "%="_ss,
    };

    static const s8* s_pp_templates[]  = {
        [(u32) preprocessor_type_t::if_]            = "#if{}",
        [(u32) preprocessor_type_t::pragma]         = "#pragma {}",
        [(u32) preprocessor_type_t::define]         = "#define{}",
        [(u32) preprocessor_type_t::endif_]         = "#endif",
        [(u32) preprocessor_type_t::local_include]  = "#include \"{}\"",
        [(u32) preprocessor_type_t::system_include] = "#include <{}>",
    };

    static str::slice_t s_aggregate_type_tokens[] = {
        [(u32) aggregate_type_t::enum_]             = "enum"_ss,
        [(u32) aggregate_type_t::union_]            = "union"_ss,
        [(u32) aggregate_type_t::class_]            = "class"_ss,
        [(u32) aggregate_type_t::struct_]           = "struct"_ss,
        [(u32) aggregate_type_t::enum_class]        = "enum class"_ss,
    };

    static status_t c99(serializer_t& s,
                        str_buf_t& buf,
                        cursor_t& cursor);

    static status_t c11(serializer_t& s,
                        str_buf_t& buf,
                        cursor_t& cursor);

    static status_t cpp17(serializer_t& s,
                          str_buf_t& buf,
                          cursor_t& cursor);

    static status_t cpp20(serializer_t& s,
                          str_buf_t& buf,
                          cursor_t& cursor);

    static status_t unsupported(serializer_t& s,
                                str_buf_t& buf,
                                cursor_t& cursor);

    static status_t process_list(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor);

    static status_t process_expr(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor);

    static status_t process_stmt(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor);

    static status_t process_scope(serializer_t& s,
                                  str_buf_t& buf,
                                  cursor_t& cursor);

    static u0 apply_type_suffix(meta_type_t meta_type,
                                u8* suffix,
                                s32& suffix_len);

    static u0 newline(serializer_t& s, str_buf_t& buf);

    static u0 process_var_flags(serializer_t& s, str_buf_t& buf, u8 flags);

    static status_t serialize_module(serializer_t& s, u32 id, alloc_t* alloc);

    static status_t decl_var(serializer_t& s, str_buf_t& buf, cursor_t& cursor);

    static revision_handler_t s_revision_handlers[] = {
        unsupported,
        c99,
        c11,
        cpp17,
        cpp20
    };

    u0 init(serializer_t& s,
            program_t& pgm,
            alloc_t* alloc,
            u16 margin,
            u16 tab_width) {
        s.line      = {};
        s.indent    = {};
        s.alloc     = alloc;
        s.margin    = margin;
        s.store     = &pgm.storage;
        s.tab_width = tab_width;
        symtab::init(s.modules, s.alloc);
        for (u32 i = 0; i < 2; ++i) {
            str::init(s.scratch[i], s.alloc);
            str::reserve(s.scratch[i], 32);
        }
    }

    u0 free(serializer_t& s) {
        for (u32 i = 0; i < 2; ++i)
            str::free(s.scratch[i]);
        assoc_array_t<str_t*> pairs{};
        assoc_array::init(pairs, s.alloc);
        symtab::find_prefix(s.modules, pairs);
        for (u32 i = 0; i < pairs.size; ++i)
            str::free(*pairs[i].value);
        symtab::free(s.modules);
        assoc_array::free(pairs);
    }

    static u0 tab(serializer_t& s) {
        s.indent += s.tab_width - (s.column % s.tab_width);
    }

    static u0 untab(serializer_t& s) {
        s.indent -= s.tab_width - (s.column % s.tab_width);
    }

    status_t serialize(serializer_t& s) {
        auto& store = *s.store;
        u32 value{};
        cursor_t pgm_cursor{};
        if (!bass::seek_first(store, pgm_cursor))
            return status_t::pgm_not_found;
        if (UNLIKELY(!bass::next_field(pgm_cursor, value, element::field::list)))
            return status_t::list_not_found;
        cursor_t list_cursor{};
        bass::seek_record(store, value, list_cursor);
        while (bass::next_field(list_cursor, value, element::field::child)) {
            auto status = serialize_module(s, value, s.alloc);
            if (UNLIKELY(!OK(status)))
                return status;
        }
        return status_t::ok;
    }

    template <typename... Args>
    static u0 at_indent(serializer_t& s,
                        str_buf_t& buf,
                        fmt::string_view format_str,
                        const Args&... args) {
        if (LIKELY(s.column == 0 && s.indent > 0))
            format::format_to(buf, "{:<{}}", " ", s.indent);
        auto start = buf.size();
        format::format_to(buf, format_str, args...);
        s.column += buf.size() - start;
        if (s.column > s.margin)
            newline(s, buf);
    }

    static status_t decl_var(serializer_t& s,
                             str_buf_t& buf,
                             cursor_t& cursor) {
        auto&       store  = *s.store;
        cursor_t    init_cursor{};
        cursor_t    rhs_cursor{};
        type_info_t type_info{.name = &s.scratch[0],
                              .var_suffix = &s.scratch[1]};

        auto       dict    = bass::dict::make(cursor);
        const auto lhs_id  = DICTV(dict, element::field::lhs);
        const auto rhs_id  = DICTV(dict, element::field::rhs);
        const auto init_id = DICTV(dict, element::field::init);
        const auto type_id = DICTV(dict, element::field::type);
        process_var_flags(s, buf, type_id);
        expand_type(store, lhs_id, type_info);
        at_indent(s, buf, "{} ", *type_info.name);
        if (UNLIKELY(!bass::seek_record(store, rhs_id, rhs_cursor)))
            return status_t::rhs_not_found;
        auto status = process_expr(s, buf, rhs_cursor);
        if (UNLIKELY(!OK(status)))
            return status;
        at_indent(s, buf, "{}", *type_info.var_suffix);
        if (init_id) {
            if (UNLIKELY(!bass::seek_record(store, init_id, init_cursor)))
                return status_t::element_not_found;
            status = process_expr(s, buf, init_cursor);
        }
        return status;
    }

    static status_t unsupported(serializer_t& s,
                                str_buf_t& buf,
                                cursor_t& cursor) {
        UNUSED(s); UNUSED(buf); UNUSED(cursor);
        return status_t::unsupported_revision;
    }

    static status_t process_list(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor) {
        auto&       store = *s.store;
        status_t    status{};
        cursor_t    element_cursor{};
        u32         id{};

        // first
        const b8 has_more = bass::next_field(cursor,
                                             id,
                                             element::field::child);
        if (!has_more) return status;
        if (UNLIKELY(!bass::seek_record(store, id, element_cursor)))
            return status_t::element_not_found;
        switch (element_cursor.header->type) {
            case element::header::scope:
                break;
            case element::header::variable:
                status = decl_var(s, buf, element_cursor);
                break;
            case element::header::statement:
                status = process_stmt(s, buf, element_cursor);
                break;
            case element::header::expression:
                status = process_expr(s, buf, element_cursor);
                break;
            default:
                return status_t::invalid_list_entry;
        }

        // rest
        while (OK(status)
            && bass::next_field(cursor, id, element::field::child)) {
            if (UNLIKELY(!bass::seek_record(store, id, element_cursor)))
                return status_t::element_not_found;
            switch (element_cursor.header->type) {
                case element::header::variable:
                    at_indent(s, buf, ", ");
                    status = decl_var(s, buf, element_cursor);
                    break;
                case element::header::statement:
                    status = process_stmt(s, buf, element_cursor);
                    break;
                case element::header::expression:
                    at_indent(s, buf, ", ");
                    status = process_expr(s, buf, element_cursor);
                    break;
                case element::header::scope:
                    break;
                default:
                    return status_t::invalid_list_entry;
            }
        }

        return status;
    }

    static status_t process_expr(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor) {
        status_t status{};
        auto& store = *s.store;
        cursor_t ident_cursor{};
        auto dict = bass::dict::make(cursor);
        switch (cursor.header->type) {
            case element::header::type: {
                bass::seek_record(
                    store,
                    DICTV(dict, element::field::ident),
                    ident_cursor);
                return process_expr(s, buf, ident_cursor);
            }
            case element::header::ident:
            case element::header::num_lit: {
                auto interned = GET_INTERN(DICTV(dict, element::field::intern));
                at_indent(s, buf, "{}", interned.slice);
                return status_t::ok;
            }
            case element::header::str_lit: {
                auto interned = GET_INTERN(DICTV(dict, element::field::intern));
                at_indent(s, buf, "\"{}\"", interned.slice);
                return status_t::ok;
            }
            case element::header::char_lit: {
                at_indent(s,
                          buf,
                          "'{}'",
                          (s8) DICTV(dict, element::field::lit));
                return status_t::ok;
            }
            case element::header::variable: {
                bass::seek_record(store,
                                  DICTV(dict, element::field::rhs),
                                  ident_cursor);
                return process_expr(s, buf, ident_cursor);
            }
            case element::header::scope:
                return process_scope(s, buf, cursor);
            case element::header::statement:
                return process_stmt(s, buf, cursor);
            case element::header::expression:
                break;
            default:
                return status_t::invalid_expr_element;
        }
        auto type_field = DICTV(dict, element::field::type);
        auto type       = (expression_type_t) BASE_TYPE(type_field);
        cursor_t lhs_cursor{}, rhs_cursor{};
        auto lhs = DICTV(dict, element::field::lhs);
        if (UNLIKELY(!bass::seek_record(store, lhs, lhs_cursor)))
            return status_t::lhs_not_found;
        switch (type) {
            case expression_type_t::raw: {
                auto intern_id = DICTV(dict, element::field::intern);
                auto interned = GET_INTERN(intern_id);
                if (UNLIKELY(!OK(interned.status)))
                    return status_t::intern_not_found;
                at_indent(s, buf, "{}", interned.slice);
                break;
            }
            case expression_type_t::unary: {
                auto op_type = (unary_op_type_t) SUB_TYPE(type_field);
                const auto unary_op_token = s_unary_op_tokens[(u32) op_type];
                switch (op_type) {
                    case unary_op_type_t::inc:
                    case unary_op_type_t::dec: {
                        auto pos_type = (position_type_t) POS_TYPE(type_field);
                        switch (pos_type) {
                            case position_type_t::none:
                                return status_t::invalid_pos_type;
                            case position_type_t::prefix: {
                                at_indent(s,
                                          buf,
                                          "{}",
                                          unary_op_token);
                                status = process_expr(s,
                                                      buf,
                                                      lhs_cursor);
                                if (UNLIKELY(!OK(status)))
                                    return status;
                                break;
                            }
                            case position_type_t::postfix: {
                                status = process_expr(s,
                                                      buf,
                                                      lhs_cursor);
                                if (UNLIKELY(!OK(status)))
                                    return status;
                                at_indent(s,
                                          buf,
                                          "{}",
                                          unary_op_token);
                                break;
                            }
                        }
                        break;
                    }
                    default:
                        at_indent(s,
                                  buf,
                                  "{}",
                                  unary_op_token);
                        break;
                }
                break;
            }
            case expression_type_t::binary: {
                auto lhs_dict = bass::dict::make(lhs_cursor);

                status = process_expr(s, buf, lhs_cursor);
                if (UNLIKELY(!OK(status)))
                    return status;

                auto rhs = DICTV(dict, element::field::rhs);
                if (UNLIKELY(!bass::seek_record(store, rhs, rhs_cursor)))
                    return status_t::rhs_not_found;

                auto op_type = (binary_op_type_t) SUB_TYPE(type_field);
                auto bin_op_token = s_bin_op_tokens[(u32) op_type];
                if (slice::empty(bin_op_token)) {
                    switch (op_type) {
                        case binary_op_type_t::member: {
                            if (lhs_cursor.header->type != element::header::variable)
                                return status_t::error;
                            cursor_t type_cursor{};
                            lhs = DICTV(lhs_dict, element::field::lhs);
                            if (!bass::seek_record(store, lhs, type_cursor))
                                return status_t::error;
                            u32 type_value{};
                            if (!bass::next_field(type_cursor, type_value, element::field::type))
                                return status_t::error;
                            auto meta_type = (meta_type_t) BASE_TYPE(type_value);
                            switch (meta_type) {
                                case meta_type_t::pointer:
                                    at_indent(s, buf, "->");
                                    break;
                                case meta_type_t::aggregate:
                                case meta_type_t::reference:
                                    at_indent(s, buf, ".");
                                    break;
                                default:
                                    return status_t::error;
                            }
                            goto bin_op_done;
                        }
                        case binary_op_type_t::cast:
                            at_indent(s, buf, "(");
                            status = process_expr(s, buf, rhs_cursor);
                            if (UNLIKELY(!OK(status)))
                                return status;
                            at_indent(s, buf, ")");
                            break;
                        case binary_op_type_t::subscript:
                            at_indent(s, buf, "[");
                            status = process_expr(s, buf, rhs_cursor);
                            if (UNLIKELY(!OK(status)))
                                return status;
                            at_indent(s, buf, "]");
                            break;
                        default:
                            return status_t::error;
                    }
                    break;
                } else {
                    at_indent(s, buf, " {} ", bin_op_token);
                }

                bin_op_done:
                status = process_expr(s, buf, rhs_cursor);
                if (UNLIKELY(!OK(status)))
                    return status;
                break;
            }
            case expression_type_t::assignment: {
                auto rhs = DICTV(dict, element::field::rhs);
                if (UNLIKELY(!bass::seek_record(store, rhs, rhs_cursor)))
                    return status_t::rhs_not_found;
                status = process_expr(s, buf, lhs_cursor);
                if (UNLIKELY(!OK(status)))
                    return status;
                auto op_type = (assignment_type_t) SUB_TYPE(type_field);
                at_indent(s,
                          buf,
                          " {} ",
                          s_assignment_tokens[(u32) op_type]);
                status = process_expr(s, buf, rhs_cursor);
                break;
            }
            case expression_type_t::initializer: {
                auto op_type = (initializer_type_t) SUB_TYPE(type_field);
                switch (op_type) {
                    case initializer_type_t::list:
                        at_indent(s, buf, "{{");
                        status = process_list(s, buf, lhs_cursor);
                        if (UNLIKELY(!OK(status)))
                            return status;
                        at_indent(s, buf, "}}");
                        break;
                    case initializer_type_t::direct:
                        at_indent(s, buf, " = ");
                        status = process_expr(s, buf, lhs_cursor);
                        if (UNLIKELY(!OK(status)))
                            return status;
                        break;
                }
                break;
            }
        }
        return status;
    }

    static status_t process_stmt(serializer_t& s,
                                 str_buf_t& buf,
                                 cursor_t& cursor) {
        status_t status{};
        auto& store  = *s.store;
        auto dict       = bass::dict::make(cursor);
        auto type_field = DICTV(dict, element::field::type);
        auto label_id   = DICTV(dict, element::field::label);
        auto type       = (statement_type_t) BASE_TYPE(type_field);
        if (label_id) {
            cursor_t label_cursor{};
            if (!bass::seek_record(store, label_id, label_cursor))
                return status_t::label_not_found;
            u32 intern_id{};
            if (!bass::next_field(label_cursor, intern_id, element::field::intern))
                return status_t::label_not_found;
            auto interned = string::interned::get(intern_id);
            at_indent(s, buf, "{}: ", interned.slice);
        }
        switch (type) {
            case statement_type_t::pp: {
                auto pp_type = (preprocessor_type_t) SUB_TYPE(type_field);
                const auto intern_id = DICTV(dict, element::field::intern);
                auto interned  = string::interned::get(intern_id);
                const auto pp_template  = s_pp_templates[(u32) pp_type];
                at_indent(s, buf, pp_template, interned.slice);
                newline(s, buf);
                break;
            }
            case statement_type_t::raw: {
                const auto intern_id = DICTV(dict, element::field::intern);
                auto interned = string::interned::get(intern_id);
                if (UNLIKELY(!OK(interned.status)))
                    return status_t::intern_not_found;
                at_indent(s, buf, "{}", interned.slice);
                break;
            }
            case statement_type_t::if_: {
                at_indent(s, buf, "if (");
                cursor_t expr_cursor{};
                auto lhs = DICTV(dict, element::field::lhs);
                if (UNLIKELY(!bass::seek_record(store, lhs, expr_cursor)))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                auto true_branch_id = DICTV(dict, element::field::tbranch);
                if (true_branch_id) {
                    cursor_t branch_cursor{};
                    bass::seek_record(store,
                                      true_branch_id,
                                      branch_cursor);
                    status = process_expr(s, buf, branch_cursor);
                }
                auto false_branch_id = DICTV(dict, element::field::fbranch);
                if (false_branch_id) {
                    at_indent(s, buf, " else ");
                    cursor_t branch_cursor{};
                    bass::seek_record(store,
                                      false_branch_id,
                                      branch_cursor);
                    status = process_expr(s, buf, branch_cursor);
                }
                newline(s, buf);
                break;
            }
            case statement_type_t::do_: {
                at_indent(s, buf, "do ");
                cursor_t branch_cursor{};
                auto tbranch_id = DICTV(dict, element::field::tbranch);
                bass::seek_record(store, tbranch_id, branch_cursor);
                status = process_expr(s, buf, branch_cursor);
                if (!OK(status))
                    return status_t::scope_not_found;
                at_indent(s, buf, "while (");
                cursor_t expr_cursor{};
                const auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, "); ");
                break;
            }
            case statement_type_t::for_: {
                at_indent(s, buf, "for (");
                cursor_t lhs_cursor{};
                cursor_t rhs_cursor{};
                cursor_t init_cursor{};
                cursor_t branch_cursor{};
                const auto lhs        = DICTV(dict, element::field::lhs);
                const auto rhs        = DICTV(dict, element::field::rhs);
                const auto init_id    = DICTV(dict, element::field::init);
                const auto tbranch_id = DICTV(dict, element::field::tbranch);
                if (bass::seek_record(store, init_id, init_cursor)) {
                    status = process_expr(s, buf, init_cursor);
                    if (!OK(status))
                        return status_t::element_not_found;
                }
                if (bass::seek_record(store, lhs, lhs_cursor)) {
                    at_indent(s, buf, "; ");
                    status = process_expr(s, buf, lhs_cursor);
                    if (!OK(status))
                        return status_t::lhs_not_found;
                }
                if (bass::seek_record(store, rhs, rhs_cursor)) {
                    at_indent(s, buf, "; ");
                    status = process_expr(s, buf, rhs_cursor);
                    if (!OK(status))
                        return status_t::rhs_not_found;
                }
                at_indent(s, buf, ") ");
                bass::seek_record(store, tbranch_id, branch_cursor);
                status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::expr: {
                cursor_t expr_cursor{};
                const auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::decl: {
                cursor_t expr_cursor{};
                auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                switch (expr_cursor.header->type) {
                    case element::header::type: {
                        auto type_dict  = bass::dict::make(expr_cursor);
                        const auto type_flags = DICTV(type_dict, element::field::type);
                        auto meta_type  = (meta_type_t) BASE_TYPE(type_flags);
                        switch (meta_type) {
                            case meta_type_t::aggregate: {
                                auto agg_type = (aggregate_type_t) SUB_TYPE(type_flags);
                                const auto agg_token = s_aggregate_type_tokens[(u32) agg_type];
                                at_indent(s,
                                          buf,
                                          "{} ",
                                          agg_token);
                                status = process_expr(s,
                                                      buf,
                                                      expr_cursor);
                                break;
                            }
                            case meta_type_t::function: {
                                cursor_t lhs_cursor{};
                                cursor_t rhs_cursor{};
                                lhs = DICTV(type_dict, element::field::lhs);
                                if (!bass::seek_record(store, lhs, lhs_cursor))
                                    return status_t::lhs_not_found;
                                if (lhs_cursor.header->type != element::header::type)
                                    return status_t::lhs_not_found;
                                status = process_expr(s, buf, lhs_cursor);
                                if (UNLIKELY(!OK(status)))
                                    return status;
                                at_indent(s, buf, " ");
                                status = process_expr(s, buf, expr_cursor);
                                if (UNLIKELY(!OK(status)))
                                    return status;
                                auto rhs = DICTV(type_dict, element::field::rhs);
                                if (!bass::seek_record(store, rhs, rhs_cursor))
                                    return status_t::rhs_not_found;
                                if (rhs_cursor.header->type != element::header::list)
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
                    default:
                        return status_t::invalid_decl_element;
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
                cursor_t expr_cursor{};
                cursor_t branch_cursor{};
                const auto lhs        = DICTV(dict, element::field::lhs);
                const auto tbranch_id = DICTV(dict, element::field::tbranch);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                if (!OK(status))
                    return status;
                at_indent(s, buf, ":");
                bass::seek_record(store, tbranch_id, branch_cursor);
                status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::goto_: {
                at_indent(s, buf, "goto ");
                cursor_t expr_cursor{};
                const auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
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
                cursor_t expr_cursor{};
                cursor_t branch_cursor{};
                const auto lhs        = DICTV(dict, element::field::lhs);
                const auto tbranch_id = DICTV(dict, element::field::tbranch);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                if (bass::seek_record(store, tbranch_id, branch_cursor))
                    status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::using_: {
                at_indent(s, buf, "using ");
                cursor_t expr_cursor{};
                const auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ";");
                newline(s, buf);
                break;
            }
            case statement_type_t::return_: {
                at_indent(s, buf, "return ");
                cursor_t expr_cursor{};
                const auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
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
                cursor_t expr_cursor{};
                cursor_t branch_cursor{};
                const auto lhs        = DICTV(dict, element::field::lhs);
                const auto tbranch_id = DICTV(dict, element::field::tbranch);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                status = process_expr(s, buf, expr_cursor);
                at_indent(s, buf, ") ");
                if (bass::seek_record(store, tbranch_id, branch_cursor))
                    status = process_expr(s, buf, branch_cursor);
                break;
            }
            case statement_type_t::default_: {
                at_indent(s, buf, "default: ");
                cursor_t branch_cursor{};
                const auto tbranch_id = DICTV(dict, element::field::tbranch);
                if (bass::seek_record(store, tbranch_id, branch_cursor))
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
                cursor_t expr_cursor{};
                const auto lhs_id = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs_id, expr_cursor))
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
                cursor_t expr_cursor{};
                auto lhs = DICTV(dict, element::field::lhs);
                if (!bass::seek_record(store, lhs, expr_cursor))
                    return status_t::lhs_not_found;
                const auto type_dict  = bass::dict::make(expr_cursor);
                const auto type_flags = DICTV(type_dict, element::field::type);
                auto       meta_type  = (meta_type_t) BASE_TYPE(type_flags);
                switch (meta_type) {
                    case meta_type_t::function: {
                        cursor_t lhs_cursor{};
                        cursor_t rhs_cursor{};
                        cursor_t branch_cursor{};
                        lhs = DICTV(type_dict, element::field::lhs);
                        const auto rhs        = DICTV(type_dict, element::field::rhs);
                        const auto tbranch_id = DICTV(type_dict, element::field::tbranch);
                        if (!bass::seek_record(store, lhs, lhs_cursor))
                            return status_t::lhs_not_found;
                        status = process_expr(s, buf, lhs_cursor);
                        if (UNLIKELY(!OK(status)))
                            return status;
                        at_indent(s, buf, " ");
                        status = process_expr(s, buf, expr_cursor);
                        if (UNLIKELY(!OK(status)))
                            return status;
                        if (!bass::seek_record(store, rhs, rhs_cursor))
                            return status_t::rhs_not_found;
                        at_indent(s, buf, "(");
                        status = process_list(s, buf, rhs_cursor);
                        if (!OK(status))
                            return status;
                        at_indent(s, buf, ") ");
                        bass::seek_record(store, tbranch_id, branch_cursor);
                        status = process_expr(s, buf, branch_cursor);
                        break;
                    }
                    case meta_type_t::aggregate: {
                        const auto flags     = DICTV(type_dict, element::field::lhs);
                        auto       agg_type  = (aggregate_type_t) SUB_TYPE(type_flags);
                        const auto agg_token = s_aggregate_type_tokens[(u32) agg_type];
                        at_indent(s, buf, "{} ", agg_token);
                        status = process_expr(s, buf, expr_cursor);
                        if (UNLIKELY(!OK(status)))
                            return status;
                        if ((flags & aggregate::final_) == aggregate::final_)
                            at_indent(s, buf, " final ");
                        const auto inheritance_list_id = DICTV(type_dict, element::field::rhs);
                        if (inheritance_list_id) {
                            cursor_t list_cursor{};
                            bass::seek_record(store, inheritance_list_id, list_cursor);
                            status = process_list(s, buf, list_cursor);
                            if (UNLIKELY(!OK(status)))
                                return status;
                        }
                        auto block_id = DICTV(dict, element::field::tbranch);
                        if (block_id) {
                            cursor_t block_cursor{};
                            if (!bass::seek_record(store, block_id, block_cursor))
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
                auto interned = GET_INTERN(DICTV(dict, element::field::intern));
                at_indent(s, buf, "//{}", interned.slice);
                newline(s, buf);
                break;
            }
            case statement_type_t::block_comment: {
                auto interned = GET_INTERN(DICTV(dict, element::field::intern));
                at_indent(s, buf, "/*{}*/", interned.slice);
                break;
            }
        }
        return status;
    }

    static status_t process_scope(serializer_t& s,
                                  str_buf_t& buf,
                                  cursor_t& cursor) {
        auto& store = *s.store;

        u32 parent_scope_id{};
        if (UNLIKELY(!bass::next_field(cursor, parent_scope_id, element::field::scope)))
            return status_t::scope_not_found;

        u32 id{};
        if (UNLIKELY(!bass::next_field(cursor, id, element::field::list)))
            return status_t::list_not_found;

        if (LIKELY(parent_scope_id)) {
            at_indent(s, buf, "{{");
            newline(s, buf);
            tab(s);
        }

        cursor_t list_cursor{};
        bass::seek_record(store, id, list_cursor);
        auto status = process_list(s, buf, list_cursor);
        if (UNLIKELY(!OK(status)))
            return status;

        if (LIKELY(parent_scope_id)) {
            untab(s);
            at_indent(s, buf, "}}");
        }

        return status_t::ok;
    }

    static u0 newline(serializer_t& s, str_buf_t& buf) {
        ++s.line;
        s.column = {};
        format::format_to(buf, "\n");
    }

    static u0 process_var_flags(serializer_t& s, str_buf_t& buf, u8 flags) {
        u8 mask = 1;
        for (u32 i = 0; i < 8; ++i) {
            if ((flags & mask) == mask)
                at_indent(s, buf, "{} ", s_var_flags[i]);
            mask <<= (u32) 1;
        }
    }

    static status_t c99(serializer_t& s, str_buf_t& buf, cursor_t& cursor) {
        UNUSED(s); UNUSED(buf); UNUSED(cursor);
        return status_t::not_implemented;
    }

    static status_t c11(serializer_t& s, str_buf_t& buf, cursor_t& cursor) {
        UNUSED(s); UNUSED(buf); UNUSED(cursor);
        return status_t::not_implemented;
    }

    static status_t cpp17(serializer_t& s, str_buf_t& buf, cursor_t& cursor) {
        UNUSED(s); UNUSED(buf); UNUSED(cursor);
        return status_t::not_implemented;
    }

    static status_t cpp20(serializer_t& s, str_buf_t& buf, cursor_t& cursor) {
        auto& store = *s.store;
        u32 id{};
        if (UNLIKELY(!bass::next_field(cursor, id, element::field::child)))
            return status_t::child_not_found;
        cursor_t scope_cursor{};
        if (!bass::seek_record(store, id, scope_cursor))
            return status_t::element_not_found;
        auto status = process_scope(s, buf, scope_cursor);
        if (UNLIKELY(!OK(status)))
            return status;
        return status_t::ok;
    }

    static status_t serialize_module(serializer_t& s, u32 id, alloc_t* alloc) {
        UNUSED(alloc);

        auto& store = *s.store;
        cursor_t cursor{};

        if (!bass::seek_record(store, id, cursor))
            return status_t::element_not_found;

        auto dict         = bass::dict::make(cursor);
        auto revision     = (revision_t) DICTV(dict, element::field::revision);
        auto filename_lit = DICTV(dict, element::field::lit);
        str::slice_t filename = "(module)"_ss;
        if (filename_lit) {
            cursor_t lit_cursor{};
            if (!bass::seek_record(store, filename_lit, lit_cursor))
                return status_t::intern_not_found;
            auto lit_dict = bass::dict::make(lit_cursor);
            const auto intern_id = DICTV(lit_dict, element::field::intern);
            auto interned = GET_INTERN(intern_id);
            if (UNLIKELY(!OK(interned.status)))
                return status_t::intern_not_found;
            filename = interned.slice;
        }
        str_t* str{};
        if (!symtab::emplace(s.modules, filename, &str))
            return status_t::error;
        str::init(*str, s.alloc);
        str::reserve(*str, 512);
        str_buf_t buf(str);
        return (*s_revision_handlers[(u32) revision])(s, buf, cursor);
    }

    status_t expand_type(bass_t& storage, u32 type_id, type_info_t& type_info) {
        cursor_t type_cursor{};
        if (!bass::seek_record(storage, type_id, type_cursor))
            return status_t::element_not_found;

        u8 suffix[8];
        s32 suffix_len{};

        auto type_dict  = bass::dict::make(type_cursor);
        auto lhs_id     = DICTV(type_dict, element::field::lhs);
        auto type_field = DICTV(type_dict, element::field::type);
        auto meta_type  = (meta_type_t) BASE_TYPE(type_field);

        if (meta_type == meta_type_t::bit_mask) {
            str::append(*type_info.var_suffix,
                        format::format(":{}",
                                       SUB_TYPE(type_field)));
        }

        str::reset(*type_info.name);
        str::reset(*type_info.var_suffix);
        apply_type_suffix(meta_type, suffix, suffix_len);
        while (lhs_id) {
            if (!bass::seek_record(storage, lhs_id, type_cursor))
                return status_t::element_not_found;
            type_dict  = bass::dict::make(type_cursor);
            lhs_id     = DICTV(type_dict, element::field::lhs);
            type_field = DICTV(type_dict, element::field::type);
            meta_type  = (meta_type_t) BASE_TYPE(type_field);
            apply_type_suffix(meta_type, suffix, suffix_len);
        }

        switch (meta_type) {
            case meta_type_t::void_:
            case meta_type_t::alias:
            case meta_type_t::array:
            case meta_type_t::boolean:
            case meta_type_t::function:
            case meta_type_t::aggregate:
            case meta_type_t::signed_integer:
            case meta_type_t::floating_point:
            case meta_type_t::unsigned_integer: {
                cursor_t ident_cursor{};
                u32 intern_id{};
                u32 ident_id = DICTV(type_dict, element::field::ident);
                if (!bass::seek_record(storage, ident_id, ident_cursor))
                    return status_t::element_not_found;
                if (!bass::next_field(ident_cursor, intern_id, element::field::intern))
                    return status_t::intern_not_found;
                auto interned = GET_INTERN(intern_id);
                str::append(*type_info.name, interned.slice);
                if (suffix_len > 0) {
                    str::append(*type_info.name, suffix, suffix_len);
                    suffix_len = {};
                }
                if (meta_type == meta_type_t::array) {
                    auto size_field = DICTV(type_dict, element::field::rhs);
                    if (size_field > 0) {
                        str::append(*type_info.var_suffix,
                                    format::format("[{}]",
                                                   size_field));
                    } else {
                        str::append(*type_info.var_suffix, "[]");
                    }
                }
                break;
            }
            default: return status_t::invalid_meta_type;
        }

        return status_t::ok;
    }

    static u0 apply_type_suffix(meta_type_t meta_type, u8* suffix, s32& suffix_len) {
        if      (meta_type == meta_type_t::pointer)     suffix[suffix_len++] = '*';
        else if (meta_type == meta_type_t::reference)   suffix[suffix_len++] = '&';
    }
}
