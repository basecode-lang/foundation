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
#include <basecode/core/types.h>
#include <basecode/core/string/ascii_string.h>
#include <basecode/core/memory/std_allocator.h>

namespace basecode::format {
    using namespace basecode::string;

    using allocator_t = memory::std_allocator_t<char>;
    using memory_buffer_t = fmt::basic_memory_buffer<char, fmt::inline_buffer_size, allocator_t>;

    force_inline string::ascii_t to_string(const memory_buffer_t& buf) {
        string::ascii_t str(buf.get_allocator().backing);
        string::reserve(str, buf.size());
        std::memcpy(str.data, buf.data(), buf.size());
        return str;
    }

    u0 vprint(memory::allocator_t*, FILE*, fmt::string_view, fmt::format_args);

    string::ascii_t vformat(memory::allocator_t*, fmt::string_view, fmt::format_args);

    template <typename... Args>
    inline u0 print(
            fmt::string_view format_str,
            const Args&... args) {
        vprint(context::current()->allocator, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline u0 print(
            memory::allocator_t* allocator,
            fmt::string_view format_str,
            const Args&... args) {
        format::allocator_t alloc(allocator);
        vprint(alloc, stdout, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string::ascii_t format(
            fmt::string_view format_str,
            const Args&... args) {
        return vformat(context::current()->allocator, format_str, fmt::make_format_args(args...));
    }

    template <typename... Args>
    inline string::ascii_t format(
            memory::allocator_t* allocator,
            fmt::string_view format_str,
            const Args&... args) {
        format::allocator_t alloc(allocator);
        return vformat(alloc, format_str, fmt::make_format_args(args...));
    }
}