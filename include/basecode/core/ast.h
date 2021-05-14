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

#pragma once

#include <basecode/core/types.h>

namespace basecode::ast {
    node_id_t make_num_lit(bass_t& ast,
                           token_id_t token,
                           num_lit_flags_t flags,
                           u32 radix);

    node_id_t make_unary_op(bass_t& ast,
                            u32 type,
                            node_id_t expr,
                            token_id_t token = {});

    node_id_t make_binary_op(bass_t& ast,
                             u32 type,
                             node_id_t lhs,
                             node_id_t rhs,
                             token_id_t token = {});

    node_id_t make_num_lit(bass_t& ast,
                           token_id_t token,
                           Radix_Concept auto radix,
                           num_lit_flags_t flags) {
        return make_num_lit(ast, token, flags, radix);
    }

    node_id_t make_ident(bass_t& ast, token_id_t token);

    node_id_t make_str_lit(bass_t& ast, token_id_t token);

    node_id_t make_comment(bass_t& ast, token_id_t token);
}

