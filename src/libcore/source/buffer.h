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

#include <basecode/core/types.h>
#include <basecode/core/string/str.h>

#define CURSOR(b)   (b.data[b.idx])
#define PEEK(b, c)  (b.data[b.idx + c])

namespace basecode::source {
    struct buffer_t final {
        u64                     idx;
        u8*                     data;
        u32                     line;
        u64                     length;
        u32                     column;
        alloc_t*                allocator;
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

        status_t load(const string_t& path, buffer_t& buf);

        buffer_t make(alloc_t* allocator = context::top()->alloc);

        u0 init(buffer_t& buf, alloc_t* allocator = context::top()->alloc);
    }
}

