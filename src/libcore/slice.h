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
#include <fmt/format.h>
#include <basecode/core/types.h>
#include <basecode/core/hashing/murmur.h>
#include <basecode/core/hashing/hashable.h>

namespace basecode {
    template<typename T> struct slice_t final {
        const T*            data;
        u32                 length;

        const T& operator[](u32 index) const { return data[index]; }
    };

    namespace string {
        using slice_t = slice_t<u8>;
    }

    template <typename T> inline b8 operator<(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length < rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) < 0;
    }

    template <typename T> inline b8 operator>(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length > rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) > 0;
    }

    template <typename T> inline b8 operator==(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return true;
        return lhs.length == rhs.length
               && std::memcmp(lhs.data, rhs.data, lhs.length) == 0;
    }

    namespace slice {
        template<typename T> inline b8 empty(slice_t<T>& slice) {
            return slice.length == 0 || slice.data == nullptr;
        }

        inline string::slice_t make(const u8* str, u32 length) {
            return string::slice_t{.data = str, .length = length};
        }

        inline string::slice_t make(const s8* str, u32 length) {
            return string::slice_t{.data = (const u8*) str, .length = length};
        }
    }

    namespace hashing {
        template <typename T> inline u64 hash64(const slice_t<T>& key) {
            return murmur::hash64(key.data, key.length);
        }
    }
}

template<>
struct fmt::formatter<basecode::string::slice_t> : fmt::formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const basecode::string::slice_t& slice, FormatContext& ctx) {
        std::string_view temp((const basecode::s8*) slice.data, slice.length);
        return formatter<std::string_view>::format(temp, ctx);
    }
};
