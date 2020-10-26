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
#include <fmt/chrono.h>
#include <basecode/core/memory/std_alloc.h>

#define FORMAT_TYPE(Type, Code)                                                     \
    template <>                                                                     \
    struct fmt::formatter<Type> {                                                   \
        template <typename ParseContext> auto parse(ParseContext& ctx) {            \
            return ctx.begin();                                                     \
        }                                                                           \
        template <typename FormatContext>                                           \
        auto format(const Type& data, FormatContext& ctx) -> decltype(ctx.out()) {  \
            Code;                                                                   \
            return ctx.out();                                                       \
        }                                                                           \
    }

#define FORMAT_TYPE_AS(Type, Base)                                                  \
    template <typename Char>                                                        \
    struct fmt::formatter<Type, Char> : fmt::formatter<Base, Char> {                \
        template <typename FormatContext>                                           \
        auto format(Type const& val, FormatContext& ctx) -> decltype(ctx.out()) {   \
            return fmt::formatter<Base, Char>::format(val, ctx);                    \
        }                                                                           \
    }

namespace basecode {
    using fmt_ctx_t         = fmt::format_context;
    using fmt_str_t         = fmt::string_view;
    using fmt_arg_t         = fmt::basic_format_arg<fmt_ctx_t>;
    using fmt_args_t        = fmt::format_args;
    using fmt_alloc_t       = std_alloc_t<char>;
    using fmt_buf_t         = fmt::basic_memory_buffer<char, fmt::inline_buffer_size, fmt_alloc_t>;
}
