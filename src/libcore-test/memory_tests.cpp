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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/array.h>
#include <basecode/core/memory/system/stack.h>
#include <basecode/core/memory/system/scratch.h>

using namespace basecode;

TEST_CASE("basecode::memory scratch allocator") {
    alloc_t scratch_alloc{};

    scratch_config_t cfg{};
    cfg.backing = context::top()->alloc;
    cfg.buf_size = 256 * 1024;

    memory::init(&scratch_alloc, alloc_type_t::scratch, &cfg);
    defer(memory::fini(&scratch_alloc));

    auto p1 = (u8*) memory::alloc(&scratch_alloc, 10 * 1024);
    REQUIRE(p1);
    REQUIRE(memory::size(&scratch_alloc, p1) == 10 * 1024);

    u8* p2[100];
    for (s32 i = 0; i < 100; ++i)
        p2[i] = (u8*) memory::alloc(&scratch_alloc, 1024);
    for (s32 i = 0; i < 100; ++i) {
        REQUIRE(p2[i]);
        REQUIRE(memory::size(&scratch_alloc, p2[i]) == 1024);
        REQUIRE(memory::free(&scratch_alloc, p2[i]) == 1032);
    }

    REQUIRE(memory::free(&scratch_alloc, p1) == (10 * 1024) + 8);

    for (s32 i = 0; i < 50; ++i)
        p2[i] = (u8*) memory::alloc(&scratch_alloc, 4096);
    for (s32 i = 0; i < 50; ++i) {
        REQUIRE(p2[i]);
        REQUIRE(memory::size(&scratch_alloc, p2[i]) == 4096);
        REQUIRE(memory::free(&scratch_alloc, p2[i]) == 4104);
    }
}

TEST_CASE("basecode::memory stack allocator") {
    alloc_t stack_alloc{};

    stack_config_t cfg{};
    cfg.backing  = context::top()->alloc;
    cfg.max_size = 4096;

    memory::init(&stack_alloc, alloc_type_t::stack, &cfg);
    defer(memory::fini(&stack_alloc));

    array_t<u32> items{};
    array::init(items, &stack_alloc);
    array::reserve(items, 1023);
    for (u32 i = 0; i < 1023; ++i)
        array::append(items, i);

    REQUIRE(stack_alloc.total_allocated == items.size * sizeof(u32));
}
