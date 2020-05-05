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
#include "types.h"
#include "slice.h"
#include "context.h"
#include "memory/memory.h"

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
        template<typename T> u0 clear(array_t<T>& array);
        template<typename T> u0 reserve(array_t<T>& array, u32 new_capacity);
        template<typename T> u0 grow(array_t<T>& array, u32 new_capacity = 8);
        template<typename T> array_t<T> make(alloc_t* alloc = context::top()->alloc);
        template<typename T> u0 init(array_t<T>& array, alloc_t* alloc = context::top()->alloc);
        template<typename T> array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template<typename T> u0 pop(array_t<T>& array) {
            --array.size;
        }

        template<typename T> u0 free(array_t<T>& array) {
            clear(array);
        }

        template<typename T> u0 trim(array_t<T>& array) {
            reserve(array, array.size);
        }

        template<typename T> u0 reset(array_t<T>& array) {
            array.size = {};
            std::memset(array.data, 0, array.capacity * sizeof(T));
        }

        template<typename T> u0 clear(array_t<T>& array) {
            assert(array.alloc);
            memory::free(array.alloc, array.data);
            array.data = {};
            array.size = array.capacity = {};
        }

        template<typename T> b8 empty(array_t<T>& array) {
            return array.size == 0;
        }

        template<typename T> T& append(array_t<T>& array) {
            if (array.size + 1 > array.capacity)
                grow(array);
            return array.data[array.size++];
        }

        template<typename T> array_t<T> make(alloc_t* alloc) {
            array_t<T> array;
            init(array, alloc);
            return array;
        }

        template<typename T> u0 erase(array_t<T>& array, u32 index) {
            if (index >= array.size)
                return;
            auto dest = array.data + index;
            std::memcpy(dest, dest + 1, (array.size - index) * sizeof(T));
            --array.size;
        }

        template<typename T> decltype(auto) back(array_t<T>& array) {
            if constexpr (std::is_pointer_v<T>) {
                return array.size == 0 ? nullptr : array.data[array.size - 1];
            } else {
                return array.size == 0 ? nullptr : &array.data[array.size - 1];
            }
        }

        template<typename T> decltype(auto) front(array_t<T>& array) {
            if constexpr (std::is_pointer_v<T>) {
                return array.size == 0 ? nullptr : array.data[0];
            } else {
                return array.size == 0 ? nullptr : &array.data[0];
            }
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

        template<typename T> u0 init(array_t<T>& array, alloc_t* alloc) {
            array.data = {};
            array.alloc = alloc;
            array.size = array.capacity = {};
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

        template<typename T> s32 contains(array_t<T>& array, const T& value) {
            for (u32 i = 0; i < array.size; ++i) {
                if (array.data[i] == value)
                    return i;
            }
            return -1;
        }

        template<typename T> u0 reserve(array_t<T>& array, u32 new_capacity) {
            assert(array.alloc);

            if (new_capacity == 0) {
                memory::free(array.alloc, array.data);
                array.data = {};
                array.capacity = array.size = {};
                return;
            }

            if (new_capacity == array.capacity)
                return;

            new_capacity = std::max(array.size, new_capacity);
            if (!array.data) {
                array.data = (T*) memory::alloc(array.alloc, new_capacity * sizeof(T), alignof(T));
                std::memset(array.data, 0, new_capacity * sizeof(T));
            } else {
                array.data = (T*) memory::realloc(array.alloc, array.data, new_capacity * sizeof(T), alignof(T));
            }
            array.capacity = new_capacity;
        }

        template<typename T> u0 insert(array_t<T>& array, u32 index, T&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(T));
            array.data[index] = value;
            ++array.size;
        }

        template<typename T> u0 insert(array_t<T>& array, u32 index, const T& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(T));
            array.data[index] = value;
            ++array.size;
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
