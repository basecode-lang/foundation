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

#include <atomic>
#include <basecode/core/term.h>

namespace basecode::term {
    struct system_t final {
        alloc_t*                alloc;
        std::atomic<b8>         enabled;
    };

    system_t                    g_term_sys{};

    namespace system {
        u0 fini() {
        }

        u0 enabled(b8 enabled) {
            g_term_sys.enabled = enabled;
        }

        status_t init(b8 enabled, alloc_t* alloc) {
            g_term_sys.alloc   = alloc;
            g_term_sys.enabled = enabled;
#ifdef _WIN32
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD dwMode{};
            GetConsoleMode(hOut, &dwMode);
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
#endif
            return status_t::ok;
        }
    }

    u0 colorize_range(str_buf_t& str_buf,
                      color_t fg,
                      color_t bg,
                      buf_t* buf,
                      u32 line_number,
                      u32 begin,
                      u32 end) {
        const auto& line = buf->lines[line_number];
        const auto color_range_len = end - begin;
        if (!g_term_sys.enabled
        ||  line.pos > end) {
            format::format_to(
                str_buf,
                "{}",
                slice::make(buf->data + line.pos, line.len));
            return;
        }
        format::format_to(
            str_buf,
            "{}",
            slice::make(buf->data + line.pos, (begin - line.pos) - 1));
        set_fg(str_buf, fg);
        set_bg(str_buf, bg);
        format::format_to(
            str_buf,
            "{}",
            slice::make(buf->data + begin - 1, color_range_len + 1));
        reset_all(str_buf);
        s32 end_len;
        if (line.pos + line.len < end) {
            end_len = end - (line.pos + line.len);
        } else {
            end_len = (line.pos + line.len) - end;
        }
        format::format_to(str_buf,
                          "{}",
                          slice::make(buf->data + end, end_len));
    }

    u0 reset_all(str_buf_t& str_buf) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[0m");
    }

    u0 cursor_up(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}A", n);
    }

    u0 scroll_up(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}S", n);
    }

    u0 scroll_down(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}T", n);
    }

    u0 cursor_back(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}D", n);
    }

    u0 cursor_down(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}B", n);
    }

    u0 set_fg(str_buf_t& str_buf, color_t fg) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}m", u32(fg));
    }

    u0 set_bg(str_buf_t& str_buf, color_t bg) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}m", u32(bg) + 10);
    }

    cursor_pos_t cursor_pos(str_buf_t& str_buf) {
        cursor_pos_t pos{};
        return pos;
    }

    u0 cursor_forward(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}C", n);
    }

    u0 cursor_next_line(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}E", n);
    }

    u0 cursor_prev_line(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}F", n);
    }

    u0 set_style(str_buf_t& str_buf, style_t style) {
        if (!g_term_sys.enabled) return;
        if (!style) {
            if (!g_term_sys.enabled) return;
            format::format_to(str_buf, "\033[0m");
            return;
        }
        for (u32 bit = 0; bit < 21; ++bit) {
            if (bit == 9) {
                bit = 19;
                continue;
            }
            const auto mask = 1U << bit;
            if ((style & mask) == mask) {
                format::format_to(str_buf, "\033[{};49m", bit + 1);
            } else {
                const auto code = bit + 21;
                format::format_to(str_buf, "\033[{};49m", code);
            }
        }
    }

    u0 cursor_to(str_buf_t& str_buf, cursor_pos_t pos) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{};{}H", pos.row, pos.col);
    }

    u0 erase_line(str_buf_t& str_buf, clear_mode_t mode) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}K", u32(mode));
    }

    u0 erase_display(str_buf_t& str_buf, clear_mode_t mode) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}J", u32(mode));
    }

    u0 cursor_horizontal_absolute(str_buf_t& str_buf, u32 n) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{}G", n);
    }

    u0 set_fg(str_buf_t& str_buf, rgb_t color, color_mode_t mode) {
        if (!g_term_sys.enabled) return;
        if (mode == color_mode_t::palette) {
            format::format_to(str_buf,
                              "\033[ 38;5;{}",
                              color.palette_value());
        } else {
            format::format_to(str_buf,
                              "\033[ 38;2;{};{};{}",
                              color.r,
                              color.g,
                              color.b);
        }
    }

    u0 set_bg(str_buf_t& str_buf, rgb_t color, color_mode_t mode) {
        if (!g_term_sys.enabled) return;
        if (mode == color_mode_t::palette) {
            format::format_to(str_buf,
                              "\033[ 48;5;{}",
                              color.palette_value());
        } else {
            format::format_to(str_buf,
                              "\033[ 48;2;{};{};{}",
                              color.r,
                              color.g,
                              color.b);
        }
    }

    u0 cursor_horz_vert_pos(str_buf_t& str_buf, cursor_pos_t pos) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\033[{};{}f", pos.row, pos.col);
    }
}
