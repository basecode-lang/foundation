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
    enum class term_color_t : u8 {
        black = 30,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white,
        default_,

        bright_black = 90,
        bright_red,
        bright_green,
        bright_yellow,
        bright_blue,
        bright_magenta,
        bright_cyan,
        bright_white
    };

    namespace term {
        enum class status_t : u8 {
            ok
        };

        namespace system {
            u0 fini();

            u0 enabled(b8 enabled);

            status_t init(b8 enabled, alloc_t* alloc = context::top()->alloc);
        }

        u0 colorize_range(str_buf_t& str_buf,
                          term_color_t fg,
                          term_color_t bg,
                          buf_t* buf,
                          u32 line_number,
                          u32 begin,
                          u32 end);

        u0 emit_color_reset(str_buf_t& str_buf);

        u0 emit_color_code(str_buf_t& str_buf, term_color_t fg, term_color_t bg);
    }
}
