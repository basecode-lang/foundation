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

#include <cstring>
#include <foundation/types.h>

namespace basecode::slice {
    template<typename T>
    struct slice_t final {
        u32 length{};
        const T* data{};
    };

    using string_slice_t = slice_t<u8>;

    [[maybe_unused]] inline static string_slice_t operator "" _ss(
            const char* value) {
        return string_slice_t{
            .data = (const u8*) value,
            .length = (u32) strlen(value)
        };
    }

    [[maybe_unused]] inline static string_slice_t operator "" _ss(
            const char* value,
            std::size_t length) {
        return string_slice_t{
            .data = (const u8*) value,
            .length = (u32) length
        };
    }
}