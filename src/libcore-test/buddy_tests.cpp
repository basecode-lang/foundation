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
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/memory/system/buddy.h>

using namespace basecode;

TEST_CASE("basecode::memory::buddy basics", "[!hide]") {
    buddy_config_t config{};
    config.backing = context::top()->alloc;
    config.heap_size = 256 * 1024;

    auto buddy_alloc = memory::system::make(alloc_type_t::buddy, &config);
    auto sc = &buddy_alloc->subclass.buddy;

    u0* blocks[8192];
    for (u32 i = 0; i < 8192; ++i) {
        blocks[i] = memory::alloc(buddy_alloc, 32);
        REQUIRE(blocks[i]);
    }

    REQUIRE(sc->reserved == 0);

    for (auto block : blocks) {
        memory::free(buddy_alloc, block);
    }

    u32 freed_size{};
    memory::system::free(buddy_alloc, true, &freed_size);
}
