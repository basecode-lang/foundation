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
#include <basecode/core/slice/slice.h>
#include <basecode/core/memory/system.h>
#include <basecode/core/context/system.h>

namespace basecode {
    template<typename T> struct array_t final {
        T*                  data;
        alloc_t*            alloc;
        u32                 size;
        u32                 capacity;

        T* end()                                { return data + size;   }
        T* rend()                               { return data;          }
        T* begin()                              { return data;          }
        T* rbegin()                             { return data + size;   }
        const T* end() const                    { return data + size;   }
        const T* rend() const                   { return data;          }
        const T* begin() const                  { return data;          }
        const T* rbegin() const                 { return data + size;   }
        T& operator[](u32 index)                { return data[index];   }
        const T& operator[](u32 index) const    { return data[index];   }
    };

    namespace array {
        template<typename T> u0 grow(array_t<T>& array, u32 new_capacity = 8);
        template<typename T> array_t<T> make(alloc_t* alloc = context::top()->alloc);
        template<typename T> u0 reserve(array_t<T>& array, u32 new_capacity, b8 copy = true);
        template<typename T> u0 init(array_t<T>& array, alloc_t* alloc = context::top()->alloc);
        template<typename T> array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template<typename T> u0 pop(array_t<T>& array) {
            --array.size;
        }

        template<typename T> u0 free(array_t<T>& array) {
            if (array.alloc)
                memory::free(array.alloc, array.data);
            array.data = {};
            array.size = array.capacity = {};
        }

        template<typename T> u0 trim(array_t<T>& array) {
            reserve(array, array.size);
        }

        template<typename T> T& back(array_t<T>& array) {
            return array.data[array.size - 1];
        }

        template<typename T> u0 reset(array_t<T>& array) {
            array.size = {};
            std::memset(array.data, 0, sizeof(T) * array.capacity);
        }

        template<typename T> u0 clear(array_t<T>& array) {
            free(array);
        }

        template<typename T> b8 empty(array_t<T>& array) {
            return array.size == 0;
        }

        template<typename T> T& front(array_t<T>& array) {
            return array.data[0];
        }

        template<typename T> T& append(array_t<T>& array) {
            if (array.size + 1 > array.capacity)
                grow(array);
            return array.data[array.size++];
        }

        template<typename T> const T& back(array_t<T>& array) {
            return array.data[array.size - 1];
        }

        template<typename T> const T& front(array_t<T>& array) {
            return array.data[0];
        }

        template<typename T> array_t<T> make(alloc_t* alloc) {
            array_t<T> array;
            init(array, alloc);
            return array;
        }

        template<typename T> u0 erase(array_t<T>& array, u32 index) {
            --array.size;
            if (index > array.size)
                return;
            auto dest = array.data + index;
            std::memcpy(dest, dest + 1, (array.size + 1) * sizeof(T));
        }

        template<typename T> u0 append(array_t<T>& array, T&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            array.data[array.size++] = value;
        }

        template<typename T> u0 resize(array_t<T>& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
            array.size = new_size;
        }

        template<typename T> u0 grow(array_t<T>& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
        }

        template<typename T> u0 append(array_t<T>& array, const T& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            array.data[array.size++] = value;
        }

        template<typename T> u0 init(array_t<T>& array, alloc_t* alloc) {
            array.data = {};
            array.alloc = alloc;
            array.size = array.capacity = {};
        }

        template<typename T> s32 contains(array_t<T>& array, const T& value) {
            const T* data = array.data;
            const T* data_end = array.data + array.size;
            while (data < data_end) {
                if (*data++ == value)
                    return (data - array.data) - 1;
            }
            return -1;
        }

        template<typename T> u0 insert(array_t<T>& array, u32 index, T&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size) {
                auto dest = array.data + index + 1;
                std::memmove(dest, dest - 1, (array.size - index) * sizeof(T));
            }
            array.data[index] = value;
            ++array.size;
        }

        template<typename T> u0 insert(array_t<T>& array, u32 index, const T& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size) {
                auto dest = array.data + index + 1;
                std::memmove(dest, dest - 1, (array.size - index) * sizeof(T));
            }
            array.data[index] = value;
            ++array.size;
        }

        template<typename T> u0 reserve(array_t<T>& array, u32 new_capacity, b8 copy) {
            assert(array.alloc);

            if (array.capacity == new_capacity)
                return;

            if (new_capacity < array.size)
                new_capacity = array.size;

            T* new_data{};
            if (new_capacity > 0) {
                new_data = (T*) memory::alloc(array.alloc, new_capacity * sizeof(T), alignof(T));
                std::memset(new_data, 0, new_capacity * sizeof(T));
                if (array.data && copy) {
                    std::memcpy(new_data, array.data, array.size * sizeof(T));
                }
            }

            memory::free(array.alloc, array.data);
            array.data = new_data;
            array.capacity = new_capacity;
        }

        template<typename T> array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
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
    template <typename T> inline slice_t<T> make(array_t<T>& array) {
        return slice_t{.data = array.data, .length = array.size};
    }

    template <typename T> inline slice_t<T> make(array_t<T>& array, u32 start, u32 end) {
        return slice_t{.data = array.data + start, .length = end - start};
    }
}