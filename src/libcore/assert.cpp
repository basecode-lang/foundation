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

#include <basecode/core/assert.h>
#include <basecode/core/format.h>

namespace basecode {
    u0 format_assert(const s8* prefix,
                     const s8* condition,
                     const s8* file,
                     s32 line,
                     const s8* msg,
                     const fmt_args_t& args) {
        str_t buf{};
        str::init(buf, memory::system::temp_alloc()); {
            str_buf_t sb(&buf);
            format::format_to(sb, "{}({}): {}: ", file, line, prefix);
            if (condition)
                format::format_to(sb, "`{}` ", condition);
            if (msg)
                fmt::vformat_to(sb, msg, args);
        }
        format::print(stderr, "{}\n", buf);
        str::free(buf);
    }
}
