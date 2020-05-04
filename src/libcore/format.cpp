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

#include "format.h"

namespace basecode::format {
    u0 hex_dump(memory_buffer_t& buf, const u0* data, u32 size) {
        const u8* bytes = (const u8*) data;
        s32 i, j;
        for (i = 0; i < size; i += 16) {
            format::format_to(buf, "${:016x}:{:08x}: ", (u64) (bytes + i), i);
            for (j = 0; j < 16; j++) {
                if (i + j < size)
                    format::format_to(buf, "{:02x} ", bytes[i + j]);
                else
                    format::format_to(buf, "   ");
            }
            format::format_to(buf, " ");
            for (j = 0; j < 16; j++) {
                if (i + j < size) {
                    format::format_to(
                        buf,
                        "{}",
                        (s8) (isprint(bytes[i + j]) ? bytes[i + j] : '.'));
                }
            }
            format::format_to(buf, "\n");
        }
    }

    string_t vformat(alloc_t* allocator, fmt::string_view format_str, fmt::format_args args) {
        format::allocator_t alloc(allocator);
        memory_buffer_t buf(alloc);
        fmt::vformat_to(buf, format_str, args);
        return to_string(buf);
    }

    u0 vprint(alloc_t* allocator, FILE* file, fmt::string_view format_str, fmt::format_args args) {
        format::allocator_t alloc(allocator);
        memory_buffer_t buf(alloc);
        fmt::vformat_to(buf, format_str, args);
        std::fwrite(buf.data(), 1, buf.size(), file);
    }
}
