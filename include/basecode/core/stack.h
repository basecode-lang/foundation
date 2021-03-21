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
#include <basecode/core/memory.h>
#include <basecode/core/context.h>

namespace basecode {
    template <typename T>
    concept Fixed_Stack = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Base_Value_Type;

        {t.data}                -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
    };

    template <typename T>
    concept Dynamic_Stack = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Base_Value_Type;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.data}                -> same_as<typename T::Value_Type*>;
        {t.size}                -> same_as<u32>;
        {t.capacity}            -> same_as<u32>;
    };

    template <typename T>
    concept Stack = Fixed_Stack<T> || Dynamic_Stack<T>;

    template <typename T, u32 Size = 8>
    struct fixed_stack_t final {
        using Value_Type        = T;
        using Base_Value_Type   = std::remove_pointer_t<T>;

        T                       data[Size];
        u32                     size;
        u32                     capacity = Size;

        T& operator[](u32 index) {
            return data[(index < 0 ? size + (-index - 1) : size - index)];
        }

        const T& operator[](u32 index) const {
            return data[(index < 0 ? size + (-index - 1) : size - index)];
        }
    };

    template <typename T>
    struct stack_t final {
        using Value_Type        = T;
        using Base_Value_Type   = std::remove_pointer_t<T>;

        alloc_t*                alloc;
        T*                      data;
        u32                     size;
        u32                     capacity;

        T& operator[](u32 index) {
            return data[(index < 0 ? size + (-index - 1) : size - index)];
        }

        const T& operator[](u32 index) const {
            return data[(index < 0 ? size + (-index - 1) : size - index)];
        }
    };
    static_assert(sizeof(stack_t<s32>) <= 24, "stack_t<T> is now larger than 24 bytes!");

    namespace stack {
        template <Stack T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>>
        inline auto top(T& stack);

        template <Stack T,
                  typename Value_Type = typename T::Value_Type,
                  b8 Is_Pointer = std::is_pointer_v<Value_Type>>
        inline auto top(const T& stack);

        template <typename T>
        u0 reserve(stack_t<T>& stack, u32 new_capacity);

        template <typename T>
        u0 grow(stack_t<T>& stack, u32 min_capacity = 16);

        template <typename T>
        stack_t<T> make(alloc_t* alloc = context::top()->alloc);

        template <Stack T>
        u0 init(T& stack, alloc_t* alloc = context::top()->alloc);

        template <typename T>
        stack_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        template <Stack T,
                  typename Value_Type = typename T::Value_Type>
        u0 reset(T& stack) {
            stack.size = {};
            std::memset(stack.data, 0, stack.capacity * sizeof(Value_Type));
        }

        template <Stack T>
        u0 clear(T& stack) {
            free(stack);
        }

        template <Stack T, typename Value_Type, b8 Is_Pointer>
        inline auto top(T& stack) {
            if constexpr (Is_Pointer) {
                return (stack.size == 0 ? nullptr : stack.data[stack.size - 1]);
            } else {
                return (stack.size == 0 ? nullptr : &stack.data[stack.size - 1]);
            }
        }

        template <typename T>
        u0 free(stack_t<T>& stack) {
            memory::free(stack.alloc, stack.data);
            stack.data     = {};
            stack.size     = stack.capacity = {};
        }

        template <typename T>
        u0 trim(stack_t<T>& stack) {
            reserve(stack, stack.size);
        }

        template <typename T>
        u0 free(fixed_stack_t<T>& stack) {
            stack.size = {};
        }

        template <typename T>
        inline auto push(stack_t<T>& stack) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            ++stack.size;
            return top(stack);
        }

        template <Stack T>
        inline b8 empty(const T& stack) {
            return stack.size == 0;
        }

        template <Stack T,
                  typename Value_Type = typename T::Value_Type>
        inline Value_Type pop(T& stack) {
            if (stack.size == 0) return (Value_Type) {};
            Value_Type top = stack.data[stack.size - 1];
            --stack.size;
            return top;
        }

        template<typename T>
        stack_t<T> make(alloc_t* alloc) {
            stack_t<T> stack{};
            init(stack, alloc);
            return stack;
        }

        template <Stack T>
        u0 init(T& stack, alloc_t* alloc) {
            stack.data = {};
            stack.alloc = alloc;
            stack.size = stack.capacity = {};
        }

        template <typename T>
        inline auto dup(stack_t<T>& stack) {
            using Value_Type = typename stack_t<T>::Value_Type;
            if (stack.size == 0)
                return Value_Type({});
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            auto top = stack.data[stack.size - 1];
            stack.data[stack.size++] = top;
            return top;
        }

        template <typename T>
        inline auto dup(fixed_stack_t<T>& stack) {
            using Value_Type = typename stack_t<T>::Value_Type;
            if (stack.size == 0)
                return Value_Type({});
            assert(stack.size + 1 < stack.capacity);
            auto top = stack.data[stack.size - 1];
            stack.data[stack.size++] = top;
            return top;
        }

        template <typename T>
        inline auto push(fixed_stack_t<T>& stack) {
            assert(stack.size + 1 < stack.capacity);
            ++stack.size;
            return top(stack);
        }

        template <Stack T, typename Value_Type, b8 Is_Pointer>
        inline auto top(const T& stack) {
            if constexpr (Is_Pointer) {
                return (const Value_Type) (stack.size == 0 ? nullptr : stack.data[stack.size - 1]);
            } else {
                return (const Value_Type*) (stack.size == 0 ? nullptr : &stack.data[stack.size - 1]);
            }
        }

        template <typename T>
        u0 resize(stack_t<T>& stack, u32 new_size) {
            if (new_size > stack.capacity)
                grow(stack, new_size);
            stack.size = new_size;
        }

        template <typename T>
        u0 grow(stack_t<T>& stack, u32 min_capacity) {
            min_capacity = std::max(min_capacity, stack.capacity);
            reserve(stack, min_capacity * 3 + 16);
        }

        template <typename T>
        u0 reserve(stack_t<T>& stack, u32 new_capacity) {
            using Value_Type = typename stack_t<T>::Value_Type;

            if (new_capacity == 0) {
                memory::free(stack.alloc, stack.data);
                stack.data     = {};
                stack.capacity = stack.size = {};
                return;
            }

            if (new_capacity == stack.capacity)
                return;

            new_capacity = std::max(stack.size, new_capacity);
            stack.data = (Value_Type*) memory::realloc(stack.alloc,
                                                       stack.data,
                                                       new_capacity * sizeof(Value_Type),
                                                       alignof(Value_Type));
            const auto data          = stack.data + stack.size;
            const auto size_to_clear = new_capacity > stack.capacity ? new_capacity - stack.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(Value_Type));
            stack.capacity = new_capacity;
        }

        template <typename T>
        inline u32 push(stack_t<T>& stack, auto& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template <typename T>
        inline u32 push(stack_t<T>& stack, auto&& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template <typename T>
        u0 insert(stack_t<T>& stack, u32 index, auto& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            auto target  = stack.data + index;
            auto current = stack.data + stack.size;
            auto prev    = current + 1;
            while (prev <= target)
                *current++ = *prev++;
            *target = value;
            ++stack.size;
        }

        template <typename T>
        inline u32 push(fixed_stack_t<T>& stack, auto&& value) {
            assert(stack.size + 1 < stack.capacity);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template <typename T>
        inline u32 push(fixed_stack_t<T>& stack, auto& value) {
            assert(stack.size + 1 < stack.capacity);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template <typename T>
        inline u32 push(stack_t<T>& stack, const auto& value) {
            if (stack.size + 1 > stack.capacity)
                grow(stack);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template <typename T>
        u0 insert(fixed_stack_t<T>& stack, u32 index, auto& value) {
            assert(stack.size + 1 < stack.capacity);
            auto target  = stack.data + index;
            auto current = stack.data + stack.size;
            auto prev    = current + 1;
            while (prev <= target)
                *current++ = *prev++;
            *target = value;
            ++stack.size;
        }

        template <typename T>
        inline u32 push(fixed_stack_t<T>& stack, const auto& value) {
            assert(stack.size + 1 < stack.capacity);
            stack.data[stack.size++] = value;
            return stack.size;
        }

        template<typename T>
        stack_t<T> make(std::initializer_list<T> elements, alloc_t* alloc) {
            stack_t<T> stack{};
            init(stack, alloc);
            reserve(stack, elements.size());
            for (auto&& e : elements)
                push(stack, e);
            return stack;
        }
    }
}
