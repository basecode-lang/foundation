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

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <basecode/core/types.h>

namespace basecode {
    using fmt_ctx_t         = fmt::format_context;
    using fmt_arg_t         = fmt::basic_format_arg<fmt_ctx_t>;
    using fmt_buf_t         = fmt::basic_memory_buffer<char,
                                                      fmt::inline_buffer_size,
                                                      fmt_alloc_t>;
    using fmt_dyn_args_t    = fmt::dynamic_format_arg_store<fmt_ctx_t>;
}

FORMAT_TYPE_AS(basecode::str_t, std::string_view);

FORMAT_TYPE_AS(basecode::str::slice_t, std::string_view);

FORMAT_TYPE(basecode::path_t, format_to(ctx.out(), "{}", data.str));

FORMAT_TYPE(basecode::source_info_t,
            format_to(ctx.out(),
                      "[S: {:>06}, E: {:>06}, [SL: {:>04}, SC: {:>03}, EL: {:>04}, EC: {:>04}]]",
                      data.pos.start,
                      data.pos.end,
                      data.start.line,
                      data.start.column,
                      data.end.line,
                      data.end.column));

FORMAT_TYPE(basecode::str_array_t,
            for (basecode::u32 i = 0; i < data.size; ++i) {
                if (i > 0) format_to(ctx.out(), ",");
                format_to(ctx.out(), "{}", data[i]);
            });

FORMAT_TYPE(basecode::uuid_t,
            format_to(ctx.out(),
                      "{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}",
                      data.data1,
                      data.data2,
                      data.data3,
                      data.data4[0], data.data4[1], data.data4[2], data.data4[3],
                      data.data4[4], data.data4[5], data.data4[6], data.data4[7]));

template <typename T>
struct fmt::formatter<basecode::variant_t<T>> {
    template <typename FormatContext>
    auto format(const basecode::variant_t<T>& data,
                FormatContext& ctx) -> decltype(ctx.out()) {
        format_to(ctx.out(), "{}", *data);
        return ctx.out();
    }
    template <typename ParseContext> auto parse(ParseContext& ctx) {
        return ctx.begin();
    }
};
