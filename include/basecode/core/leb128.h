// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
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

namespace basecode {
    struct leb_128_t final {
        u8                      data[16];
        u32                     size;
        b8                      is_signed;
    };

    namespace leb {
        u0 init(leb_128_t& leb);

        template <typename T, b8 Is_Signed = same_as<T, s128>>
        requires same_as<T, s128> || same_as<T, u128>
        auto encode(T value) {
            leb_128_t leb{};
            init(leb);
            leb.is_signed = Is_Signed;
            UNUSED(value);
            return leb_128_t{};
        }

        auto decode(const leb_128_t& value);
    }
}
