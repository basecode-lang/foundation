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

namespace basecode::cxx {
    static u32 s_integral_size_in_bytes[]   = {0, 0, 1, 2, 4, 8};
    static u32 s_integral_size_in_bits[]    = {0, 0, 8, 16, 32, 64};

    static str::slice_t s_statuses[] = {
        "ok"_ss,
        "general error"_ss,
        "field not found: lhs"_ss,
        "field not found: rhs"_ss,
        "program not found"_ss,
        "field not found: list"_ss,
        "field not found: scope"_ss,
        "field not found: label"_ss,
        "field not found: child"_ss,
        "invalid pre-processor type"_ss,
        "field not found: intern"_ss,
        "invalid module revision"_ss,
        "invalid unary position type"_ss,
        "element not found"_ss,
        "invalid meta type"_ss,
        "invalid list entry"_ss,
        "invalid def element"_ss,
        "invalid decl element"_ss,
        "invalid expr element"_ss,
        "unsupported revision"_ss,
        "not implemented"_ss,
    };

    static str::slice_t s_revisions[] = {
        "none"_ss,
        "c99"_ss,
        "c11"_ss,
        "cpp17"_ss,
        "cpp20"_ss,
    };

    static str::slice_t s_meta_types[] = {
        "empty"_ss,
        "none"_ss,
        "alias"_ss,
        "void_"_ss,
        "array"_ss,
        "boolean"_ss,
        "pointer"_ss,
        "bit_mask"_ss,
        "function"_ss,
        "reference"_ss,
        "aggregate"_ss,
        "signed_integer"_ss,
        "floating_point"_ss,
        "unsigned_integer"_ss,
    };

    static str::slice_t s_aggregate_types[] = {
        "none"_ss,
        "enum_"_ss,
        "union_"_ss,
        "class_"_ss,
        "struct_"_ss,
        "enum_class"_ss,
    };

    static str::slice_t s_integral_sizes[] = {
        "none"_ss,
        "zero"_ss,
        "byte"_ss,
        "word"_ss,
        "dword"_ss,
        "qword"_ss,
    };

    static str::slice_t s_statement_types[] = {
        "none"_ss,
        "empty"_ss,
        "pp"_ss,
        "raw"_ss,
        "if_"_ss,
        "do_"_ss,
        "for_"_ss,
        "expr"_ss,
        "decl"_ss,
        "case_"_ss,
        "goto_"_ss,
        "break_"_ss,
        "while_"_ss,
        "using_"_ss,
        "return_"_ss,
        "public_"_ss,
        "switch_"_ss,
        "default_"_ss,
        "private_"_ss,
        "continue_"_ss,
        "using_ns_"_ss,
        "protected_"_ss,
        "definition"_ss,
        "line_comment"_ss,
        "block_comment"_ss,
    };

    static str::slice_t s_preprocessor_types[] = {
        "none"_ss,
        "if_"_ss,
        "pragma"_ss,
        "define"_ss,
        "endif_"_ss,
        "local_include"_ss,
        "system_include"_ss,
    };

    static str::slice_t s_expression_types[] = {
        "none"_ss,
        "raw"_ss,
        "unary"_ss,
        "binary"_ss,
        "assignment"_ss,
        "initializer"_ss,
    };

    static str::slice_t s_initializer_types[] = {
        "none"_ss,
        "direct"_ss,
        "list"_ss,
    };

    static str::slice_t s_assignment_types[] = {
        "none"_ss,
        "direct"_ss,
        "bor"_ss,
        "sum"_ss,
        "shl"_ss,
        "shr"_ss,
        "diff"_ss,
        "band"_ss,
        "bxor"_ss,
        "product"_ss,
        "quotient"_ss,
        "remainder"_ss,
    };

    static str::slice_t s_unary_op_types[] = {
        "none"_ss,
        "neg"_ss,
        "inc"_ss,
        "dec"_ss,
        "lnot"_ss,
        "bnot"_ss,
        "deref"_ss,
        "addrof"_ss,
    };

    static str::slice_t s_position_types[] = {
        "none"_ss,
        "prefix"_ss,
        "postfix"_ss
    };

    static str::slice_t s_binary_op_types[] = {
        "none"_ss,
        "eq"_ss,
        "lt"_ss,
        "gt"_ss,
        "lor"_ss,
        "add"_ss,
        "sub"_ss,
        "mul"_ss,
        "div"_ss,
        "mod"_ss,
        "shl"_ss,
        "shr"_ss,
        "bor"_ss,
        "neq"_ss,
        "lte"_ss,
        "gte"_ss,
        "band"_ss,
        "bxor"_ss,
        "land"_ss,
        "cast"_ss,
        "comma"_ss,
        "scope"_ss,
        "range"_ss,
        "member"_ss,
        "subscript"_ss,
    };

    static str::slice_t s_element_fields[] = {
        "none"_ss,
        "id"_ss,
        "lhs"_ss,
        "rhs"_ss,
        "lit"_ss,
        "init"_ss,
        "list"_ss,
        "type"_ss,
        "child"_ss,
        "scope"_ss,
        "radix"_ss,
        "ident"_ss,
        "intern"_ss,
        "parent"_ss,
        "revision"_ss,
        "label"_ss,
        "tbranch"_ss,
        "fbranch"_ss,
    };

    static str::slice_t s_element_headers[] = {
        "none"_ss,
        "type"_ss,
        "list"_ss,
        "scope"_ss,
        "ident"_ss,
        "module"_ss,
        "num_lit"_ss,
        "str_lit"_ss,
        "program"_ss,
        "variable"_ss,
        "statement"_ss,
        "expression"_ss,
        "char_lit"_ss,
        "label"_ss,
    };

    str::slice_t element::field::name(u8 value) {
        return s_element_fields[value];
    }

    str::slice_t element::header::name(u8 value) {
        return s_element_headers[value];
    }

    str::slice_t program::status_name(status_t status) {
        return s_statuses[(u32) status];
    }

    str::slice_t program::revision_name(revision_t rev) {
        return s_revisions[(u32) rev];
    }

    str::slice_t scope::type::meta_name(meta_type_t type) {
        return s_meta_types[(u32) type];
    }

    u32 program::integral_size_in_bits(integral_size_t size) {
        return s_integral_size_in_bits[(u32) size];
    }

    str::slice_t scope::stmt::name(statement_type_t type) {
        return s_statement_types[(u32) type];
    }

    str::slice_t scope::expr::name(expression_type_t type) {
        return s_expression_types[(u32) type];
    }

    u32 program::integral_size_in_bytes(integral_size_t size) {
        return s_integral_size_in_bytes[(u32) size];
    }

    str::slice_t scope::expr::unary::name(unary_op_type_t type) {
        return s_unary_op_types[(u32) type];
    }

    str::slice_t scope::stmt::pp::name(preprocessor_type_t type) {
        return s_preprocessor_types[(u32) type];
    }

    str::slice_t scope::expr::init::name(initializer_type_t type) {
        return s_initializer_types[(u32) type];
    }

    str::slice_t scope::expr::binary::name(binary_op_type_t type) {
        return s_binary_op_types[(u32) type];
    }

    str::slice_t scope::expr::assign::name(assignment_type_t type) {
        return s_assignment_types[(u32) type];
    }

    str::slice_t scope::type::aggregate_name(aggregate_type_t type) {
        return s_aggregate_types[(u32) type];
    }

    str::slice_t scope::type::integral_size_name(integral_size_t size) {
        return s_integral_sizes[(u32) size];
    }

    str::slice_t scope::expr::unary::position_name(position_type_t type) {
        return s_position_types[(u32) type];
    }
}
