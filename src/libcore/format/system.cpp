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

#include "system.h"

namespace basecode::format {
    u0 vprint(
            memory::allocator_t* allocator,
            FILE* file,
            fmt::string_view format_str,
            fmt::format_args args) {
        format::allocator_t alloc(allocator);
        memory_buffer_t buf(alloc);
        fmt::vformat_to(buf, format_str, args);
        std::fwrite(buf.data(), 1, buf.size(), file);
    }

    string::ascii_t format(
            memory::allocator_t* allocator,
            fmt::string_view format_str,
            fmt::format_args args) {
        format::allocator_t alloc(allocator);
        memory_buffer_t buf(alloc);
        fmt::vformat_to(buf, format_str, args);
        return to_string(buf);
    }
}