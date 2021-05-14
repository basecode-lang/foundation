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

#include <basecode/core/buf.h>
#include <basecode/core/types.h>
#include <basecode/core/format.h>

namespace basecode {
    struct rgb_t final {
        u32                     r:      8;
        u32                     g:      8;
        u32                     b:      8;
        u32                     pad:    8;

        u32 palette_value() const {
            return (16 + 36 * r) + (6 * g) + b;
        }

        inline b8 operator==(const rgb_t& other) const {
            return r == other.r
                   && g == other.g
                   && b == other.b;
        }

        inline b8 operator!=(const rgb_t& other) const {
            return !(*this == other);
        }
    };

    struct cursor_pos_t final {
        u32                     row;
        u32                     col;
    };

    struct color_value_t final {
        union {
            term::color_t       index;
            rgb_t               triplet;
        }                       value;
        color_mode_t            mode;
    };

    struct term_command_t final {
        union {
            cursor_pos_t        pos;
            clear_mode_t        clear_mode;
            struct {
                u32             mode;
                u32             mask;
                rgb_t           fg;
                rgb_t           bg;
            }                   sgr;
            struct {
                u32             n;
            }                   edit;
        }                       params;
        term_command_type_t     type;
    };

    struct term_t final {
        alloc_t*                alloc;
        term_command_array_t    commands;
        color_value_t           fg;
        color_value_t           bg;
        cursor_pos_t            cursor;
        style_t                 style;
        u8                      dirty:      1;
        u8                      enabled:    1;
        u8                      pad:        7;
    };

    namespace term {
        namespace system {
            u0 fini();

            u0 enabled(b8 enabled);

            status_t init(b8 enabled,
                          alloc_t* alloc = context::top()->alloc.main);
        }

        u0 free(term_t& term);

        u0 colorize_range(term_t& term,
                          str_buf_t& str_buf,
                          color_t fg,
                          color_t bg,
                          buf_t* buf,
                          u32 line_number,
                          u32 begin,
                          u32 end);

        u0 reset_all(term_t& term);

        u0 get_cursor_pos(term_t& term);

        u0 set_fg(term_t& term, color_t fg);

        u0 set_bg(term_t& term, color_t bg);

        u0 cursor_up(term_t& term, u32 n = 1);

        u0 scroll_up(term_t& term, u32 n = 1);

        u0 scroll_down(term_t& term, u32 n = 1);

        u0 cursor_back(term_t& term, u32 n = 1);

        u0 cursor_down(term_t& term, u32 n = 1);

        u0 refresh(term_t& term, str_buf_t& buf);

        u0 set_style(term_t& term, style_t style);

        u0 cursor_forward(term_t& term, u32 n = 1);

        u0 cursor_to(term_t& term, cursor_pos_t pos);

        u0 cursor_next_line(term_t& term, u32 n = 1);

        u0 cursor_prev_line(term_t& term, u32 n = 1);

        u0 cursor_horiz_abs(term_t& term, u32 n = 1);

        u0 erase_line(term_t& term, clear_mode_t mode);

        constexpr rgb_t rgb(u8 red, u8 green, u8 blue) {
            return rgb_t{red, green, blue, 0};
        }

        u0 erase_display(term_t& term, clear_mode_t mode);

        u0 set_fg(term_t& term,
                  rgb_t color,
                  color_mode_t mode = color_mode_t::palette);

        u0 set_bg(term_t& term,
                  rgb_t color,
                  color_mode_t mode = color_mode_t::palette);

        u0 cursor_horz_vert_pos(term_t& term, cursor_pos_t pos);

        u0 init(term_t& term, alloc_t* alloc = context::top()->alloc.main);
    }
}
