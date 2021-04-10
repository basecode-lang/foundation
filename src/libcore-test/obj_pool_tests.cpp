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

#include <catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/obj_pool.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::obj_pool basics", "[obj_pool]") {
    obj_pool_t pool{};
    obj_pool::init(pool);

    array_t<str_t*> strings{};
    array::init(strings);
    defer(
        array::free(strings);
        obj_pool::free(pool, false));

    TIME_BLOCK(
        "obj_pool: make 100 str_t instances"_ss,
        for (u32 i = 0; i < 100; ++i) {
            array::append(strings, obj_pool::make<str_t>(pool, "hello world!"));
        });

    REQUIRE(!obj_pool::empty(pool));
    REQUIRE(strings.size == 100);
    REQUIRE(pool.size == strings.size);

    TIME_BLOCK(
        "obj_pool: destroy 100 str_t instances"_ss,
        for (u32 i = 0; i < strings.size; ++i)
            obj_pool::destroy(pool, strings[i]););

    REQUIRE(obj_pool::empty(pool));
}
