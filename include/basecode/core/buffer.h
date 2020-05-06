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

#include <basecode/core/str.h>

#define CURSOR(b)   (b.data[b.idx])
#define PEEK(b, c)  (b.data[b.idx + c])

namespace basecode {
    struct buffer_t final {
        alloc_t*                alloc;
        u8*                     data;
        u64                     idx;
        u64                     length;
        u32                     line;
        u32                     column;
    };

    namespace buffer {
        enum class status_t : u8 {
            ok,
            unable_to_open_file,
        };

        u0 free(buffer_t& buf);

        b8 has_more(buffer_t& buf);

        u0 prev_char(buffer_t& buf);

        u0 next_char(buffer_t& buf);

        u0 next_line(buffer_t& buf);

        status_t load(const str_t& path, buffer_t& buf);

        buffer_t make(alloc_t* alloc = context::top()->alloc);

        u0 init(buffer_t& buf, alloc_t* alloc = context::top()->alloc);

        b8 each_line(const buffer_t& buf, const line_callback_t& cb, str::slice_t sep = "\n"_ss);
    }
}

