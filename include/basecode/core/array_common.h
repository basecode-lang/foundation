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

#include <basecode/core/array.h>

namespace basecode {
    template <typename T>
    concept Proxy_Array = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Size_Per_16;
        typename                T::Backing_Array;

        {t.backing}             -> same_as<typename T::Backing_Array>;
        {t.start}               -> same_as<u32>;
        {t.size}                -> same_as<u32>;
    };

    template <typename T>
    concept Static_Array_Concept = same_as<typename T::Is_Static, std::true_type>
                                   && requires(const T& t) {
        typename                T::Is_Static;
        typename                T::Value_Type;

        {t.data}                -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
    };

    template <typename T>
    concept Dynamic_Array_Concept = same_as<typename T::Is_Static, std::false_type>
                                    && requires(const T& t) {
        typename                T::Is_Static;
        typename                T::Value_Type;
        typename                T::Size_Per_16;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.data}                -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
    };

    template <typename T>
    concept Array_Concept = Static_Array_Concept<T>
        || Dynamic_Array_Concept<T>
        || Proxy_Array<T>;

    namespace array {
        template <Array_Concept T>
        u0 pop(T& array) {
            --array.size;
        }

        template <Array_Concept T>
        b8 empty(const T& array) {
            return array.size == 0;
        }

        template <Array_Concept T,
            b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>>
        decltype(auto) back(T& array) {
            if constexpr (Is_Pointer) {
                return array.size == 0 ? nullptr : array.data[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array.data[array.size - 1];
            }
        }

        template <Array_Concept T,
                  b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>>
        decltype(auto) front(T& array) {
            if constexpr (Is_Pointer) {
                return array.size == 0 ? nullptr : array.data[0];
            } else {
                return array.size == 0 ? nullptr : &array.data[0];
            }
        }

        template <Array_Concept Dst,
                  Array_Concept Src,
                  b8 Dst_Is_Static = Dst::Is_Static::value,
                  typename Dst_Value_Type = typename Dst::Value_Type,
                  typename Src_Value_Type = typename Src::Value_Type>
        u0 copy(Dst& dst, const Src& src) {
            if constexpr (!same_as<Src_Value_Type, Dst_Value_Type>) {
                static_assert("array::copy only supported between equivalent types");
            }
            if constexpr (Dst_Is_Static) {
                assert(src.size <= dst.capacity);
            } else {
                if (src.size > dst.capacity)
                    grow(dst);
            }
            std::memcpy(dst.data,
                        src.data,
                        src.size * sizeof(Src_Value_Type));
            dst.size = src.size;
        }

        template <Array_Concept T>
        u0 shrink(T& array, u32 new_size) {
            if (new_size < array.size)
                array.size = new_size;
        }

        template <Array_Concept T,
                  b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>>
        decltype(auto) back(const T& array) {
            if constexpr (Is_Pointer) {
                return array.size == 0 ? nullptr : array.data[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array.data[array.size - 1];
            }
        }

        template <Array_Concept T,
                  b8 Is_Pointer = std::is_pointer_v<typename T::Value_Type>>
        decltype(auto) front(const T& array) {
            if constexpr (Is_Pointer) {
                return array.size == 0 ? nullptr : array.data[0];
            } else {
                return array.size == 0 ? nullptr : &array.data[0];
            }
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type,
                  u32 Size_Per_16 = T::Size_Per_16::value,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  b8 Is_Integral = std::is_integral_v<Value_Type>,
                  b8 Is_Floating_Point = std::is_floating_point_v<Value_Type>>
        s32 contains(T& array, const auto& value) {
            s32 idx = -1;
            if (array.size == 0)
                return idx;
            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (Is_Integral || Is_Floating_Point || Is_Pointer) {
                if (Size_Per_16 > array.size)
                    goto remainder;
                __m128i n{};
                if constexpr (Size_Per_16 == 4) {
                    n = _mm_set1_epi32((s32) value);
                } else if constexpr (Size_Per_16 == 2) {
                    n = _mm_set1_epi64x((s64) value);
                }
                m128i_bytes_t dqw{};
                while (i + Size_Per_16 < array.capacity) {
                    dqw.value = _mm_loadu_si128((const __m128i*) (array.data + i));
                    __m128i cmp{};
                    if constexpr (Size_Per_16 == 4) {
                        cmp = _mm_cmpeq_epi32(dqw.value, n);
                    } else if constexpr (Size_Per_16 == 2) {
                        cmp = _mm_cmpeq_epi64(dqw.value, n);
                    }
                    u16 mask = _mm_movemask_epi8(cmp);
                    if (!mask) {
                        i += Size_Per_16;
                    } else {
                        switch (mask) {
                            // 32-bit values matching
                            case 0xf:               break;
                            case 0xf0:  ++i;        break;
                            case 0xf00: i += 2;     break;
                            case 0xf000:i += 3;     break;

                            // 64-bit values matching
                            case 0xff:              break;
                            case 0xff00:++i;        break;

                            default:
                                i += Size_Per_16;
                                continue;
                        }
                        if (i >= array.size)    return -1;
                        idx = i;
                        break;
                    }
                }
            }
#endif
        remainder:
            if (idx == -1) {
                for (; i < array.size; ++i) {
                    if (array.data[i] == value) {
                        idx = i;
                        break;
                    }
                }
            }
            return idx;
        }
    }
}
