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
#include <basecode/core/slice.h>
#include <basecode/core/memory.h>

namespace basecode {
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
    concept Array_Concept = Static_Array_Concept<T> || Dynamic_Array_Concept<T>;

    template <typename T, u32 Capacity = 8>
    struct static_array_t final {
        using Is_Static         = std::integral_constant<b8, true>;
        using Value_Type        = T;
        using Size_Per_16       = std::integral_constant<u32, 16 / sizeof(T)>;

        Value_Type              data[Capacity];
        u32                     size     = 0;
        u32                     capacity = Capacity;

        Value_Type& operator[](u32 index)               { return data[index];       }
        const Value_Type& operator[](u32 index) const   { return data[index];       }

        T* end()                                        { return data + size;       }
        T* rend()                                       { return data - 1;          }
        T* begin()                                      { return data;              }
        T* rbegin()                                     { return data + size - 1;   }
        [[nodiscard]] const T* end() const              { return data + size;       }
        [[nodiscard]] const T* rend() const             { return data - 1;          }
        [[nodiscard]] const T* begin() const            { return data;              }
        [[nodiscard]] const T* rbegin() const           { return data + size - 1;   }
    };

    template<typename T>
    struct array_t final {
        using Value_Type        = T;
        using Is_Static         = std::integral_constant<b8, false>;
        using Size_Per_16       = std::integral_constant<u32, 16 / sizeof(T)>;

        alloc_t*                alloc;
        Value_Type*             data;
        u32                     size;
        u32                     capacity;

        Value_Type& operator[](u32 index)               { return data[index];       }
        const Value_Type& operator[](u32 index) const   { return data[index];       }

        T* end()                                        { return data + size;       }
        T* rend()                                       { return data - 1;          }
        T* begin()                                      { return data;              }
        T* rbegin()                                     { return data + size - 1;   }
        [[nodiscard]] const T* end() const              { return data + size;       }
        [[nodiscard]] const T* rend() const             { return data - 1;          }
        [[nodiscard]] const T* begin() const            { return data;              }
        [[nodiscard]] const T* rbegin() const           { return data + size - 1;   }
    };
    static_assert(sizeof(array_t<s32>) <= 24, "array_t<T> is now larger than 24 bytes!");

    namespace array {
        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 free(T& array);

        template <Dynamic_Array_Concept T,
                  typename Value_Type = typename T::Value_Type>
        u0 reserve(T& array, u32 new_capacity);

        template <Dynamic_Array_Concept T>
        u0 grow(T& array, u32 new_capacity = 8);

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 init(T& array, alloc_t* alloc = context::top()->alloc);

