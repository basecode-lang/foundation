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
