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
#include <cstring>
#include <algorithm>
#include <basecode/core/types.h>
#include <basecode/core/slice.h>
#include <basecode/core/memory.h>

namespace basecode {
    template <typename T, u32 Capacity = 8> struct static_array_t final {
        using value_type    = T;
        using is_static     = std::integral_constant<b8, true>;
        using size_per_16   = std::integral_constant<u32, 16 / sizeof(T)>;

        T                   data[Capacity];
        u32                 size;
        u32                 capacity = Capacity;

        T* end()                                { return data + size;       }
        T* rend()                               { return data - 1;          }
        T* begin()                              { return data;              }
        T* rbegin()                             { return data + (size - 1); }
        const T* end() const                    { return data + size;       }
        const T* rend() const                   { return data - 1;          }
        const T* begin() const                  { return data;              }
        const T* rbegin() const                 { return data + (size - 1); }
        T& operator[](u32 index)                { return data[index];       }
        const T& operator[](u32 index) const    { return data[index];       }
    };

    template<typename T> struct array_t final {
        using value_type    = T;
        using is_static     = std::integral_constant<b8, false>;
        using size_per_16   = std::integral_constant<u32, 16 / sizeof(T)>;

        T*                  data;
        alloc_t*            alloc;
        u32                 size;
        u32                 capacity;

        T* end()                                { return data + size;       }
        T* rend()                               { return data - 1;          }
        T* begin()                              { return data;              }
        T* rbegin()                             { return data + (size - 1); }
        const T* end() const                    { return data + size;       }
        const T* rend() const                   { return data - 1;          }
        const T* begin() const                  { return data;              }
        const T* rbegin() const                 { return data + (size - 1); }
        T& operator[](u32 index)                { return data[index];       }
        const T& operator[](u32 index) const    { return data[index];       }
    };
    static_assert(sizeof(array_t<s32>) <= 24, "array_t<T> is now larger than 24 bytes!");

    namespace array {
        u0 free(Array_Concept auto& array);
        u0 reserve(Dynamic_Array_Concept auto& array, u32 new_capacity);
        u0 grow(Dynamic_Array_Concept auto& array, u32 new_capacity = 8);
        u0 init(Array_Concept auto& array, alloc_t* alloc = context::top()->alloc);
        template <typename T> array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        u0 pop(Array_Concept auto& array) {
            --array.size;
        }

        u0 reset(Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            array.size = {};
            std::memset(array.data, 0, array.capacity * sizeof(typename T::value_type));
        }

        u0 clear(Array_Concept auto& array) {
            free(array);
        }

        auto& append(Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (T::is_static::value) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            return array.data[array.size++];
        }

        b8 empty(const Array_Concept auto& array) {
            return array.size == 0;
        }

        u0 free(Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (!T::is_static::value) {
                memory::free(array.alloc, array.data);
                array.data = {};
            }
            array.size = array.capacity = {};
        }

        u0 trim(Dynamic_Array_Concept auto& array) {
            reserve(array, array.size);
        }

        u0 erase(Array_Concept auto& array, u32 index) {
            using T = std::remove_reference_t<decltype(array)>;
            if (index >= array.size)
                return;
            auto dest = array.data + index;
            std::memcpy(dest, dest + 1, (array.size - index) * sizeof(typename T::value_type));
            --array.size;
        }

        decltype(auto) back(Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (std::is_pointer_v<typename T::value_type>) {
                return array.size == 0 ? nullptr : array.data[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array.data[array.size - 1];
            }
        }

        decltype(auto) front(Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (std::is_pointer_v<typename T::value_type>) {
                return array.size == 0 ? nullptr : array.data[0];
            } else {
                return array.size == 0 ? nullptr : &array.data[0];
            }
        }

        u0 append(Array_Concept auto& array, auto&& value) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (T::is_static::value) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            array.data[array.size++] = value;
        }

        u0 init(Array_Concept auto& array, alloc_t* alloc) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (!T::is_static::value) {
                array.data     = {};
                array.alloc    = alloc;
                array.capacity = {};
            }
            array.size = {};
        }

        u0 shrink(Array_Concept auto& array, u32 new_size) {
            if (new_size < array.size) array.size = new_size;
        }

        decltype(auto) back(const Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (std::is_pointer_v<typename T::value_type>) {
                return array.size == 0 ? nullptr : array.data[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array.data[array.size - 1];
            }
        }

        template <typename T> array_t<T> make(alloc_t* alloc) {
            array_t<T> array;
            init(array, alloc);
            return array;
        }

        decltype(auto) front(const Array_Concept auto& array) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (std::is_pointer_v<typename T::value_type>) {
                return array.size == 0 ? nullptr : array.data[0];
            } else {
                return array.size == 0 ? nullptr : &array.data[0];
            }
        }