        template <typename T>
        array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template <Array_Concept T>
        u0 pop(T& array) {
            --array.size;
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type>
        u0 reset(T& array) {
            array.size = {};
            std::memset(array.data,
                        0,
                        array.capacity * sizeof(Value_Type));
        }

        template <Array_Concept T>
        u0 clear(T& array) {
            free(array);
        }

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        auto& append(T& array) {
            if constexpr (Is_Static) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            return array.data[array.size++];
        }

        template <Array_Concept T, b8 Is_Static>
        u0 free(T& array) {
            if constexpr (!Is_Static) {
                memory::free(array.alloc, array.data);
                array.data = {};
            }
            array.size = array.capacity = {};
        }

        template <Dynamic_Array_Concept T>
        u0 trim(T& array) {
            reserve(array, array.size);
        }

        template <Array_Concept T>
        b8 empty(const T& array) {
            return array.size == 0;
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type>
        u0 erase(T& array, u32 index) {
            if (index >= array.size)
                return;
            auto dest = array.data + index;
            std::memcpy(dest,
                        dest + 1,
                        (array.size - index) * sizeof(Value_Type));
            --array.size;
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

        template <typename T>
        array_t<T> make(alloc_t* alloc) {
            array_t<T> array;
            init(array, alloc);
            return array;
        }

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 append(T& array, auto&& value) {
            if constexpr (Is_Static) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            array.data[array.size++] = value;
        }

        template <Array_Concept T, b8 Is_Static>
        u0 init(T& array, alloc_t* alloc) {
            if constexpr (!Is_Static) {
                array.data     = {};
                array.alloc    = alloc;
                array.capacity = {};
            }
            array.size = {};
        }

        template <Array_Concept T>
        u0 shrink(T& array, u32 new_size) {
            if (new_size < array.size)
                array.size = new_size;
        }

        template <Dynamic_Array_Concept T>
        u0 resize(T& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
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
        b8 erase(T& array, const auto& value) {
            s32 idx = -1;
            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (Is_Integral || Is_Floating_Point || Is_Pointer) {
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
                        if (i >= array.size)    return false;
                        idx = i;
                        break;
                    }
                }
            }
#endif
            if (idx == -1) {
                for (; i < array.size; ++i) {
                    if (array.data[i] == value) {
                        idx = i;
                        break;
                    }
                }
                if (idx == -1) return false;
            }
            auto dest = array.data + idx;
            std::memcpy(dest,
                        dest + 1,
                        (array.size - idx) * sizeof(Value_Type));
            --array.size;
            return true;
        }

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 append(T& array, const auto& value) {
            if constexpr (Is_Static) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            array.data[array.size++] = value;
        }

        template <Array_Concept Dst,
                  Array_Concept Src,
                  b8 Dst_Is_Static = Dst::Is_Static::value,
                  typename Dst_Value_Type = typename Dst::Value_Type,
                  typename Src_Value_Type = typename Src::Value_Type>
        u0 append(Dst& dst, Src& src) {
            if constexpr (!same_as<Src_Value_Type, Dst_Value_Type>) {
                static_assert("array::append array-to-array only supported between equivalent types");
            }
            if constexpr (Dst_Is_Static) {
                assert(dst.size + src.size < dst.capacity);
            } else {
                const auto new_size = dst.size + src.size;
                if (new_size > dst.capacity)
                    grow(dst, new_size);
            }
            std::memcpy(dst.data + dst.size,
                        src.data,
                        src.size * sizeof(Src_Value_Type));
            dst.size += src.size;
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

        template <Dynamic_Array_Concept T>
        u0 grow(T& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
        }

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value,
                  typename Value_Type = typename T::Value_Type>
        u0 insert(T& array, u32 index, auto&& value) {
            if constexpr (Is_Static) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            if (index < array.size) {
                std::memmove(array.data + index + 1,
                             array.data + index,
                             (array.size - index) * sizeof(Value_Type));
            }
            array.data[index] = value;
            ++array.size;
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type,
                  u32 Size_Per_16 = T::Size_Per_16::value,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>,
                  b8 Is_Integral = std::is_integral_v<Value_Type>,
                  b8 Is_Floating_Point = std::is_floating_point_v<Value_Type>>
        s32 contains(T& array, const auto& value) {
            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (Is_Integral || Is_Floating_Point || Is_Pointer) {
                __m128i n;
                if constexpr (Size_Per_16 == 4) {
                    n = _mm_set1_epi32((s32) value);
                } else if constexpr (Size_Per_16 == 2) {
                    n = _mm_set1_epi64x((s64) value);
                }
                m128i_bytes_t dqw{};
                while (i + Size_Per_16 < array.capacity) {
                    dqw.value = _mm_loadu_si128((const __m128i*) (array.data + i));
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
                if (array.data[i] == value) return i;
            return -1;
        }

        template <Dynamic_Array_Concept T, typename Value_Type>
        u0 reserve(T& array, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(array.alloc, array.data);
                array.data     = {};
                array.capacity = array.size = {};
                return;
            }

            if (new_capacity == array.capacity)
                return;

            new_capacity = std::max(array.size, new_capacity);
            array.data = (Value_Type*) memory::realloc(
                array.alloc,
                array.data,
                new_capacity * sizeof(Value_Type),
                alignof(Value_Type));
            const auto data          = array.data + array.size;
            const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(Value_Type));
            array.capacity = new_capacity;
        }

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value,
                  typename Value_Type = typename T::Value_Type>
        u0 insert(T& array, u32 index, const auto& value) {
            if constexpr (Is_Static) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            if (index < array.size) {
                std::memmove(array.data + index + 1,
                             array.data + index,
                             (array.size - index) * sizeof(Value_Type));
            }
            array.data[index] = value;
            ++array.size;
        }

        template <typename T>
        array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
            array_t<T> array;
            init(array, alloc);
            reserve(array, elements.size());
            for (auto&& e : elements)
                append(array, e);
            return array;
        }
    }
}

namespace basecode::slice {
    template <typename T>
    inline slice_t<T> make(const Array_Concept auto& array) {
        return slice_t{.data = array.data, .length = array.size};
    }

    template <typename T>
    inline slice_t<T> make(const Array_Concept auto& array, u32 start, u32 end) {
        return slice_t{.data = array.data + start, .length = end - start};
    }
}
