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

#include <basecode/core/buf.h>
#include <basecode/core/format.h>

namespace basecode {
    namespace error {
        template<typename ...Args> inline u0 print(FILE* file, buf_cursor_t& cursor, fmt::string_view fmt_msg, Args&& ... args) {
            const auto msg = format::format(cursor.buf->alloc, fmt_msg, std::forward<Args>(args)...);
            format::print(cursor.buf->alloc, file, "error({}:{}): {}\n", cursor.line + 1, cursor.column + 1, msg);
        }
    }
}

