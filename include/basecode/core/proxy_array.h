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

#include <cassert>
#include <basecode/core/types.h>
#include <basecode/core/iterator.h>

//#define POOL_ARRAY_INIT(type, array)        SAFE_SCOPE(                 \
//     auto ot = obj_pool::register_type<type>(storage);                  \
//     proxy_array::init((array), ot->objects, ot->objects.size);)
//
//#define POOL_ARRAY_APPEND(type, array)                                  \
//     ((array).size++, obj_pool::make<type>(storage))

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

    template<typename T, typename Backing_Type = T>
    struct proxy_array_t final {
        using Value_Type        = T;
        using Size_Per_16       = std::integral_constant<u32, 16 / sizeof(T)>;
        using Backing_Array     = const array_t<Backing_Type>*;

        Backing_Array           backing;
        u32                     start;
        u32                     size;

        const Value_Type& operator[](u32 index) const               { return (const Value_Type&) backing->data[start + index];  }

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const proxy_array_t* ref)                 { pos = ref->size;      }
            inline u0 next(const proxy_array_t* ref)                { UNUSED(ref); ++pos;   }
            inline Value_Type get(proxy_array_t* ref)               { return (*ref)[pos];   }
            inline u0 begin(const proxy_array_t* ref)               { UNUSED(ref); pos = 0; }
            inline b8 cmp(const iterator_state_t& s) const          { return pos != s.pos;  }
            inline const Value_Type get(const proxy_array_t* ref)   { return (*ref)[pos];   }
        };
        DECL_ITERS(proxy_array_t, Value_Type, iterator_state_t);
    };
    static_assert(sizeof(proxy_array_t<s32>) <= 16, "proxy_array_t<T> is now larger than 16 bytes!");

    namespace proxy_array {
        template <Proxy_Array T>
        u0 pop(T& array) {
            --array.size;
        }

        template <Proxy_Array T>
        b8 empty(const T& array) {
            return array.size == 0;
        }

        template <Proxy_Array T,
                  typename Value_Type = std::remove_reference_t<typename T::Value_Type>>
        decltype(auto) back(T& array) {
            if constexpr (std::is_pointer_v<Value_Type>) {
                return array.size == 0 ? nullptr : array[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array[array.size - 1];
            }
        }

        template <Array_Concept D, Proxy_Array S,
                  b8 Dst_Is_Static = typename D::Is_Static::value(),
                  typename Dst_Type = std::remove_reference_t<typename D::Value_Type>,
                  typename Src_Type = std::remove_reference_t<typename S::Value_Type>>
        u0 copy(D& dst, const S& src) {
            if constexpr (!same_as<Src_Type, Dst_Type>) {
                static_assert("proxy_array::copy only supported between equivalent types");
            }
            if constexpr (Dst_Is_Static) {
                assert(src.size <= dst.capacity);
            } else {
                if (src.size > dst.capacity)
                    grow(dst);
            }
            std::memcpy(dst.data, src.backing->data, src.size * sizeof(Src_Type));
            dst.size = src.size;
        }

        template <Proxy_Array T,
                  typename Value_Type = std::remove_reference_t<T>>
        decltype(auto) front(T& array) {
            if constexpr (std::is_pointer_v<Value_Type>) {
                return array.size == 0 ? nullptr : array[0];
            } else {
                return array.size == 0 ? nullptr : &array[0];
            }
        }

        u0 shrink(Array_Concept auto& array, u32 new_size) {
            if (new_size < array.size)
                array.size = new_size;
        }

        template <Proxy_Array T,
                  u32 Size_Per_16 = typename T::Size_Per_16::value(),
                  typename Value_Type = std::remove_reference_t<typename T::Value_Type>>
        s32 contains(const T& array, const auto& value) {
            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (std::is_integral_v<Value_Type>
                        || std::is_floating_point_v<Value_Type>
                        || std::is_pointer_v<Value_Type>) {
                __m128i n;
                if constexpr (Size_Per_16 == 4) {
                    n = _mm_set1_epi32((s32) value);
                } else if constexpr (Size_Per_16 == 2) {
                    n = _mm_set1_epi64x((s64) value);
                }
                m128i_bytes_t dqw{};
                while (i + Size_Per_16 < array.capacity) {
                    dqw.value = _mm_loadu_si128((const __m128i*) (array.backing->data + i));
                    __m128i cmp;
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
                            case 0xf:               break;
                            case 0xf0:  ++i;        break;
                            case 0xf00: i += 2;     break;
                            case 0xf000:i += 3;     break;
                            default:
                                i += Size_Per_16;
                                continue;
                        }
                        if (i >= array.size)    return -1;
                        return i;
                    }
                }
            }
#endif
            for (; i < array.size; ++i)
                if (array[i] == value) return i;
            return -1;
        }

        template <Proxy_Array T,
                  typename Value_Type = std::remove_reference_t<typename T::Value_Type>>
        decltype(auto) front(const Array_Concept auto& array) {
            if constexpr (std::is_pointer_v<Value_Type>) {
                return array.size == 0 ? nullptr : array[0];
            } else {
                return array.size == 0 ? nullptr : &array[0];
            }
        }

        template <Proxy_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 init(T& array, const array_t<Value_Type>& backing, u32 start, u32 size = 0) {
            array.backing = &backing;
            array.start   = start;
            array.size    = size;
        }
    }
}
