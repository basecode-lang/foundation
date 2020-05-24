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

#include <basecode/core/path.h>

#define CRSR_POS(c)      (c.pos)
#define CRSR_NEXT(c)     (c.pos++)
#define CRSR_READ(c)     (c.buf->data[c.pos])
#define CRSR_MORE(c)     (c.pos < c.buf->length)

namespace basecode {
    struct buf_line_t final {
        u32                     pos;
        u32                     len;
    };

    struct buf_t final {
        alloc_t*                alloc;
        u8*                     data;
        array_t<buf_line_t>     lines;
        u32                     length;
        u32                     capacity;
    };

    struct buf_crsr_t final {
        buf_t*                  buf;
        u32                     pos;
    };

    namespace buf {
        enum class status_t : u8 {
            ok,
            unable_to_open_file,
        };

        namespace cursor {
            buf_crsr_t make(buf_t& buf);

            u0 init(buf_crsr_t& crsr, buf_t& buf);
        }

        u0 free(buf_t& buf);

        u0 reset(buf_t& buf);

        u0 index(buf_t& buf);

        u0 reserve(buf_t& buf, u32 new_capacity);

        status_t load(buf_t& buf, const path_t& path);

        buf_t make(alloc_t* alloc = context::top()->alloc);

        u0 write(buf_t& buf, u32 offset, FILE* file, u32 length);

        u0 init(buf_t& buf, alloc_t* alloc = context::top()->alloc);

        u0 write(buf_t& buf, u32 offset, const u8* data, u32 length);

        status_t save(buf_t& buf, const path_t& path, u32 offset = {}, u32 length = {});

        b8 each_line(const buf_t& buf, const line_callback_t& cb, str::slice_t sep = "\n"_ss);
    }
}

