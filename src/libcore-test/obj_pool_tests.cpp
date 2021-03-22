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
#include <basecode/core/obj_pool.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::obj_pool basics") {
    stopwatch_t timer{};
    stopwatch::start(timer);

    obj_pool_t pool{};
    obj_pool::init(pool);

    array_t<str_t*> strings{};
    array::init(strings);
    defer(array::free(strings); obj_pool::free(pool));

    for (u32 i = 0; i < 100; ++i) {
        array::append(strings, obj_pool::make<str_t>(pool, "hello world!"));
    }

    stopwatch::stop(timer);
    stopwatch::print_elapsed("total obj_pool make time"_ss, 40, timer);

    REQUIRE(!obj_pool::empty(pool));
    REQUIRE(strings.size == 100);
    REQUIRE(pool.size == strings.size);

    stopwatch::start(timer);
    for (u32 i = 0; i < strings.size; ++i)
        obj_pool::destroy(pool, strings[i]);
    stopwatch::stop(timer);
    stopwatch::print_elapsed("total obj_pool destroy time"_ss, 40, timer);

    REQUIRE(hashtab::empty(pool.storage));
    REQUIRE(obj_pool::empty(pool));
}
