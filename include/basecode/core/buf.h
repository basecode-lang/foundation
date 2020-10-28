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
#include <basecode/core/defer.h>

#define CRSR_POS(c)             ((c).pos)
#define CRSR_NEXT(c)            SAFE_SCOPE(++(c).pos; ++(c).column;)
#define CRSR_READ(c)            ((c).buf->data[(c).pos])
#define CRSR_PEEK(c)            ((c).buf->data[(c).pos + 1])
#define CRSR_MORE(c)            ((c).pos < (c).buf->length)
#define CRSR_NEWLINE(c)         SAFE_SCOPE((c).column = 0; ++(c).line;)

namespace basecode {
    struct buf_line_t final {
        u32                     pos;
        u32                     len;
    };

    struct buf_token_t final {
        u32                     pos;
        u32                     len;
    };

    struct buf_t final {
        alloc_t*                alloc;
        u8*                     data;
        array_t<buf_line_t>     lines;
        array_t<buf_token_t>    tokens;
        u32                     length;
        u32                     capacity;
    };

    struct buf_crsr_t final {
        buf_t*                  buf;
        u32                     pos;
        u32                     line;
        u32                     column;
    };

    namespace buf {
        enum class status_t : u8 {
            ok                  = 0,
            unable_to_open_file = 100,
        };

        u0 zero_fill(buf_t& buf, u32 offset, u32 length);

        u0 write(buf_t& buf, u32 offset, FILE* file, u32 length);

        u0 write(buf_t& buf, u32 offset, const u8* data, u32 length);

        namespace cursor {
            buf_crsr_t make(buf_t& buf);

            u0 seek(buf_crsr_t& crsr, u32 offset);

            u0 init(buf_crsr_t& crsr, buf_t& buf);

            u0 write_u8(buf_crsr_t& crsr, u8 value);

            u0 write_u16(buf_crsr_t& crsr, u16 value);

            u0 write_s16(buf_crsr_t& crsr, s16 value);

            u0 write_u32(buf_crsr_t& crsr, u32 value);

            u0 write_u64(buf_crsr_t& crsr, u64 value);

            u0 write_str(buf_crsr_t& crsr, const String_Concept auto& str) {
                write(*crsr.buf, crsr.pos, str.data, str.length);
                crsr.pos += str.length;
            }
        }

        u0 free(buf_t& buf);

        u0 reset(buf_t& buf);

        u0 index(buf_t& buf);

        str::slice_t line(buf_t& buf, u32 idx);

        u0 reserve(buf_t& buf, u32 new_capacity);

        status_t load(buf_t& buf, const path_t& path);

        buf_t make(alloc_t* alloc = context::top()->alloc);

        u0 init(buf_t& buf, alloc_t* alloc = context::top()->alloc);

        status_t load(buf_t& buf, const String_Concept auto& value) {
            auto file = ::fmemopen((u0*) value.data, value.length, "r");
            if (!file)
                return status_t::unable_to_open_file;
            defer(::fclose(file));
            write(buf, 0, file, value.length);
            return status_t::ok;
        }

        status_t save(buf_t& buf, const path_t& path, u32 offset = {}, u32 length = {});

        b8 each_line(const buf_t& buf, const line_callback_t& cb, str::slice_t sep = "\n"_ss);
    }
}

