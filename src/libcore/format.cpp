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

#include <basecode/core/format.h>

namespace basecode::format {
    static str::slice_t s_byte_units[] = {
        "bytes"_ss, "KB"_ss, "MB"_ss, "GB"_ss, "TB"_ss,
        "PB"_ss,    "EB"_ss, "ZB"_ss, "YB"_ss
    };

    u0 unitized_byte_size(fmt_buf_t& buf, u64 size) {
        u64 i{};
        while (size > 1024) {
            size /= 1024;
            i++;
        }
        if (i > 1) {
            format::format_to(buf, "{}.{}{}", i, size, s_byte_units[i]);
        } else {
            format::format_to(buf, "{}", size, s_byte_units[i]);
        }
    }

    u0 hex_dump(fmt_buf_t& buf, const u0* data, u32 size) {
        const u8* bytes = (const u8*) data;
        for (u32 i = 0; i < size; i += 16) {
            format::format_to(buf, "${:016x}:{:08x}: ", (u64) (bytes + i), i);
            for (u32 j = 0; j < 16; j++) {
                if (i + j < size)
                    format::format_to(buf, "{:02x} ", bytes[i + j]);
                else
                    format::format_to(buf, "   ");
            }
            format::format_to(buf, " ");
            for (u32 j = 0; j < 16; j++) {
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

    str_t vformat(alloc_t* alloc, fmt::string_view format_str, fmt::format_args args) {
        fmt_alloc_t fmt_alloc(alloc);
        fmt_buf_t buf(fmt_alloc);
        fmt::vformat_to(buf, format_str, args);
        return to_string(buf);
    }

    u0 vprint(alloc_t* alloc, FILE* file, fmt::string_view format_str, fmt::format_args args) {
        fmt_alloc_t fmt_alloc(alloc);
        fmt_buf_t buf(fmt_alloc);
        fmt::vformat_to(buf, format_str, args);
        std::fwrite(buf.data(), 1, buf.size(), file);
    }
}
