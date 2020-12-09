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
#include <basecode/core/array.h>
#include <basecode/core/memory/system/stack.h>

using namespace basecode;

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
