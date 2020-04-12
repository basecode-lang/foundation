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

#include <foundation/types.h>

namespace basecode::slice {
    template<typename T>
    struct slice_t final {
        u32         length{};
        const T*    data{};

        b8 operator<(const slice_t& other) const {
            if (this == &other) return false;
            if (length < other.length) return true;
            return std::memcmp(data, other.data, length) < 0;
        }

        b8 operator>(const slice_t& other) const {
            if (this == &other) return false;
            if (length > other.length) return true;
            return std::memcmp(data, other.data, length) > 0;
        }

        b8 operator==(const slice_t& other) const {
            if (this == &other) return true;
            return length == other.length
                && std::memcmp(data, other.data, length) == 0;
        }
    };

    template <typename T> inline b8 empty(slice_t<T>& slice) {
        return slice.length == 0 || slice.data == nullptr;
    }
}
