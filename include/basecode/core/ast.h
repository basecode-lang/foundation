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

#include <basecode/core/bass.h>
#include <basecode/core/token.h>

namespace basecode::ast {
    namespace node {
        namespace field {
            static constexpr u8 none        = 0b00000;
            static constexpr u8 id          = 0b00001;
            static constexpr u8 lhs         = 0b00010;
            static constexpr u8 rhs         = 0b00011;
            static constexpr u8 expr        = 0b00100;
            static constexpr u8 flags       = 0b00101;
            static constexpr u8 token       = 0b00110;
            static constexpr u8 radix       = 0b00111;
            static constexpr u8 child       = 0b01000;
            static constexpr u8 parent      = 0b01001;
            static constexpr u8 custom      = parent + 1;
        }

        namespace header {
            static constexpr u8 none        = 0b00000;
            static constexpr u8 ident       = 0b00001;
            static constexpr u8 unary       = 0b00010;
            static constexpr u8 binary      = 0b00011;
            static constexpr u8 comment     = 0b00100;
            static constexpr u8 str_lit     = 0b00101;
            static constexpr u8 num_lit     = 0b00110;
            static constexpr u8 custom      = num_lit + 1;
        }
    }

    struct node_id_t final {
        constexpr node_id_t()       : id(0)             {}
        constexpr node_id_t(u32 id) : id(id)            {}
        constexpr operator u32() const                  { return id;        }
        [[nodiscard]] constexpr b8 empty() const        { return id == 0;   }
        static constexpr node_id_t null()               { return 0;         }
    private:
        u32                     id;
    };

    struct node_type_t final {
        constexpr node_type_t()         : type(0)       {}
        constexpr node_type_t(u8 type)  : type(type)    {}
        constexpr operator u8() const                   { return type;        }
        [[nodiscard]] constexpr b8 empty() const        { return type == 0;   }
        static constexpr node_type_t none()             { return 0;           }
    private:
        u8                      type:   5;
        [[maybe_unused]] u8     pad:    3{};
    };

    struct num_lit_flags_t final {
        u8                      sign:       1;
        u8                      integer:    1;
        u8                      exponent:   1;
        [[maybe_unused]] u8     pad:        5;
    };

    node_id_t make_ident(bass_t& ast, token_id_t token);

    node_id_t make_str_lit(bass_t& ast, token_id_t token);

    node_id_t make_comment(bass_t& ast, token_id_t token);

    node_id_t make_unary_op(bass_t& ast, u32 type, node_id_t expr, token_id_t token = {});

    node_id_t make_binary_op(bass_t& ast, u32 type, node_id_t lhs, node_id_t rhs, token_id_t token = {});

    node_id_t make_num_lit(bass_t& ast, token_id_t token, Radix_Concept auto radix, num_lit_flags_t flags) {
        cursor_t c{};
        bass::seek_current(ast, c);
        bass::new_record(c, node::header::num_lit, 3);
        bass::write_field(c, node::field::token, token);
        bass::write_field(c, node::field::radix, radix);
        bass::write_field(c, node::field::flags, *((const u8*) &flags));
        return c.id;
    }
}

