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
#include <basecode/core/context.h>
#include <basecode/core/iterator.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template<typename T> struct stable_array_t final {
        T**                 data;
        alloc_t*            alloc;
        alloc_t*            slab_alloc;
        u32                 size;
        u32                 capacity;

        T& operator[](u32 index)                { return *data[index]; }
        const T& operator[](u32 index) const    { return *data[index]; }

        struct iterator_state_t {
            u32                 pos;

            inline T* get(stable_array_t* ref) {
                return ref->data[pos];
            }
            inline u0 end(const stable_array_t* ref) {
                pos = ref->size;
            }
            inline u0 next(const stable_array_t* ref) {
                ++pos;
            }
            inline u0 begin(const stable_array_t* ref) {
                pos = 0;
            }
            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
            inline const T* get(const stable_array_t* ref) {
                return ref->data[pos];
            }
        };
        DECL_ITERS(stable_array_t, T*, iterator_state_t);
    };

    namespace stable_array {
        template<typename T> u0 clear(stable_array_t<T>& array);
        template<typename T> u0 reserve(stable_array_t<T>& array, u32 new_capacity);
        template<typename T> u0 grow(stable_array_t<T>& array, u32 new_capacity = 8);
        template<typename T> stable_array_t<T> make(alloc_t* alloc = context::top()->alloc);
        template<typename T> u0 init(stable_array_t<T>& array, alloc_t* alloc = context::top()->alloc, u8 num_pages = 1);
        template<typename T> stable_array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template<typename T> u0 pop(stable_array_t<T>& array) {
            if (array.size == 0) return;
            memory::free(array.slab_alloc, array.data[array.size - 1]);
            --array.size;
        }

        template<typename T> u0 free(stable_array_t<T>& array) {
            clear(array);
        }

        template<typename T> u0 trim(stable_array_t<T>& array) {
            reserve(array, array.size);
        }

        template<typename T> T* back(stable_array_t<T>& array) {
            return array.size == 0 ? nullptr : array.data[array.size - 1];
        }

        template<typename T> u0 reset(stable_array_t<T>& array) {
            array.size = {};
            memory::slab::reset(array.slab_alloc);
            std::memset(array.data, 0, array.capacity * sizeof(T*));
        }

        template<typename T> u0 clear(stable_array_t<T>& array) {
            memory::free(array.alloc, array.data);
            memory::system::free(array.slab_alloc);
            array.data = {};
            array.size = array.capacity = {};
        }

        template<typename T> b8 empty(stable_array_t<T>& array) {
            return array.size == 0;
        }

        template<typename T> T* front(stable_array_t<T>& array) {
            return array.size == 0 ? nullptr : array.data[0];
        }

        template<typename T> T& append(stable_array_t<T>& array) {
            if (array.size + 1 > array.capacity)
                grow(array);
            T* v = (T*) memory::alloc(array.slab_alloc);
            array.data[array.size++] = v;
            return *v;
        }

        template<typename T> stable_array_t<T> make(alloc_t* alloc) {
            stable_array_t<T> array;
            init(array, alloc);
            return array;
        }

        template<typename T> u0 erase(stable_array_t<T>& array, u32 index) {
            if (index >= array.size)
                return;
            memory::free(array.slab_alloc, array.data[index]);
            auto dest = array.data + index;
            std::memcpy(dest, dest + 1, (array.size - index) * sizeof(T*));
            --array.size;
        }

        template<typename T> u0 append(stable_array_t<T>& array, T&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            auto v = (T*) memory::alloc(array.slab_alloc);
            *v = value;
            array.data[array.size++] = v;
        }

        template<typename T> u0 resize(stable_array_t<T>& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
            array.size = new_size;
        }

        template<typename T> b8 erase(stable_array_t<T>& array, const T* value) {
            s32 idx = -1;
            for (u32 i = 0; i < array.size; ++i) {
                if (array.data[i] == value) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) return false;
            memory::free(array.slab_alloc, array.data[idx]);
            auto dest = array.data + idx;
            std::memcpy(dest, dest + 1, (array.size - idx) * sizeof(T*));
            --array.size;
            return true;
        }

        template<typename T> u0 grow(stable_array_t<T>& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
        }

        template<typename T> u0 append(stable_array_t<T>& array, const T& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            T* v = (T*) memory::alloc(array.slab_alloc);
            *v = value;
            array.data[array.size++] = v;
        }

        template<typename T> s32 contains(stable_array_t<T>& array, const T* value) {
            for (u32 i = 0; i < array.size; ++i) {
                if (array.data[i] == value)
                    return i;
            }
            return -1;
        }

        template<typename T> u0 reserve(stable_array_t<T>& array, u32 new_capacity) {
            assert(array.alloc);

            if (new_capacity == array.capacity)
                return;

            if (new_capacity == 0) {
                memory::free(array.alloc, array.data);
                array.data = {};
                array.capacity = array.size = {};
                return;
            } else if (new_capacity < array.capacity) {
                for (u32 i = new_capacity - 1; i < array.capacity; ++i)
                    memory::free(array.slab_alloc, array.data[i]);
            }

            new_capacity = std::max(array.size, new_capacity);
            array.data = (T**) memory::realloc(array.alloc, array.data, new_capacity * sizeof(T*), alignof(T*));
            const auto data = array.data + array.size;
            const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(T*));
            array.capacity = new_capacity;
        }

        template<typename T> u0 insert(stable_array_t<T>& array, u32 index, T&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(T*));
            T* v = (T*) memory::alloc(array.slab_alloc);
            *v = value;
            array.data[index] = v;
            ++array.size;
        }

        template<typename T> u0 insert(stable_array_t<T>& array, u32 index, const T& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size)
                std::memmove(array.data + index + 1, array.data + index, (array.size - index) * sizeof(T*));
            T* v = (T*) memory::alloc(array.slab_alloc);
            *v = value;
            array.data[index] = v;
            ++array.size;
        }

        template<typename T> u0 init(stable_array_t<T>& array, alloc_t* alloc, u8 num_pages) {
            array.data  = {};
            array.alloc = alloc;
            array.size  = array.capacity = {};

            slab_config_t slab_config{};
            slab_config.backing   = array.alloc;
            slab_config.buf_size  = sizeof(T);
            slab_config.buf_align = alignof(T);
            slab_config.num_pages = num_pages;
            array.slab_alloc      = memory::system::make(alloc_type_t::slab, &slab_config);
        }

        template<typename T> stable_array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
            stable_array_t<T> array;
            init(array, alloc);
            reserve(array, elements.size());
            for (auto&& e : elements)
                append(array, e);
            return array;
        }
    }
}
