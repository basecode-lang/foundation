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
#include <algorithm>
#include <foundation/types.h>
#include <foundation/memory/system.h>

namespace basecode::adt {

    template <typename T>
    struct array_t final {
        T* data{};
        u32 size{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        T& operator[](usize index);
        const T& operator[](usize index) const;
    };

    template<typename T>
    T& array_t<T>::operator[](usize index) {
        return data[index];
    }

    template<typename T>
    const T& array_t<T>::operator[](usize index) const {
        return data[index];
    }

    namespace array {
        template <typename T> u0 reserve(array_t<T>& array, u32 new_capacity);

        namespace internal {
            template <typename T>
            u0 grow(array_t<T>& array, u32 new_capacity = 0) {
                new_capacity = std::max(new_capacity, array.capacity);
                reserve(array, new_capacity * 2 + 8);
            }

            template <typename T>
            T* prepare_insert(array_t<T>& array, const T* it) {
                const auto offset = it - array.data;
                if (array.size == array.capacity)
                    reserve(array, array.size + 1);
                if (offset < array.size) {
                    auto count = array.size - offset;
                    std::memmove(
                        array.data + offset + 1,
                        array.data + offset,
                        count * sizeof(T));
                }
                return offset;
            }
        }

        template <typename T>
        u0 pop(array_t<T>& array) {
            --array.size;
        }

        template <typename T>
        u0 reset(array_t<T>& array) {
            array.size = 0;
            std::memset(array.data, 0, sizeof(T) * array.capacity);
        }

        template <typename T>
        u0 trim(array_t<T>& array) {
            reserve(array, array.size);
        }

        template <typename T>
        u0 clear(array_t<T>& array) {
            memory::deallocate(array.allocator, array.data);
            array.size = 0;
            array.capacity = 0;
            array.data = nullptr;
        }

        template <typename T>
        b8 empty(array_t<T>& array) {
            return array.size == 0;
        }

        template <typename T>
        T* end(array_t<T>& array) {
            return array.data + array.size;
        }

        template <typename T>
        const T* end(array_t<T>& array) {
            return array.data + array.size;
        }

        template <typename T>
        T* begin(array_t<T>& array) {
            return array.data;
        }

        template <typename T>
        const T* begin(array_t<T>& array) {
            return array.data;
        }

        template <typename T>
        T* rend(array_t<T>& array) {
            return array.data;
        }

        template <typename T>
        const T* rend(array_t<T>& array) {
            return array.data;
        }

        template <typename T>
        T* rbegin(array_t<T>& array) {
            return array.data + array.size;
        }

        template <typename T>
        const T* rbegin(array_t<T>& array) {
            return array.data + array.size;
        }

        template <typename T>
        u0 append(array_t<T>& array, T&& value) {
            if (array.size + 1 > array.capacity)
                internal::grow(array);
            array.data[array.size++] = value;
        }

        template <typename T>
        u0 resize(array_t<T>& array, u32 new_size) {
            if (new_size > array.capacity)
                internal::grow(array, new_size);
            array.size = new_size;
        }

        template <typename T>
        u0 append(array_t<T>& array, const T& value) {
            if (array.size + 1 > array.capacity)
                internal::grow(array);
            array.data[array.size++] = value;
        }

        template <typename T>
        u0 reserve(array_t<T>& array, u32 new_capacity) {
            if (array.capacity == new_capacity)
                return;

            if (new_capacity < array.size) {
                new_capacity = array.size * 2 + 8;
            }

            T* new_data{};
            if (new_capacity > 0) {
                new_data = memory::allocate(
                    array.allocator,
                    new_capacity * sizeof(T),
                    alignof(T));
                std::memcpy(new_data, array.data, array.size * sizeof(T));
            }

            memory::deallocate(array.allocator, array.data);
            array.data = new_data;
            array.capacity = new_capacity;
        }

        template <typename T>
        b8 contains(array_t<T>& array, const T& value) {
            const T* data = array.data;
            const T* data_end = array.data + array.size;
            while (data < data_end) {
                if (*data++ == value)
                    return true;
            }
            return false;
        }

        template <typename T>
        T* insert(array_t<T>& array, const T* it, T&& value) {
            const auto offset = internal::prepare_insert(array, it);
            array.data[offset] = value;
            ++array.size;
            return array.data + offset;
        }

    }

    ///////////////////////////////////////////////////////////////////////////

    using string_t = array_t<u8>;

    ///////////////////////////////////////////////////////////////////////////

    struct slice_t final {
        u8* data{};
        u32 length{};
    };

}