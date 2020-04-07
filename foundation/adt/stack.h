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
#include <foundation/types.h>
#include <foundation/memory/system.h>

namespace basecode::stack {
    template<typename T>
    struct stack_t final {
        explicit stack_t(memory::allocator_t* allocator = context::current()->allocator);

        ~stack_t();

        stack_t(const stack_t<T>& other);

        stack_t(stack_t<T>&& other) noexcept;

        T* sp{};
        T* data{};
        u32 size{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        T& operator[](u32 index);

        const T& operator[](u32 index) const;

        stack_t<T>& operator=(const stack_t<T>& other);

        stack_t<T>& operator=(stack_t&& other) noexcept;
    };

    ///////////////////////////////////////////////////////////////////////////

    template <typename T> u0 clear(stack_t<T>&);
    template <typename T> u32 push(stack_t<T>&, T&&);
    template <typename T> u0 reserve(stack_t<T>&, u32, b8 copy = true);

    ///////////////////////////////////////////////////////////////////////////

    template <typename T> u0 grow(
            stack_t<T>& stack,
            u32 new_capacity = 0) {
        new_capacity = std::max(new_capacity, stack.capacity);
        reserve(stack, new_capacity * 2 + 8);
    }

    template <typename T> u0 init(
            stack_t<T>* stack,
            memory::allocator_t* allocator = context::current()->allocator) {
        stack->sp = stack->data = {};
        stack->allocator = allocator;
        stack->size = stack->capacity = {};
    }

    template <typename T> u0 reserve(
            stack_t<T>& stack,
            u32 new_capacity,
            b8 copy) {
        if (stack.capacity == new_capacity)
            return;

        if (new_capacity < stack.size)
            new_capacity = stack.size;

        T* new_data{};
        if (new_capacity > 0) {
            new_data = (T*) memory::allocate(
                stack.allocator,
                new_capacity * sizeof(T),
                alignof(T));
            std::memset(new_data, 0, new_capacity * sizeof(T));
            if (stack.data && copy) {
                std::memcpy(
                    new_data,
                    stack.data,
                    stack.size * sizeof(T));
            }
        }

        memory::deallocate(stack.allocator, stack.data);
        stack.data = new_data;
        stack.capacity = new_capacity;
        stack.sp = stack.data + stack.capacity;
    }

    template <typename T> stack_t<T> make(
            std::initializer_list<T> elements,
            memory::allocator_t* allocator = context::current()->allocator) {
        stack_t<T> stack(allocator);
        for (auto&& e : elements)
            push(stack, e);
        return stack;
    }

    template <typename T> u0 trim(stack_t<T>& stack) {
        reserve(stack, stack.size);
    }

    template <typename T> u0 free(stack_t<T>& stack) {
        clear(stack);
    }

    template <typename T> u0 reset(stack_t<T>& stack) {
        stack.size = {};
        stack.sp = stack.data;
        std::memset(stack.data, 0, sizeof(T) * stack.capacity);
    }

    template <typename T> u0 clear(stack_t<T>& stack) {
        memory::deallocate(stack.allocator, stack.data);
        stack.data = stack.sp = {};
        stack.size = stack.capacity = {};
    }

    template <typename T> b8 empty(stack_t<T>& stack) {
        return stack.size == 0;
    }

    template <typename T> stack_t<T> make(memory::allocator_t* allocator = context::current()->allocator) {
        return stack_t<T>(allocator);
    }

    template <typename T> void pop(stack_t<T>& stack) {
        if (stack.sp < stack.data + stack.size)
            stack.sp++;
    }

    template <typename T> decltype(auto) top(stack_t<T>& stack) {
        if constexpr (std::is_pointer_v<T>) {
            return (T) *stack.sp;
        } else {
            return (T*) &(*stack.sp);
        }
    }

    template <typename T> u32 push(stack_t<T>& stack, T& value) {
        if (stack.size + 1 > stack.capacity)
            grow(stack);
        *(--stack.sp) = value;
        ++stack.size;
        return stack.size;
    }

    template <typename T> u32 push(stack_t<T>& stack, T&& value) {
        if (stack.size + 1 > stack.capacity)
            grow(stack);
        *(--stack.sp) = value;
        ++stack.size;
        return stack.size;
    }

    template <typename T> u32 push(stack_t<T>& stack, const T& value) {
        if (stack.size + 1 > stack.capacity)
            grow(stack);
        *(--stack.sp) = value;
        ++stack.size;
        return stack.size;
    }

    template <typename T> u0 resize(stack_t<T>& stack, u32 new_size) {
        if (new_size > stack.capacity)
            grow(stack, new_size);
        stack.size = new_size;
        stack.sp = stack.data + stack.size;
    }

    template <typename T> u0 insert(stack_t<T>& stack, u32 index, T& value) {
        if (stack.size + 1 > stack.capacity)
            grow(stack);
        stack.sp--;
        auto target = stack.sp + index;
        auto current = stack.sp;
        auto prev = current + 1;
        while (prev <= target)
            *current++ = *prev++;
        *target = value;
    }

    template<typename T> inline stack_t<T>::stack_t(memory::allocator_t* allocator) : allocator(allocator) {
        assert(allocator);
    }

    template<typename T> inline stack_t<T>::~stack_t() {
        clear(*this);
    }

    template<typename T> inline stack_t<T>::stack_t(const stack_t<T>& other) {
        operator=(other);
    }

    template<typename T> inline stack_t<T>::stack_t(stack_t<T>&& other) noexcept {
        operator=(other);
    }

    template<typename T> T& stack_t<T>::operator[](u32 index) {
        if (index < 0) {
            index = -index;
            return sp + (index - 1);
        } else {
            return (data + size) - index;
        }
    }

    template<typename T> const T& stack_t<T>::operator[](u32 index) const {
        if (index < 0) {
            index = -index;
            return sp + (index - 1);
        } else {
            return (data + size) - index;
        }
    }

    template<typename T> stack_t<T>& stack_t<T>::operator=(const stack_t<T>& other) {
        if (this == &other)
            return *this;
        if (!allocator)
            allocator = other.allocator;
        reserve(*this, other.size, false);
        std::memcpy(data, other.data, sizeof(T) * other.size);
        sp = data + size;
        size = other.size;
        return *this;
    }

    template<typename T> stack_t<T>& stack_t<T>::operator=(stack_t<T>&& other) noexcept {
        if (this == &other)
            return *this;
        clear(*this);
        sp = other.sp;
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = other.size = other.capacity = {};
        return *this;
    }
}
