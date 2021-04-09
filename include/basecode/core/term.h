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

#pragma once

#include <basecode/core/buf.h>
#include <basecode/core/types.h>
#include <basecode/core/format.h>

namespace basecode {
    using style_t               = u32;

    enum class clear_mode_t : u8 {
        cursor_to_bottom,
        cursor_to_top,
        entire
    };

    enum class color_mode_t : u8 {
        palette,
        true_color
    };

    struct cursor_pos_t final {
        u32                     row;
        u32                     col;
    };

    namespace term {
        namespace style {
            constexpr u32 none              = 0b00000000000000000000000000000000;
            constexpr u32 bold              = 0b00000000000000000000000000000001;
            constexpr u32 dim               = 0b00000000000000000000000000000010;
            constexpr u32 italic            = 0b00000000000000000000000000000100;
            constexpr u32 underline         = 0b00000000000000000000000000001000;
            constexpr u32 slow_blink        = 0b00000000000000000000000000010000;
            constexpr u32 fast_blink        = 0b00000000000000000000000000100000;
            constexpr u32 reverse           = 0b00000000000000000000000001000000;
            constexpr u32 hidden            = 0b00000000000000000000000010000000;
            constexpr u32 strike            = 0b00000000000000000000000100000000;
            constexpr u32 double_underline  = 0b00000000000100000000000000000000;
        }

        enum class color_t : u8 {
            black = 30,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,

            reset,

            bright_black = 90,
            bright_red,
            bright_green,
            bright_yellow,
            bright_blue,
            bright_magenta,
            bright_cyan,
            bright_white
        };

        struct rgb_t final {
            u32                     r:      8;
            u32                     g:      8;
            u32                     b:      8;
            u32                     pad:    8;

            u32 palette_value() const {
                return (16 + 36 * r) + (6 * g) + b;
            }
        };

        enum class status_t : u8 {
            ok
        };

        namespace system {
            u0 fini();

            u0 enabled(b8 enabled);

            status_t init(b8 enabled, alloc_t* alloc = context::top()->alloc);
        }

        u0 colorize_range(str_buf_t& str_buf,
                          color_t fg,
                          color_t bg,
                          buf_t* buf,
                          u32 line_number,
                          u32 begin,
                          u32 end);

        u0 reset_all(str_buf_t& str_buf);

        u0 set_fg(str_buf_t& str_buf, color_t fg);

        u0 set_bg(str_buf_t& str_buf, color_t bg);

        cursor_pos_t cursor_pos(str_buf_t& str_buf);

        u0 cursor_up(str_buf_t& str_buf, u32 n = 1);

        u0 scroll_up(str_buf_t& str_buf, u32 n = 1);

        u0 scroll_down(str_buf_t& str_buf, u32 n = 1);

        u0 cursor_back(str_buf_t& str_buf, u32 n = 1);

        u0 cursor_down(str_buf_t& str_buf, u32 n = 1);

        u0 set_style(str_buf_t& str_buf, style_t style);

        constexpr rgb_t rgb(u8 red, u8 green, u8 blue) {
            return rgb_t{red, green, blue, 0};
        }

        u0 cursor_forward(str_buf_t& str_buf, u32 n = 1);

        u0 cursor_to(str_buf_t& str_buf, cursor_pos_t pos);

        u0 cursor_next_line(str_buf_t& str_buf, u32 n = 1);

        u0 cursor_prev_line(str_buf_t& str_buf, u32 n = 1);

        u0 set_fg(str_buf_t& str_buf,
                  rgb_t color,
                  color_mode_t mode = color_mode_t::palette);

        u0 set_bg(str_buf_t& str_buf,
                  rgb_t color,
                  color_mode_t mode = color_mode_t::palette);

        u0 erase_line(str_buf_t& str_buf, clear_mode_t mode);

        u0 erase_display(str_buf_t& str_buf, clear_mode_t mode);

        u0 cursor_horizontal_absolute(str_buf_t& str_buf, u32 n = 1);

        u0 cursor_horz_vert_pos(str_buf_t& str_buf, cursor_pos_t pos);
    }
}
