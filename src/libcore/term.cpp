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
            return status_t::ok;
        }
    }

    u0 colorize_range(str_buf_t& str_buf,
                      term_color_t fg,
                      term_color_t bg,
                      buf_t* buf,
                      u32 line_number,
                      u32 begin,
                      u32 end) {
        const auto& line = buf->lines[line_number];
        const auto color_range_len = end - begin;
        if (!g_term_sys.enabled) {
            format::format_to(str_buf, "{}", slice::make(buf->data + line.pos, line.len));
            return;
        }
        format::format_to(str_buf, "{}", slice::make(buf->data + line.pos, (begin - line.pos) - 1));
        emit_color_code(str_buf, fg, bg);
        format::format_to(str_buf, "{}", slice::make(buf->data + begin - 1, color_range_len + 1));
        emit_color_reset(str_buf);
        s32 end_len;
        if (line.pos + line.len < end) {
            end_len = end - (line.pos + line.len);
        } else {
            end_len = (line.pos + line.len) - end;
        }
        format::format_to(str_buf, "{}", slice::make(buf->data + end, end_len));
    }

    u0 emit_color_reset(str_buf_t& str_buf) {
        if (!g_term_sys.enabled) return;
        format::format_to(str_buf, "\x1b[0m");
    }

    u0 emit_color_code(str_buf_t& str_buf, term_color_t fg, term_color_t bg) {
        if (!g_term_sys.enabled) return;
        const u32 fg_ = u32(fg);
        const u32 bg_ = u32(bg) + 10;
        format::format_to(str_buf, "\x1b[{};{}m", fg_, bg_);
    }
}
