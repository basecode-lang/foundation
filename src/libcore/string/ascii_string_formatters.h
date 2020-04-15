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

#include <basecode/core/format/system.h>
#include "ascii_string.h"

using namespace basecode::string;

template<>
struct fmt::formatter<slice_t> : fmt::formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const slice_t& slice, FormatContext& ctx) {
        std::string_view temp((const char*) slice.data, slice.length);
        return formatter<std::string_view>::format(temp, ctx);
    }
};

template<>
struct fmt::formatter<ascii_t> : fmt::formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const ascii_t& str, FormatContext& ctx) {
        std::string_view temp((const char*) str.data, str.length);
        return formatter<std::string_view>::format(temp, ctx);
    }
};
