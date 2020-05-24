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
#include <basecode/core/types.h>
#include <basecode/core/format_types.h>

namespace basecode {
    static inline str::slice_t s_byte_units[] = {
        "bytes"_ss, "KB"_ss, "MB"_ss, "GB"_ss, "TB"_ss,
        "PB"_ss,    "EB"_ss, "ZB"_ss, "YB"_ss
    };

    class str_buf_t : public fmt::internal::buffer<s8> {
        str_t*      _str;
    public: explicit str_buf_t(str_t* str) : _str(str) {
            set((s8*) _str->data, str->capacity);
        }
        str_buf_t(str_buf_t&& other) FMT_NOEXCEPT {
            _str->operator=(*other._str);
        }
        ~str_buf_t() {
            _str->length = size();
        }
        str_buf_t& operator=(str_buf_t&& other) FMT_NOEXCEPT {
            _str->operator=(*other._str);
            return *this;
        }
    protected:
        u0 grow(usize capacity) FMT_OVERRIDE {
            str::grow(*_str, capacity);
            set((s8*) _str->data, _str->capacity);
        }
    };

    namespace format {
        u0 hex_dump(fmt_buf_t& buf, const u0* data, u32 size);
        str_t vformat(alloc_t* alloc, fmt_str_t format_str, fmt_args_t args);
        u0 vprint(alloc_t* alloc, FILE* file, fmt_str_t format_str, fmt_args_t args);

        force_inline str_t to_string(const fmt_buf_t& buf) {
            str_t str;
            str::init(str, buf.get_allocator().backing);
            str::reserve(str, buf.size());
            std::memcpy(str.data, buf.data(), buf.size());
            str.length = buf.size();
            return str;
        }

        template <typename... Args>
        inline u0 print(fmt_str_t format_str, const Args&... args) {
            vprint(context::top()->alloc, stdout, format_str, fmt::make_format_args(args...));
        }

        force_inline u0 to_string(const fmt_buf_t& buf, str_t& str) {
            str::reserve(str, buf.size());
            std::memcpy(str.data, buf.data(), buf.size());
            str.length = buf.size();
        }

        template <typename... Args>
        inline str_t format(fmt_str_t format_str, const Args&... args) {
            return vformat(context::top()->alloc, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(FILE* file, fmt_str_t format_str, const Args&... args) {
            vprint(context::top()->alloc, file, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(alloc_t* alloc, fmt_str_t format_str, const Args&... args) {
            vprint(alloc, stdout, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline str_t format(alloc_t* alloc, fmt_str_t format_str, const Args&... args) {
            return vformat(alloc, format_str, fmt::make_format_args(args...));
        }

        template <typename... Args>
        inline u0 print(alloc_t* alloc, FILE* file, fmt_str_t format_str, const Args&... args) {
            vprint(alloc, file, format_str, fmt::make_format_args(args...));
        }

        template <typename Buffer, typename... Args>
        inline decltype(auto) format_to(Buffer& buf, fmt_str_t format_str, const Args&... args) {
            return fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
        }

        template <typename Buffer> inline u0 unitized_byte_size(Buffer& buf, u64 size) {
            u64 i{};
            while (size > 1024) {
                size /= 1024;
                i++;
            }
            if (i > 1) {
                format::format_to(buf, "{}.{} {}", size, i, s_byte_units[i]);
            } else {
                format::format_to(buf, "{} {}", size, s_byte_units[i]);
            }
        }

        inline u0 to_radix(str_t& str, Integer_Concept auto value, Radix_Concept auto radix = 10) {
            str_buf_t buf(&str);
            switch (radix) {
                case 2:     format_to(buf, "0b{:b}", value);    break;
                case 8:     format_to(buf, "0o{:o}", value);    break;
                case 16:    format_to(buf, "0x{:x}", value);    break;
                default:    format_to(buf, "{}", value);        break;
            }
        }

        force_inline str_t to_radix(Integer_Concept auto value, Radix_Concept auto radix = 10, alloc_t* alloc = context::top()->alloc) {
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
