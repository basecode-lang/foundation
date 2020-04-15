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
#include "buffer.h"

namespace basecode::source::buffer {
    u0 free(buffer_t& buf) {
        memory::deallocate(buf.allocator, buf.data);
    }

    load_result_t load(
            const string::ascii_t& path,
            buffer_t& buf) {
        auto file = fopen((const char*) path.data, "rb");
        if (!file)
            return load_result_t::unable_to_open_file;
        defer(fclose(file));

        fseek(file, 0, SEEK_END);
        buf.length = ftell(file) + 1;
        fseek(file, 0, SEEK_SET);

        buf.data = (u8*) memory::allocate(
            context::current()->allocator,
            buf.length);
        fread(buf.data, 1, buf.length, file);
        buf.data[buf.length - 1] = 255;

        return load_result_t::ok;
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

    buffer_t make(memory::allocator_t* allocator) {
        buffer_t buf{};
        init(buf, allocator);
        return buf;
    }

    u0 init(buffer_t& buf, memory::allocator_t* allocator) {
        buf.data = {};
        buf.allocator = allocator;
        buf.idx = buf.length = {};
        buf.column = buf.line = {};
    }
}