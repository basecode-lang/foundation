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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/buf.h>
#include <basecode/core/format.h>

namespace basecode {
    static inline str::slice_t s_byte_units[] = {
        "bytes"_ss,
        "KB"_ss,
        "MB"_ss,
        "GB"_ss,
        "TB"_ss,
        "PB"_ss,
        "EB"_ss,
        "ZB"_ss,
        "YB"_ss
    };

    // ------------------------------------------------------------------------

    str_buf_t::~str_buf_t() {
        _str->length = size();
    }

    u0 str_buf_t::grow(usize capacity) {
        str::grow(*_str, capacity);
        set((s8*) _str->data, _str->capacity);
    }

    str_buf_t::str_buf_t(str_t* str) : _str(str) {
        set((s8*) _str->data, _str->capacity);
    }

    str_buf_t::str_buf_t(str_buf_t&& other) FMT_NOEXCEPT {
        _str->operator=(*other._str);
    }

    str_buf_t& str_buf_t::operator=(str_buf_t&& other) FMT_NOEXCEPT {
        _str->operator=(*other._str);
        return *this;
    }

    // ------------------------------------------------------------------------

    mem_buf_t::~mem_buf_t() {
        _buf->length = size();
    }

    u0 mem_buf_t::grow(usize capacity) {
        buf::grow(*_buf, capacity);
        set((s8*) _buf->data, _buf->capacity);
    }

    mem_buf_t::mem_buf_t(buf_t* buf) : _buf(buf) {
        set((s8*) _buf->data, _buf->capacity);
    }

    mem_buf_t::mem_buf_t(mem_buf_t&& other) FMT_NOEXCEPT {
        _buf->operator=(*other._buf);
    }

    mem_buf_t& mem_buf_t::operator=(mem_buf_t&& other) FMT_NOEXCEPT {
        _buf->operator=(*other._buf);
        return *this;
    }

    // ------------------------------------------------------------------------

    namespace format {
        u0 print_hex_dump(const u0* data,
                          u32 size,
                          b8 show_address,
                          b8 show_offset,
                          u32 indent) {
            const u8* bytes = (const u8*) data;
            for (u32 i = 0; i < size; i += 16) {
                if (indent > 0)
                    format::print("{:<{}}", " ", indent);
                if (show_address)
                    format::print("${:016x}: ", (u64) (bytes + i));
                else if (show_offset)
                    format::print("{:08x}: ", i);
                for (u32 j = 0; j < 16; j++) {
                    if (i + j < size)
                        format::print("{:02x} ", bytes[i + j]);
                    else
                        format::print("   ");
                }
                format::print(" ");
                for (u32 j = 0; j < 16; j++) {
                    if (i + j < size) {
                        format::print(
                            "{}",
                            (s8) (isprint(bytes[i + j]) ?
                                  bytes[i + j] : '.'));
                    }
                }
                format::print("\n");
            }
        }

        u0 format_hex_dump(str_buf_t& buf,
                           const u0* data,
                           u32 size,
                           b8 show_address,
                           b8 show_offset,
                           u32 indent) {
            const u8* bytes = (const u8*) data;
            for (u32 i = 0; i < size; i += 16) {
                if (indent > 0)
                    format::print("{:<{}}", " ", indent);
                if (show_address)
                    format::format_to(buf, "${:016x}: ", (u64) (bytes + i));
                else if (show_offset)
                    format::format_to(buf, "{:08x}: ", i);
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
                            (s8) (isprint(bytes[i + j]) ?
                                  bytes[i + j] : '.'));
                    }
                }
                format::format_to(buf, "\n");
            }
        }

        u0 vprint(alloc_t* alloc,
                  FILE* file,
                  fmt_str_t format_str,
                  fmt::format_args args) {
            fmt_alloc_t fmt_alloc(alloc);
            fmt_buf_t buf(fmt_alloc);
            fmt::vformat_to(buf, format_str, args);
            std::fwrite(buf.data(), 1, buf.size(), file);
        }

        str_t vformat(alloc_t* alloc,
                      fmt_str_t format_str,
                      fmt::format_args args) {
            fmt_alloc_t fmt_alloc(alloc);
            fmt_buf_t buf(fmt_alloc);
            fmt::vformat_to(buf, format_str, args);
            return to_string(buf);
        }

        str::slice_t byte_unit_name(u32 idx) {
            return s_byte_units[idx];
        }
    }
}

