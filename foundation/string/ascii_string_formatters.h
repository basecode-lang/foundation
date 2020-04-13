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

#include <foundation/format/system.h>
#include "ascii_string.h"

namespace fmt {
    using namespace basecode::string;

    template<>
    struct formatter<slice_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(slice_t slice, FormatContext& ctx) {
            auto it = format_to_n(
                ctx.out(),
                slice.length,
                "{}",
                (const char*) slice.data);
            return it.out;
        }
    };

    template<>
    struct formatter<ascii_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(ascii_t str, FormatContext& ctx) {
            auto it = format_to_n(
                ctx.out(),
                str.length,
                "{}",
                (const char*) str.data);
            return it.out;
        }
    };
}