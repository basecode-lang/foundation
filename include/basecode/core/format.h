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

#pragma once

#include <basecode/core/str.h>
#include <basecode/core/types.h>
#include <basecode/core/context.h>
#include <basecode/core/format_fwd.h>
#include <basecode/core/memory/std_alloc.h>

namespace basecode {
    class mem_buf_t : public fmt::detail::buffer<s8> {
        buf_t*                  _buf;
    public:
        ~mem_buf_t();

        explicit mem_buf_t(buf_t* buf);

        mem_buf_t(mem_buf_t&& other) FMT_NOEXCEPT;

        mem_buf_t& operator=(mem_buf_t&& other) FMT_NOEXCEPT;

    protected:
        u0 grow(usize capacity) FMT_OVERRIDE;
    };

    class str_buf_t : public fmt::detail::buffer<s8> {
        str_t*                  _str;
    public:
        ~str_buf_t();

        explicit str_buf_t(str_t* str);

        str_buf_t(str_buf_t&& other) FMT_NOEXCEPT;

        str_buf_t& operator=(str_buf_t&& other) FMT_NOEXCEPT;

    protected:
        u0 grow(usize capacity) FMT_OVERRIDE;
    };

    namespace format {
        u0 vprint(alloc_t* alloc,
                  FILE* file,
                  fmt_str_t format_str,
                  fmt_args_t args);

        u0 print_hex_dump(const u0* data,
                          u32 size,
                          b8 show_address = true,
                          b8 show_offset = true,
                          u32 indent = 0);

        u0 format_hex_dump(str_buf_t& buf,
                           const u0* data,
                           u32 size,
                           b8 show_address = true,
                           b8 show_offset = true,
                           u32 indent = 0);

        template <typename Buffer, typename... Args>
        inline decltype(auto) format_to(Buffer& buf,
                                        fmt_str_t format_str,
                                        const Args&... args);

        str_t vformat(alloc_t* alloc, fmt_str_t format_str, fmt_args_t args);

        FORCE_INLINE str_t to_string(const fmt_buf_t& buf) {
            str_t str;
            str::init(str, buf.get_allocator().backing);
            str::reserve(str, buf.size());
            std::memcpy(str.data, buf.data(), buf.size());
            str.length = buf.size();
            return str;
        }

        template <typename... Args>
        inline u0 print(fmt_str_t format_str, const Args&... args) {
            vprint(context::top()->alloc.main,
                   stdout,
                   format_str,
                   fmt::make_format_args(args...));
        }

        FORCE_INLINE u0 to_string(const fmt_buf_t& buf, str_t& str) {
            str::reserve(str, buf.size());
            std::memcpy(str.data, buf.data(), buf.size());
            str.length = buf.size();
        }

        template <typename... Args>
        inline str_t format(fmt_str_t format_str,
                            const Args&... args) {
            return vformat(context::top()->alloc.main,
                           format_str,
                           fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(FILE* file,
                        fmt_str_t format_str,
                        const Args&... args) {
            vprint(context::top()->alloc.main,
                   file,
                   format_str,
                   fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(alloc_t* alloc,
                        fmt_str_t format_str,
                        const Args&... args) {
            vprint(alloc, stdout, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline str_t format(alloc_t* alloc,
                            fmt_str_t format_str,
                            const Args&... args) {
            return vformat(alloc, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(alloc_t* alloc,
                        FILE* file,
                        fmt_str_t format_str,
                        const Args&... args) {
            vprint(alloc, file, format_str, fmt::make_format_args(args...));
        }

        template <typename Buffer>
        inline u0 unitized_byte_size(Buffer& buf,
                                     f64 size) {
            static str::slice_t s_byte_units[] = {
                "bytes"_ss, "KB"_ss, "MB"_ss, "GB"_ss,
                "TB"_ss, "PB"_ss, "EB"_ss, "ZB"_ss, "YB"_ss
            };
            const f64 one_kb = 1_kb;
            str::slice_t* unit{};
            for (u32 i = 0; i < 9; ++i) {
                unit = &s_byte_units[i];
                if (size < 1024.0)
                    break;
                size /= one_kb;
            }
            format::format_to(buf, "{:.5g} {}", size, *unit);
        }

        template <typename... Args>
        inline u0 print_ellipsis(alloc_t* alloc,
                                 FILE* file,
                                 fmt_str_t label_str,
                                 u32 width,
                                 fmt_str_t format_str,
                                 const Args&... args) {
            vprint(alloc,
                   file,
                   "{} {:.<{}} ",
                   fmt::make_format_args(label_str,
                                         ".",
                                         width - label_str.size()));
            vprint(alloc,
                   file,
                   format_str,
                   fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print_ellipsis(fmt_str_t label_str,
                                 u32 width,
                                 fmt_str_t format_str,
                                 const Args&... args) {
            vprint(context::top()->alloc.main,
                   stdout,
                   "{} {:.<{}} ",
                   fmt::make_format_args(label_str,
                                         ".",
                                         width - label_str.size()));
            vprint(context::top()->alloc.main,
                   stdout,
                   format_str,
                   fmt::make_format_args(args...));
        }

        inline u0 to_radix(str_t& str,
                           Integer_Concept auto value,
                           Radix_Concept auto radix = 10) {
            str_buf_t buf(&str);
            switch (radix) {
                case 2:     format_to(buf, "0b{:b}", value);    break;
                case 8:     format_to(buf, "0o{:o}", value);    break;
                case 16:    format_to(buf, "0x{:x}", value);    break;
                default:    format_to(buf, "{}", value);        break;
            }
        }

        template <typename Buffer, typename... Args>
        inline decltype(auto) format_to(Buffer& buf,
                                        fmt_str_t format_str,
                                        const Args&... args) {
            return fmt::vformat_to(buf,
                                   format_str,
                                   fmt::make_format_args(args...));
        }

        FORCE_INLINE str_t to_radix(Integer_Concept auto value,
                                    Radix_Concept auto radix = 10,
                                    alloc_t* alloc = context::top()->alloc.main) {
            fmt_alloc_t fmt_alloc(alloc);
            fmt_buf_t buf(fmt_alloc);
            switch (radix) {
                case 2:     format_to(buf, "0b{:b}", value);    break;
                case 8:     format_to(buf, "0o{:o}", value);    break;
                case 16:    format_to(buf, "0x{:x}", value);    break;
                default:    format_to(buf, "{}", value);        break;
            }
            return to_string(buf);
        }
    }
}