        b8 erase(Array_Concept auto& array, const auto& value) {
            using T = std::remove_reference_t<decltype(array)>;

            s32 idx = -1;
            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (std::is_integral_v<typename T::value_type> || std::is_floating_point_v<typename T::value_type> || std::is_pointer_v<typename T::value_type>) {
                __m128i n;
                if constexpr (T::size_per_16::value == 4) {
                    n = _mm_set1_epi32((s32) value);
                } else if constexpr (T::size_per_16::value == 2) {
                    n = _mm_set1_epi64x((s64) value);
                }
                m128i_bytes_t dqw{};
                while (i + T::size_per_16::value < array.capacity) {
                    dqw.value = _mm_loadu_si128((const __m128i*) (array.data + i));
                    __m128i cmp;
                    if constexpr (T::size_per_16::value == 4) {
                        cmp = _mm_cmpeq_epi32(dqw.value, n);
                    } else if constexpr (T::size_per_16::value == 2) {
                        cmp = _mm_cmpeq_epi64(dqw.value, n);
                    }
                    u16 mask = _mm_movemask_epi8(cmp);
                    if (!mask) {
                        i += T::size_per_16::value;
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

                            default:    i += T::size_per_16::value;  continue;
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
            std::memcpy(dest, dest + 1, (array.size - idx) * sizeof(typename T::value_type));
            --array.size;
            return true;
        }

        u0 append(Array_Concept auto& array, const auto& value) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (T::is_static::value) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            array.data[array.size++] = value;
        }

        u0 resize(Dynamic_Array_Concept auto& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
            array.size = new_size;
        }

        u0 grow(Dynamic_Array_Concept auto& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
        }

        u0 insert(Array_Concept auto& array, u32 index, auto&& value) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (T::is_static::value) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(typename T::value_type));
            array.data[index] = value;
            ++array.size;
        }

        s32 contains(const Array_Concept auto& array, const auto& value) {
            using T = std::remove_reference_t<decltype(array)>;

            u32 i{};
#if defined(__SSE4_2__) || defined(__SSE4_1__)
            if constexpr (std::is_integral_v<typename T::value_type> || std::is_floating_point_v<typename T::value_type> || std::is_pointer_v<typename T::value_type>) {
                __m128i n;
                if constexpr (T::size_per_16::value == 4) {
                    n = _mm_set1_epi32((s32) value);
                } else if constexpr (T::size_per_16::value == 2) {
                    n = _mm_set1_epi64x((s64) value);
                }
                m128i_bytes_t dqw{};
                while (i + T::size_per_16::value < array.capacity) {
                    dqw.value = _mm_loadu_si128((const __m128i*) (array.data + i));
                    __m128i cmp;
                    if constexpr (T::size_per_16::value == 4) {
                        cmp = _mm_cmpeq_epi32(dqw.value, n);
                    } else if constexpr (T::size_per_16::value == 2) {
                        cmp = _mm_cmpeq_epi64(dqw.value, n);
                    }
                    u16 mask = _mm_movemask_epi8(cmp);
                    if (!mask) {
                        i += T::size_per_16::value;
                    } else {
                        switch (mask) {
                            case 0xf:               break;
                            case 0xf0:  ++i;        break;
                            case 0xf00: i += 2;     break;
                            case 0xf000:i += 3;     break;
                            default:    i += T::size_per_16::value; continue;
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

        u0 reserve(Dynamic_Array_Concept auto& array, u32 new_capacity) {
            using T = std::remove_reference_t<decltype(array)>;

            if (new_capacity == 0) {
                memory::free(array.alloc, array.data);
                array.data     = {};
                array.capacity = array.size = {};
                return;
            }

            if (new_capacity == array.capacity)
                return;

            new_capacity = std::max(array.size, new_capacity);
            array.data = (typename T::value_type*) memory::realloc(array.alloc, array.data, new_capacity * sizeof(typename T::value_type), alignof(typename T::value_type));
            const auto data          = array.data + array.size;
            const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(typename T::value_type));
            array.capacity = new_capacity;
        }

        u0 insert(Array_Concept auto& array, u32 index, const auto& value) {
            using T = std::remove_reference_t<decltype(array)>;
            if constexpr (T::is_static::value) {
                assert(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(typename T::value_type));
            array.data[index] = value;
            ++array.size;
        }

        template <typename T> array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
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
    template <typename T> inline slice_t<T> make(const Array_Concept auto& array) {
        return slice_t{.data = array.data, .length = array.size};
    }

    template <typename T> inline slice_t<T> make(const Array_Concept auto& array, u32 start, u32 end) {
        return slice_t{.data = array.data + start, .length = end - start};
    }
}
