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

#include <basecode/core/types.h>
#include <basecode/core/format.h>
#include <basecode/core/source/buffer.h>

namespace basecode {
    namespace error {
        template<typename ...Args>
        inline u0 print(
                FILE* file,
                source::buffer_t& buf,
                fmt::string_view fmt_msg,
                Args&& ... args) {
            const auto msg = format::format(buf.allocator, fmt_msg, std::forward<Args>(args)...);
            format::print(
                buf.allocator,
                file,
                "error({}:{}): {}\n",
                buf.line + 1,
                buf.column + 1,
                msg);
        }
    }
}

