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

#include <fmt/format.h>
#include <basecode/core/str.h>
#include <basecode/core/types.h>
#include <basecode/core/memory/std_allocator.h>

namespace basecode::format {
    using allocator_t = memory::std_allocator_t<char>;
    using memory_buffer_t = fmt::basic_memory_buffer<char, fmt::inline_buffer_size, allocator_t>;

    u0 vprint(alloc_t*, FILE*, fmt::string_view, fmt::format_args);

    string_t vformat(alloc_t*, fmt::string_view, fmt::format_args);

    template <typename... Args>
    inline u0 print(fmt::string_view format_str, const Args&... args) {
        vprint(context::top()->alloc, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string_t format(fmt::string_view format_str, const Args&... args) {
        return vformat(context::top()->alloc, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline u0 print(FILE* file, fmt::string_view format_str, const Args&... args) {
        vprint(context::top()->alloc, file, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline u0 print(alloc_t* allocator, fmt::string_view format_str, const Args&... args) {
        vprint(allocator, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string_t format(alloc_t* allocator, fmt::string_view format_str, const Args&... args) {
        return vformat(allocator, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline u0 print(alloc_t* allocator, FILE* file, fmt::string_view format_str, const Args&... args) {
        vprint(allocator, file, format_str, fmt::make_format_args(args...));
    }

    template <typename S, typename Char = std::enable_if_t<fmt::internal::is_string<S>::value, fmt::char_t<S>>, typename... Args>
    inline typename fmt::buffer_context<Char>::iterator format_to(memory_buffer_t& buf, const S& format_str, const Args&... args) {
        return fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
    }

    force_inline string_t to_string(const memory_buffer_t& buf) {
        string_t str;
        string::init(str, buf.get_allocator().backing);
        string::reserve(str, buf.size());
        std::memcpy(str.data, buf.data(), buf.size());
        str.length = buf.size();
        return str;
    }

    // XXX: std::enable_if_t<std::numeric_limits<T>::is_integer> not working?
    template <typename T>
    force_inline string_t to_radix(T value, u32 radix = 10, alloc_t* allocator = context::top()->alloc) {
        format::allocator_t alloc(allocator);
        memory_buffer_t buf(alloc);
        switch (radix) {
            case 2: {
                format_to(buf, "0b{:b}", value);
                break;
            }
            case 8: {
                format_to(buf, "0o{:o}", value);
                break;
            }
            case 16: {
                format_to(buf, "0x{:x}", value);
                break;
            }
            default: {
                format_to(buf, "{}", value);
                break;
            }
        }
        return to_string(buf);
    }
}
