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

#include <basecode/core/defer.h>
#include <basecode/core/buffer.h>

namespace basecode::buffer {
    u0 free(buffer_t& buf) {
        memory::free(buf.alloc, buf.data);
    }

    b8 has_more(buffer_t& buf) {
        return buf.idx < buf.length;
    }

    u0 prev_char(buffer_t& buf) {
        buf.idx--;
        buf.column--;
    }

    u0 next_char(buffer_t& buf) {
        buf.idx++;
        buf.column++;
    }

    u0 next_line(buffer_t& buf) {
        buf.line++;
        buf.column = 0;
    }

    buffer_t make(alloc_t* alloc) {
        buffer_t buf{};
        init(buf, alloc);
        return buf;
    }

    u0 init(buffer_t& buf, alloc_t* alloc) {
        buf.data = {};
        buf.alloc = alloc;
        buf.idx = buf.length = {};
        buf.column = buf.line = {};
    }

    status_t load(const str_t& path, buffer_t& buf) {
        auto file = fopen(str::c_str(const_cast<str_t&>(path)), "rb");
        if (!file)
            return status_t::unable_to_open_file;
        defer(fclose(file));

        fseek(file, 0, SEEK_END);
        buf.length = ftell(file) + 1;
        fseek(file, 0, SEEK_SET);

        buf.data = (u8*) memory::alloc(context::top()->alloc, buf.length);
        fread(buf.data, 1, buf.length, file);
        buf.data[buf.length - 1] = 255;

        return status_t::ok;
    }

    b8 each_line(const buffer_t& buf, const line_callback_t& cb, str::slice_t sep) {
        s32 i{}, start_pos{};
        while (i < buf.length) {
            if (std::memcmp(buf.data + i, sep.data, sep.length) == 0) {
                str::slice_t line;
                line.data = buf.data + start_pos;
                line.length = (i - start_pos) - 1;
                if (!cb(line))
                    return false;
                start_pos = i + sep.length;
            }
            ++i;
        }
        return 0;
    }
}
