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

#include <basecode/core/types.h>
#include <basecode/core/slice.h>
#include <basecode/core/assert.h>
#include <basecode/core/memory.h>
#include <basecode/core/context.h>
#include <basecode/core/array_common.h>

namespace basecode {
    template <typename T, u32 Capacity = 8>
    struct static_array_t final {
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, true>;
        using Value_Type        = T;
        using Size_Per_16       [[maybe_unused]] = std::integral_constant<u32, 16 / sizeof(T)>;

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
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, false>;
        using Value_Type        = T;
        using Size_Per_16       [[maybe_unused]] = std::integral_constant<u32, 16 / sizeof(T)>;

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

        template <typename T>
        array_t<T> make(std::initializer_list<T> elements,
                        alloc_t* alloc = context::top()->alloc.main);

        template <Array_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 init(T& array, alloc_t* alloc = context::top()->alloc.main);

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
                BC_ASSERT(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            return array.data[array.size++];
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
                BC_ASSERT(array.size + 1 < array.capacity);
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

        template <Dynamic_Array_Concept T>
        u0 resize(T& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
            array.size = new_size;
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type>
        b8 erase(T& array, const auto& value) {
            s32 idx = contains(array, value);
            if (idx == -1)
                return false;
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
                BC_ASSERT(array.size + 1 < array.capacity);
            } else {
                if (array.size + 1 > array.capacity)
                    grow(array);
            }
            array.data[array.size++] = value;
        }

        template <Array_Concept T,
                  typename Value_Type = typename T::Value_Type>
        u0 erase(T& array, u32 start_idx, u32 end_idx) {
            const s32 erase_diff = end_idx - start_idx;
            const s32 move_diff  = array.size - end_idx;
            if (erase_diff <= 0 || move_diff <= 0)
                return;
            auto dest = array.data + start_idx;
            auto src  = array.data + end_idx;
            std::memcpy(dest,
                        src,
                        move_diff * sizeof(Value_Type));
            array.size -= erase_diff;
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
                BC_ASSERT(dst.size + src.size < dst.capacity);
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

        template <Dynamic_Array_Concept T>
        u0 grow(T& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
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
        u0 insert(T& array, u32 index, auto&& value) {
            if constexpr (Is_Static) {
                BC_ASSERT(array.size + 1 < array.capacity);
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
                  b8 Is_Static = T::Is_Static::value,
                  typename Value_Type = typename T::Value_Type>
        u0 insert(T& array, u32 index, const auto& value) {
            if constexpr (Is_Static) {
                BC_ASSERT(array.size + 1 < array.capacity);
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
