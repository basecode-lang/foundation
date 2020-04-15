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

#include <catch2/catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/stack/stack.h>

using namespace basecode;

TEST_CASE("basecode::stack basics") {
    auto numbers = stack::make<s32>({0, 1, 2, 3, 4, 5, 6, 7, 8});
    defer(stack::free(numbers));

    REQUIRE(!stack::empty(numbers));
    REQUIRE(numbers.size == 9);

    auto n = 8;
    while (!stack::empty(numbers)) {
        auto top = *stack::top(numbers);
        REQUIRE(top == n--);
        stack::pop(numbers);
    }

    REQUIRE(stack::empty(numbers));

    stack::push(numbers, 100);
    stack::push(numbers, 200);
    stack::push(numbers, 300);
    REQUIRE(numbers.size == 3);

    n = 300;
    while (!stack::empty(numbers)) {
        auto top = *stack::top(numbers);
        REQUIRE(top == n);
        n -= 100;
        stack::pop(numbers);
    }

    stack::reset(numbers);
    REQUIRE(stack::empty(numbers));
}

TEST_CASE("basecode::stack force grow multiple times") {
    stack::stack_t<s32> numbers{};
    stack::init(numbers, context::current()->allocator);

    REQUIRE(stack::empty(numbers));

    for (s32 i = 0; i < 1000; ++i)
        stack::push(numbers, i);

    REQUIRE(numbers.size == 1000);

    auto n = 999;
    while (!stack::empty(numbers)) {
        auto top = *stack::top(numbers);
        REQUIRE(top == n--);
        stack::pop(numbers);
    }

    REQUIRE(stack::empty(numbers));

    stack::push(numbers, 100);
    stack::push(numbers, 200);
    stack::push(numbers, 300);
    REQUIRE(numbers.size == 3);

    n = 300;
    while (!stack::empty(numbers)) {
        auto top = *stack::top(numbers);
        REQUIRE(top == n);
        n -= 100;
        stack::pop(numbers);
    }

    stack::reset(numbers);
    REQUIRE(stack::empty(numbers));
}

TEST_CASE("basecode::stack insert") {
    stack::stack_t<s32> numbers{};
    stack::init(numbers, context::current()->allocator);

    REQUIRE(stack::empty(numbers));

    for (s32 i = 0; i < 10; ++i)
        stack::push(numbers, i);

    REQUIRE(numbers.size == 10);

    auto temp = 30;
    stack::insert(numbers, 3, temp);

    REQUIRE(numbers.size == 11);

    temp = 100;
    stack::insert(numbers, numbers.size, temp);
    REQUIRE(numbers.size == 12);
}
