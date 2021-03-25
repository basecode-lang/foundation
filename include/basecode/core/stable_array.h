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
#include <basecode/core/context.h>
#include <basecode/core/iterator.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template <typename T>
    concept Stable_Array = requires(const T& t) {
        typename                T::Value_Type;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.slab}                -> same_as<alloc_t*>;
        {t.data}                -> same_as<typename T::Value_Type**>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
    };

    template<typename T>
    struct stable_array_t final {
        using                   Value_Type = T;

        alloc_t*                alloc;
        alloc_t*                slab;
        Value_Type**            data;
        u32                     size;
        u32                     capacity;

        Value_Type& operator[](u32 index)                               { return *data[index];      }
        const Value_Type& operator[](u32 index) const                   { return *data[index];      }

        struct iterator_state_t {
            u32                 pos;
            inline Value_Type* get(stable_array_t* ref)                 { return ref->data[pos];    }
            inline u0 end(const stable_array_t* ref)                    { pos = ref->size;          }
            inline u0 next(const stable_array_t* ref)                   { UNUSED(ref); ++pos;       }
            inline u0 begin(const stable_array_t* ref)                  { UNUSED(ref); pos = 0;     }
            inline b8 cmp(const iterator_state_t& s) const              { return pos != s.pos;      }
            inline const Value_Type* get(const stable_array_t* ref)     { return ref->data[pos];    }
        };
        DECL_ITERS(stable_array_t, Value_Type*, iterator_state_t);
    };

    namespace stable_array {
        template <Stable_Array T>
        u0 grow(T& array, u32 new_capacity = 16);

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 reserve(T& array, u32 new_capacity);

        template<Stable_Array T>
        u0 pop(T& array) {
            if (array.size == 0) return;
            memory::free(array.slab, array.data[array.size - 1]);
            --array.size;
        }

        template <Stable_Array T>
        u0 clear(T& array) {
            memory::free(array.alloc, array.data);
            memory::system::free(array.slab);
            array.data = {};
            array.size = array.capacity = {};
        }

        template <Stable_Array T>
        u0 free(T& array) {
            clear(array);
        }

        template <Stable_Array T>
        u0 trim(T& array) {
            reserve(array, array.size);
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 reset(T& array) {
            array.size = {};
            memory::slab::reset(array.slab);
            std::memset(array.data, 0, array.capacity * sizeof(Value_Type*));
        }

        template <Stable_Array T>
        b8 empty(T& array) {
            return array.size == 0;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        Value_Type* back(T& array) {
            return array.size == 0 ? nullptr : array.data[array.size - 1];
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        Value_Type* front(T& array) {
            return array.size == 0 ? nullptr : array.data[0];
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        Value_Type& append(T& array) {
            if (array.size + 1 > array.capacity)
                grow(array);
            auto v = (Value_Type*) memory::alloc(array.slab);
            array.data[array.size++] = v;
            return *v;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 erase(T& array, u32 index) {
            if (index >= array.size)
                return;
            memory::free(array.slab, array.data[index]);
            auto dest = array.data + index;
            std::memcpy(dest,
                        dest + 1,
                        (array.size - index) * sizeof(Value_Type*));
            --array.size;
        }

        template <Stable_Array T>
        u0 resize(T& array, u32 new_size) {
            if (new_size > array.capacity)
                grow(array, new_size);
            array.size = new_size;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 append(T& array, Value_Type&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            auto v = (Value_Type*) memory::alloc(array.slab);
            *v = value;
            array.data[array.size++] = v;
        }

        template <Stable_Array T>
        u0 truncate(T& array, u32 index) {
            for (u32 i = index; i < array.size; ++i)
                memory::free(array.slab, array.data[i]);
            array.size = index;
        }

        template <Stable_Array T>
        u0 grow(T& array, u32 new_capacity) {
            new_capacity = std::max(new_capacity, array.capacity);
            reserve(array, new_capacity * 2 + 8);
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        b8 erase(T& array, const Value_Type* value) {
            s32 idx = -1;
            for (u32 i = 0; i < array.size; ++i) {
                if (array.data[i] == value) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) return false;
            memory::free(array.slab, array.data[idx]);
            --array.size;
            auto dest = array.data + idx;
            std::memcpy(dest,
                        dest + 1,
                        (array.size - idx) * sizeof(Value_Type*));
            return true;
        }

        template <Stable_Array T, typename Value_Type>
        u0 reserve(T& array, u32 new_capacity) {
            if (new_capacity == array.capacity)
                return;

            if (new_capacity == 0) {
                memory::free(array.alloc, array.data);
                array.data = {};
                array.capacity = array.size = {};
                return;
            } else if (new_capacity < array.capacity) {
                for (u32 i = new_capacity - 1; i < array.capacity; ++i)
                    memory::free(array.slab, array.data[i]);
            }

            new_capacity = std::max(array.size, new_capacity);
            array.data = (Value_Type**) memory::realloc(array.alloc,
                                                       array.data,
                                                       new_capacity * sizeof(Value_Type*),
                                                       alignof(Value_Type*));
            const auto data = array.data + array.size;
            const auto size_to_clear = new_capacity > array.capacity ? new_capacity - array.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(Value_Type*));
            array.capacity = new_capacity;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 append(T& array, const Value_Type& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            auto v = (Value_Type*) memory::alloc(array.slab);
            *v = value;
            array.data[array.size++] = v;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        s32 contains(T& array, const Value_Type* value) {
            for (u32 i = 0; i < array.size; ++i) {
                if (array.data[i] == value)
                    return i;
            }
            return -1;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 insert(T& array, u32 index, Value_Type&& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size) {
                std::memmove(array.data + index + 1,
                             array.data + index,
                             (array.size - index) * sizeof(Value_Type*));
            }
            auto v = (Value_Type*) memory::alloc(array.slab);
            *v = value;
            array.data[index] = v;
            ++array.size;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 insert(T& array, u32 index, const Value_Type& value) {
            if (array.size + 1 > array.capacity)
                grow(array);
            if (index < array.size) {
                std::memmove(array.data + index + 1,
                             array.data + index,
                             (array.size - index) * sizeof(Value_Type*));
            }
            auto v = (Value_Type*) memory::alloc(array.slab);
            *v = value;
            array.data[index] = v;
            ++array.size;
        }

        template <Stable_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 init(T& array, alloc_t* alloc = context::top()->alloc, u8 num_pages = 1) {
            array.data  = {};
            array.alloc = alloc;
            array.size  = array.capacity = {};

            slab_config_t slab_config{};
            slab_config.backing   = array.alloc;
            slab_config.buf_size  = sizeof(Value_Type);
            slab_config.buf_align = alignof(Value_Type);
            slab_config.num_pages = num_pages;
            array.slab            = memory::system::make(alloc_type_t::slab, &slab_config);
        }

        template <typename T>
        stable_array_t<T> make(alloc_t* alloc = context::top()->alloc) {
            stable_array_t<T> array;
            init(array, alloc);
            return array;
        }

        template <typename T>
        stable_array_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc) {
            stable_array_t<T> array;
            init(array, alloc);
            reserve(array, elements.size());
            for (auto&& e : elements)
                append(array, e);
            return array;
        }
    }
}
