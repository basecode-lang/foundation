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
#include <basecode/core/error.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/memory/system/buddy.h>

using namespace basecode;

static u8 s_zeros[32];

TEST_CASE("basecode::memory::buddy basics") {
    buddy_config_t config{};
    config.backing = context::top()->alloc;
    config.heap_size = 256 * 1024;

    auto buddy_alloc = memory::system::make(alloc_type_t::buddy, &config);

    // N.B. catch is *slow*!  i want to time allocating the blocks
    //      from the buddy allocator without adding any unnecessary overhead.
    //      after the blocks are allocated and we stop the stopwatch, then it's
    //      ok to run the catch assertions.
    stopwatch_t timer{};
    stopwatch::start(timer);

    const auto num_blocks = config.heap_size / 32;
    u0* blocks[num_blocks];
    for (u32 i = 0; i < num_blocks; ++i)
        blocks[i] = memory::alloc(buddy_alloc, 32);

    stopwatch::stop(timer);
    format::print("buddy alloc: heap_size = {}, block size = {}, # blocks = {}\n", config.heap_size, 32, num_blocks);
    stopwatch::print_elapsed("buddy alloc time"_ss, 40, timer);

    // N.B. this allocation should return nullptr
    REQUIRE(!memory::alloc(buddy_alloc, 32));
    REQUIRE(buddy_alloc->total_allocated == config.heap_size);

    for (u32 i = 0; i < num_blocks; ++i) {
        REQUIRE(blocks[i]);
        auto is_block_zero = std::memcmp(s_zeros, blocks[i], 32) == 0;
        if (!is_block_zero) {
            format::print("block {} non-zero!\n", i);
            format::print_hex_dump(blocks[i], 32);
            format::print("\n");
        }
        REQUIRE(is_block_zero);
#if 0
        format::print("block: {}, valid: {}\n", i, is_block_zero);
        format::print_hex_dump(blocks[i], 32);
        format::print("\n");
#endif
    }

    stopwatch::start(timer);

    for (auto block : blocks)
        memory::free(buddy_alloc, block);

    stopwatch::stop(timer);
    stopwatch::print_elapsed("buddy free time"_ss, 40, timer);

    REQUIRE(buddy_alloc->total_allocated == 0);

    u32 freed_size{};
    memory::system::free(buddy_alloc, true, &freed_size);
}
