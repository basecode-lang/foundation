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

#pragma once

#include <cstring>
#include <basecode/core/types.h>
#include <basecode/core/format_fwd.h>
#include <basecode/core/hash/murmur.h>

#define WITH_SLICE_AS_CSTR(Slice, Code) \
        [&](str::slice_t s) -> void {                               \
            s8 Slice[s.length + 1];                                 \
            std::memcpy(Slice, s.data, s.length);                   \
            Slice[s.length] = '\0';                                 \
            Code;                                                   \
        }(Slice)                                                    \

namespace basecode {
    namespace slice {
        inline b8 contains(const str::slice_t& slice, s8 ch) {
            for (u32 i = 0; i < slice.length; ++i) {
                if (slice[i] == ch)
                    return true;
            }
            return false;
        }

        template<typename T>
        inline b8 empty(const slice_t<T>& slice) {
            return slice.length == 0 || slice.data == nullptr;
        }

        inline str::slice_t make(const u8* str, u32 length) {
            return str::slice_t{.data = str, .length = length};
        }

        inline str::slice_t make(const s8* str, s32 length = -1) {
            return str::slice_t{
                .data = (const u8*) str,
                .length = length == -1 ? u32(strlen(str)) : length
            };
        }
    }

    template <typename T>
    inline b8 operator<(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length < rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) < 0;
    }

    template <typename T>
    inline b8 operator>(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length > rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) > 0;
    }

    template <typename T>
    inline b8 operator==(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return true;
        return lhs.length == rhs.length
               && std::memcmp(lhs.data, rhs.data, lhs.length) == 0;
    }

    namespace hash {
        template <typename T>
        inline u64 hash64(const slice_t<T>& key) {
            return murmur::hash64(key.data, key.length);
        }
    }
}

FORMAT_TYPE_AS(basecode::str::slice_t, std::string_view);
