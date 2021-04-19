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

#include <basecode/core/ast.h>

namespace basecode::ast {
    node_id_t make_ident(bass_t& ast, token_id_t token) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::ident, 1);
        bass::write_field(c, node::field::token, token);
        return c.id;
    }

    node_id_t make_str_lit(bass_t& ast, token_id_t token) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::str_lit, 1);
        bass::write_field(c, node::field::token, token);
        return c.id;
    }

    node_id_t make_comment(bass_t& ast, token_id_t token) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::comment, 1);
        bass::write_field(c, node::field::token, token);
        return c.id;
    }

    node_id_t make_unary_op(bass_t& ast, u32 type, node_id_t expr, token_id_t token) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::unary, 3);
        bass::write_field(c, node::field::token, token);
        bass::write_field(c, node::field::expr, expr);
        bass::write_field(c, node::field::flags, type);
        return c.id;
    }

    node_id_t make_binary_op(bass_t& ast, u32 type, node_id_t lhs, node_id_t rhs, token_id_t token) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::binary, 4);
        bass::write_field(c, node::field::token, token);
        bass::write_field(c, node::field::rhs, lhs);
        bass::write_field(c, node::field::lhs, rhs);
        bass::write_field(c, node::field::flags, type);
        return c.id;
    }
}
