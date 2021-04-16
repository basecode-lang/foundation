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
        std::atomic<b8>         redirected;
    };

    system_t                    g_term_sys{};

    constexpr u32 num_style_bits    = 10;
    static u32 s_style_bits[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 21
    };

    namespace system {
        u0 fini() {
        }

        u0 enabled(b8 enabled) {
            g_term_sys.enabled = enabled;
        }

        status_t init(b8 enabled, alloc_t* alloc) {
            g_term_sys.alloc   = alloc;
#ifdef _WIN32
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD dwMode{};
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
                g_term_sys.redirected = false;
            } else {
                g_term_sys.redirected = true;
            }
#else
            g_term_sys.redirected = !isatty(fileno(stdout));
#endif
            g_term_sys.enabled = enabled && !g_term_sys.redirected;
            return status_t::ok;
        }
    }

    u0 free(term_t& term) {
        array::free(term.commands);
    }

    u0 colorize_range(term_t& term,
                      str_buf_t& str_buf,
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
        set_fg(term, fg);
        set_bg(term, bg);
        refresh(term, str_buf);
        format::format_to(
            str_buf,
            "{}",
            slice::make(buf->data + begin - 1, color_range_len + 1));
        reset_all(term);
        refresh(term, str_buf);
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

    u0 reset_all(term_t& term) {
        auto& cmd = array::append(term.commands);
        cmd.params.sgr.mode = 0;
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 get_cursor_pos(term_t& term) {
    }

    u0 cursor_up(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_up;
    }

    u0 scroll_up(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::scroll_up;
    }

    u0 scroll_down(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::scroll_down;
    }

    u0 cursor_back(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_back;
    }

    u0 cursor_down(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_down;
    }

    u0 set_fg(term_t& term, color_t fg) {
        auto& cmd = array::append(term.commands);
        cmd.params.sgr.mode = u32(fg);
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 set_bg(term_t& term, color_t bg) {
        auto& cmd = array::append(term.commands);
        auto bg_num = u32(bg);
        if (bg_num < u32(color_t::bg_black))
            bg_num += 10;
        cmd.params.sgr.mode = bg_num;
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 init(term_t& term, alloc_t* alloc) {
        term.alloc          = alloc;
        term.dirty          = false;
        term.style          = {};
        term.cursor         = {};
        term.enabled        = g_term_sys.enabled;
        term.fg.mode        = color_mode_t::indexed;
        term.bg.mode        = color_mode_t::indexed;
        term.fg.value.index = color_t::fg_default;
        term.bg.value.index = color_t::bg_default;
        array::init(term.commands, term.alloc);
    }

    u0 cursor_forward(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_forward;
    }

    u0 refresh(term_t& term, str_buf_t& buf) {
        if (!term.enabled) {
            array::reset(term.commands);
            term.dirty = false;
            return;
        }
        for (const auto& cmd : term.commands) {
            switch (cmd.type) {
                case term_command_type_t::cursor_up:
                case term_command_type_t::scroll_up:
                case term_command_type_t::cursor_down:
                case term_command_type_t::scroll_down:
                case term_command_type_t::cursor_back:
                case term_command_type_t::cursor_forward:
                case term_command_type_t::cursor_next_line:
                case term_command_type_t::cursor_prev_line:
                case term_command_type_t::cursor_horiz_abs:
                    format::format_to(buf,
                                      "\033[{}{}",
                                      cmd.params.edit.n,
                                      s8(cmd.type));
                    break;
                case term_command_type_t::cursor_to:
                case term_command_type_t::cursor_horiz_vert_pos:
                    term.cursor = cmd.params.pos;
                    format::format_to(buf,
                                      "\033[{};{}H",
                                      term.cursor.row,
                                      term.cursor.col);
                    break;
                case term_command_type_t::erase_line:
                case term_command_type_t::erase_display:
                    format::format_to(buf,
                                      "\033[{}K",
                                      u32(cmd.params.clear_mode));
                    break;
                case term_command_type_t::select_graphic_rendition: {
                    auto sgr = &cmd.params.sgr;
                    switch (cmd.params.sgr.mode) {
                        case 0:
                            term.style          = style::none;
                            term.fg.mode        = color_mode_t::indexed;
                            term.bg.mode        = color_mode_t::indexed;
                            term.fg.value.index = color_t::fg_default;
                            term.bg.value.index = color_t::bg_default;
                            format::format_to(buf, "\033[m");
                            break;
                        case 21:
                        case 1 ... 9: {
                            for (u32 i = 0; i < num_style_bits; ++i) {
                                const auto bit  = s_style_bits[i] - 1;
                                const auto mask = 1U << bit;
                                b8 is_set = (term.style & mask) == mask;
                                if ((sgr->mask & mask) == mask) {
                                    if (!is_set) {
                                        format::format_to(
                                            buf,
                                            "\033[{}m",
                                            bit + 1);
                                        term.style |= mask;
                                    }
                                } else {
                                    if (is_set) {
                                        auto bit2 = bit + 1;
                                        if (bit2 == 1)          bit2 = 22;
                                        else if (bit2 == 6)     bit2 = 25;
                                        else                    bit2 += 20;
                                        format::format_to(
                                            buf,
                                            "\033[{}m",
                                            bit2);
                                        term.style &= ~mask;
                                    }
                                }
                            }
                            break;
                        }
                        case 39:
                        case 30 ... 37: {
                            const auto index = color_t(sgr->mode);
                            if (term.fg.mode != color_mode_t::indexed
                            ||  index != term.fg.value.index) {
                                term.fg.mode = color_mode_t::indexed;
                                term.fg.value.index = index;
                                format::format_to(buf,
                                                  "\033[{}m",
                                                  u32(term.fg.value.index));
                            }
                            break;
                        }
                        case 38:
                            if (sgr->fg == term.fg.value.triplet)
                                break;
                            term.fg.value.triplet = sgr->fg;
                            term.fg.mode = color_mode_t(sgr->mask);
                            if (term.fg.mode == color_mode_t::palette) {
                                format::format_to(
                                    buf,
                                    "\033[ 38;5;{} m",
                                    term.fg.value.triplet.palette_value());
                            } else {
                                format::format_to(
                                    buf,
                                    "\033[ 38;2;{};{};{} m",
                                    term.fg.value.triplet.r,
                                    term.fg.value.triplet.g,
                                    term.fg.value.triplet.b);
                            }
                            break;
                        case 49:
                        case 40 ... 47: {
                            const auto index = color_t(sgr->mode);
                            if (term.bg.mode != color_mode_t::indexed
                            ||  index != term.bg.value.index) {
                                term.bg.mode = color_mode_t::indexed;
                                term.bg.value.index = index;
                                format::format_to(buf,
                                                  "\033[{}m",
                                                  u32(term.bg.value.index));
                            }
                            break;
                        }
                        case 48:
                            if (sgr->bg == term.bg.value.triplet)
                                break;
                            term.bg.value.triplet = sgr->bg;
                            term.bg.mode = color_mode_t(sgr->mask);
                            if (term.bg.mode == color_mode_t::palette) {
                                format::format_to(
                                    buf,
                                    "\033[ 48;5;{} m",
                                    term.bg.value.triplet.palette_value());
                            } else {
                                format::format_to(
                                    buf,
                                    "\033[ 48;2;{};{};{} m",
                                    term.bg.value.triplet.r,
                                    term.bg.value.triplet.g,
                                    term.bg.value.triplet.b);
                            }
                            break;
                    }
                    break;
                }
            }
        }
        array::reset(term.commands);
    }

    u0 cursor_next_line(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_next_line;
    }

    u0 cursor_horiz_abs(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_horiz_abs;
    }

    u0 cursor_prev_line(term_t& term, u32 n) {
        auto& cmd = array::append(term.commands);
        cmd.params.edit.n = n;
        cmd.type = term_command_type_t::cursor_prev_line;
    }

    u0 set_style(term_t& term, style_t style) {
        auto& cmd = array::append(term.commands);
        cmd.params.sgr.mask = style;
        cmd.params.sgr.mode = 1;
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 cursor_to(term_t& term, cursor_pos_t pos) {
        auto& cmd = array::append(term.commands);
        cmd.params.pos = pos;
        cmd.type = term_command_type_t::cursor_to;
    }

    u0 erase_line(term_t& term, clear_mode_t mode) {
        auto& cmd = array::append(term.commands);
        cmd.params.clear_mode = mode;
        cmd.type = term_command_type_t::erase_line;
    }

    u0 erase_display(term_t& term, clear_mode_t mode) {
        auto& cmd = array::append(term.commands);
        cmd.params.clear_mode = mode;
        cmd.type = term_command_type_t::erase_display;
    }

    u0 set_fg(term_t& term, rgb_t color, color_mode_t mode) {
        auto& cmd = array::append(term.commands);
        cmd.params.sgr.mode = 38;
        cmd.params.sgr.mask = u32(mode);
        cmd.params.sgr.fg   = color;
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 set_bg(term_t& term, rgb_t color, color_mode_t mode) {
        auto& cmd = array::append(term.commands);
        cmd.params.sgr.mode = 48;
        cmd.params.sgr.mask = u32(mode);
        cmd.params.sgr.bg   = color;
        cmd.type = term_command_type_t::select_graphic_rendition;
    }

    u0 cursor_horz_vert_pos(term_t& term, cursor_pos_t pos) {
        auto& cmd = array::append(term.commands);
        cmd.params.pos = pos;
        cmd.type = term_command_type_t::cursor_horiz_vert_pos;
    }
}
