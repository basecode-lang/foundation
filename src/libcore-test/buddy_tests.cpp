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
#include <basecode/core/error.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/memory/system/buddy.h>

using namespace basecode;

TEST_CASE("basecode::memory::buddy basics") {
    buddy_config_t config{};
    config.heap_size     = 256 * 1024;
    config.backing.alloc = context::top()->alloc.main;

    auto buddy_alloc = memory::system::make(&config);
    const auto working_heap_size = buddy_alloc->subclass.buddy.size
                                   - buddy_alloc->subclass.buddy.metadata_size;

    // N.B. catch is *slow*!  i want to time allocating the blocks
    //      from the buddy allocator without adding any unnecessary overhead.
    //      after the blocks are allocated and we stop the stopwatch, then it's
    //      ok to run the catch assertions.
    const auto block_size = 32;
    const auto num_blocks = working_heap_size / block_size;
    u0* blocks[num_blocks];
    TIME_BLOCK("buddy alloc time"_ss,
               for (u32 i = 0; i < num_blocks; ++i) {
                   blocks[i] = memory::alloc(buddy_alloc, block_size);
               });
    format::print("buddy alloc: heap_size = {}, block size = {}, # blocks = {}\n",
                  working_heap_size,
                  block_size,
                  num_blocks);

    // N.B. this allocation should return nullptr
    REQUIRE(!memory::alloc(buddy_alloc, block_size));

    // N.B. we may not be able to perfectly utilize the entire heap
    REQUIRE(buddy_alloc->total_allocated > working_heap_size);

    for (u32 i = 0; i < num_blocks; ++i) {
        if (!blocks[i])
            REQUIRE(false);
    }

    TIME_BLOCK("buddy free time"_ss,
               for (u32 i = 0; i < num_blocks; ++i) {
                   memory::free(buddy_alloc, blocks[i]);
               });

    REQUIRE(buddy_alloc->total_allocated == buddy_alloc->subclass.buddy.metadata_size);

    memory::system::free(buddy_alloc);
}
