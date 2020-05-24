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

#include <cstdint>
#include <cstddef>
#include <sys/types.h>

#if defined(__GNUC__)
#   define force_inline inline __attribute__((always_inline, unused))
#   define never_inline inline __attribute__((noinline, unused))
#   ifndef likely
#       define likely(x) __builtin_expect(!!(x), 1)
#   endif
#   ifndef unlikely
#       define unlikely(x) __builtin_expect(!!(x), 0)
#   endif
#elif defined(_MSC_VER)
#   define force_inline __forceinline
#   define never_inline __declspec(noinline)
#   ifndef likely
#       define likely(x) x
#   endif
#   ifndef unlikely
#       define unlikely(x) x
#   endif
#else
#   define force_inline inline
#   define never_inline
#   ifndef likely
#       define likely(x) x
#   endif
#   ifndef unlikely
#       define unlikely(x) x
#   endif
#endif

#define UNIQUE_NAME_1(x, y)     x##y
#define UNIQUE_NAME_2(x, y)     UNIQUE_NAME_1(x, y)
#define UNIQUE_NAME(x)    _     UNIQUE_NAME_2(x, __COUNTER__)
#define UNUSED(x)               ((void) x)
#define OK(x)                   (0 == (u32) x)
#define SAFE_SCOPE(x)           do { x } while (false)
#define ZERO_MEM(x, s)          std::memset((x), 0, sizeof((s)))
#define HAS_ZERO(v)             (((v)-UINT64_C(0x0101010101010101)) & ~(v)&UINT64_C(0x8080808080808080))

namespace basecode {
    using u0    = void;
    using u8    = uint8_t;
    using u16   = uint16_t;
    using u32   = uint32_t;
    using u64   = uint64_t;
    using s8    = char;
    using s16   = int16_t;
    using s32   = int32_t;
    using s64   = int64_t;
    using b8    = bool;
    using f32   = float;
    using f64   = double;
    using s128  = __int128_t;
    using u128  = __uint128_t;
    using usize = std::size_t;
    using ssize = ssize_t;

    template <typename From, typename To>
    concept convertible_to =
        std::is_convertible_v<From, To> &&
        requires(std::add_rvalue_reference_t<From> (&f)()) {
            static_cast<To>(f());
        };

    template <typename T, typename U>
    concept same_helper = std::is_same_v<T, U>;

    template <typename T, typename U>
    concept same_as = same_helper<T, U> && same_helper<U, T>;

    template <typename T> concept Slice_Concept = requires(const T& t) {
        {t.data}    -> same_as<const u8*>;
        {t.length}  -> same_as<u32>;
    };

    template <typename T> concept String_Concept  = Slice_Concept<T> || requires(const T& t) {
        {t.data}    -> same_as<u8*>;
        {t.length}  -> same_as<u32>;
    };

    template <typename T> concept Integer_Concept = std::is_integral_v<T>;

    template <typename T> concept Radix_Concept   = Integer_Concept<T> && requires(T radix) {
        radix == 2 || radix == 8 || radix == 10 || radix == 16;
    };
}
