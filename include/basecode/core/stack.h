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
    template <typename T, u32 Size = 8>
    struct fixed_stack_t final {
        using Value_Type        = T;
        using Is_Static         = std::integral_constant<b8, true>;

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

    template<typename T>
    struct stack_t final {
        using Value_Type        = T;
        using Is_Static         = std::integral_constant<b8, false>;

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
        u0 free(Stack_Concept auto& stack);

        inline decltype(auto) top(Stack_Concept auto& stack);

        template <typename T>
        stack_t<T> make(alloc_t* alloc = context::top()->alloc);

        inline decltype(auto) top(const Stack_Concept auto& stack);

        u0 reserve(Dynamic_Stack_Concept auto& stack, u32 new_capacity);

        u0 grow(Dynamic_Stack_Concept auto& stack, u32 min_capacity = 16);

        u0 init(Stack_Concept auto& stack, alloc_t* alloc = context::top()->alloc);

        template <typename T>
        stack_t<T> make(std::initializer_list<T> elements, alloc_t* alloc = context::top()->alloc);

        u0 free(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (!T::Is_Static::value) {
                memory::free(stack.alloc, stack.data);
                stack.data     = {};
                stack.capacity = {};
            }
            stack.capacity = {};
        }

        u0 clear(Stack_Concept auto& stack) {
            free(stack);
        }

        u0 reset(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            stack.size = {};
            std::memset(stack.data, 0, stack.capacity * sizeof(typename T::Value_Type));
        }

        u0 trim(Dynamic_Stack_Concept auto& stack) {
            reserve(stack, stack.size);
        }

        inline b8 empty(const Stack_Concept auto& stack) {
            return stack.size == 0;
        }

        u0 init(Stack_Concept auto& stack, alloc_t* alloc) {
            stack.data = {};
            stack.alloc = alloc;
            stack.size = stack.capacity = {};
        }

        inline decltype(auto) pop(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            using Value_Type = typename T::Value_Type;

            if (stack.size == 0) return (Value_Type) {};
            Value_Type top = stack.data[stack.size - 1];
            --stack.size;
            return top;
        }

        inline decltype(auto) dup(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            if (stack.size == 0) return (typename T::Value_Type) {};
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            auto top = stack.data[stack.size - 1];
            stack.data[stack.size++] = top;
            return top;
        }

        template<typename T> stack_t<T> make(alloc_t* alloc) {
            stack_t<T> stack{};
            init(stack, alloc);
            return stack;
        }

        inline decltype(auto) top(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (std::is_pointer_v<typename T::Value_Type>) {
                return (stack.size == 0 ? nullptr : stack.data[stack.size - 1]);
            } else {
                return (stack.size == 0 ? nullptr : &stack.data[stack.size - 1]);
            }
        }

        inline decltype(auto) push(Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            ++stack.size;
            return top(stack);
        }

        inline u32 push(Stack_Concept auto& stack, auto& value) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            stack.data[stack.size++] = value;
            return stack.size;
        }

        inline u32 push(Stack_Concept auto& stack, auto&& value) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            stack.data[stack.size++] = value;
            return stack.size;
        }

        inline decltype(auto) top(const Stack_Concept auto& stack) {
            using T = std::remove_reference_t<decltype(stack)>;
            using Value_Type = typename T::value_type;

            if constexpr (std::is_pointer_v<Value_Type>) {
                return (const Value_Type) (stack.size == 0 ? nullptr : stack.data[stack.size - 1]);
            } else {
                return (const Value_Type*) (stack.size == 0 ? nullptr : &stack.data[stack.size - 1]);
            }
        }

        u0 resize(Dynamic_Stack_Concept auto& stack, u32 new_size) {
            if (new_size > stack.capacity)
                grow(stack, new_size);
            stack.size = new_size;
        }

        u0 insert(Stack_Concept auto& stack, u32 index, auto& value) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            auto target  = stack.data + index;
            auto current = stack.data + stack.size;
            auto prev    = current + 1;
            while (prev <= target)
                *current++ = *prev++;
            *target = value;
            ++stack.size;
        }

        u0 grow(Dynamic_Stack_Concept auto& stack, u32 min_capacity) {
            min_capacity = std::max(min_capacity, stack.capacity);
            reserve(stack, min_capacity * 3 + 16);
        }

        inline u32 push(Stack_Concept auto& stack, const auto& value) {
            using T = std::remove_reference_t<decltype(stack)>;
            if constexpr (T::Is_Static::value) {
                assert(stack.size + 1 < stack.capacity);
            } else {
                if (stack.size + 1 > stack.capacity)
                    grow(stack);
            }
            stack.data[stack.size++] = value;
            return stack.size;
        }

        u0 reserve(Dynamic_Stack_Concept auto& stack, u32 new_capacity) {
            using T = std::remove_reference_t<decltype(stack)>;
            using Value_Type = typename T::Value_Type;

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
