// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
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
#include <basecode/core/memory/system.h>
#include <basecode/core/context/system.h>

namespace basecode {
    template<typename T> struct stack_t final {
        T*                      data;
        alloc_t*                alloc;
        u32                     size;
        u32                     capacity;

        T& operator[](u32 index) {
            if (index < 0) {
                return data[size + (-index - 1)];
            } else {
                return data[size - index];
            }
        }

        const T& operator[](u32 index) const {
            if (index < 0) {
                return data[size + (-index - 1)];
            } else {
                return data[size - index];
            }
        }
    };

    namespace stack {
        template<typename T> u0 clear(stack_t<T>& stack);
        template<typename T> u32 push(stack_t<T>& stack, T&& value);
        template<typename T> u0 grow(stack_t<T>& stack, u32 min_capacity = 8);
        template <typename T> stack_t<T> make(alloc_t* alloc = context::top()->alloc);
        template<typename T> u0 reserve(stack_t<T>& stack, u32 new_capacity, b8 copy = true);
        template<typename T> u0 init(stack_t<T>& stack, alloc_t* alloc = context::top()->alloc);
        template<typename T> stack_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template<typename T> T pop(stack_t<T>& stack) {
            if (stack.size == 0) return {};
            T top = stack.data[stack.size - 1];
            --stack.size;
            return top;
        }

        template<typename T> u0 trim(stack_t<T>& stack) {
            reserve(stack, stack.size);
        }

        template<typename T> u0 free(stack_t<T>& stack) {
            clear(stack);
        }

        template<typename T> u0 reset(stack_t<T>& stack) {
            stack.size = {};
            std::memset(stack.data, 0, sizeof(T) * stack.capacity);
        }

        template<typename T> b8 empty(stack_t<T>& stack) {
            return stack.size == 0;
        }

        template<typename T> u0 clear(stack_t<T>& stack) {
            memory::free(stack.alloc, stack.data);
            stack.data = {};
            stack.size = stack.capacity = {};
        }

        template<typename T> stack_t<T> make(alloc_t* alloc) {
            stack_t<T> stack{};
            init(stack, alloc);
            return stack;
        }

        template<typename T> decltype(auto) top(stack_t<T>& stack) {
            if constexpr (std::is_pointer_v<T>) {
                return (T) stack.data[stack.size - 1];
            } else {
                return (T*) &stack.data[stack.size - 1];
            }
        }

        template<typename T> u32 push(stack_t<T>& stack, T& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template<typename T> u32 push(stack_t<T>& stack, T&& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template<typename T> decltype(auto) push(stack_t<T>& stack) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            ++stack.size;
            return top(stack);
        }

        template<typename T> u0 init(stack_t<T>& stack, alloc_t* alloc) {
            stack.data = {};
            stack.alloc = alloc;
            stack.size = stack.capacity = {};
        }

        template<typename T> u0 resize(stack_t<T>& stack, u32 new_size) {
            if (new_size > stack.capacity)
                grow(stack, new_size);
            stack.size = new_size;
        }

        template<typename T> u32 push(stack_t<T>& stack, const T& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template<typename T> u0 grow(stack_t<T>& stack, u32 min_capacity) {
            min_capacity = std::max(min_capacity, stack.capacity);
            reserve(stack, min_capacity * 2 + 8);
        }

        template<typename T> u0 insert(stack_t<T>& stack, u32 index, T& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            auto target = stack.data + index;
            auto current = stack.data + stack.size;
            auto prev = current + 1;
            while (prev <= target)
                *current++ = *prev++;
            *target = value;
            ++stack.size;
        }

        template<typename T> u0 reserve(stack_t<T>& stack, u32 new_capacity, b8 copy) {
            if (stack.capacity == new_capacity)
                return;

            if (new_capacity < stack.size)
                new_capacity = stack.size;

            T* new_data{};
            if (new_capacity > 0) {
                new_data = (T*) memory::alloc(stack.alloc, new_capacity * sizeof(T), alignof(T));
                std::memset(new_data, 0, new_capacity * sizeof(T));
                if (stack.data && copy)
                    std::memcpy(new_data, stack.data, stack.size * sizeof(T));
            }

            memory::free(stack.alloc, stack.data);
            stack.data = new_data;
            stack.capacity = new_capacity;
        }

        template<typename T> stack_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
            stack_t<T> stack{};
            init(stack, alloc);
            reserve(stack, elements.size());
            for (auto&& e : elements)
                push(stack, e);
            return stack;
        }
    }
}
