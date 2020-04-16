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
#include <basecode/core/array/array.h>
#include <basecode/core/intern/intern.h>
#include <basecode/core/format/system.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/profiler/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::intern simple") {
    intern::pool_t pool{};
    intern::pool::init(pool);
    defer(intern::pool::free(pool));

    auto r1 = intern::pool::intern(pool, "test1"_ss);
    REQUIRE(r1.status == intern::status_t::ok);
    REQUIRE(r1.slice.data);
    REQUIRE(r1.slice.length > 0);

    auto r2 = intern::pool::intern(pool, "test2"_ss);
    REQUIRE(r2.status == intern::status_t::ok);
    REQUIRE(r2.slice.data);
    REQUIRE(r2.slice.length > 0);

    auto r3 = intern::pool::intern(pool, "test3"_ss);
    REQUIRE(r3.status == intern::status_t::ok);
    REQUIRE(r3.slice.data);
    REQUIRE(r3.slice.length > 0);

    auto r4 = intern::pool::intern(pool, "test1"_ss);
    REQUIRE(r4.id == r1.id);
    REQUIRE(r4.hash == r1.hash);
    REQUIRE(r4.slice == r1.slice);
    REQUIRE(r4.status == r1.status);
}

TEST_CASE("basecode::intern") {
    constexpr auto expected_intern_count = 200;

    const auto allocated_before = context::top()->alloc->total_allocated;

    intern::pool_t pool{};
    intern::pool::init(pool);
    defer(intern::pool::free(pool));

    array_t<intern::result_t> interned_list{};
    array::init(interned_list);
    array::reserve(interned_list, expected_intern_count, false);
    defer(array::free(interned_list));

    defer({
        const auto memory_used = context::top()->alloc->total_allocated - allocated_before;
        format::print("memory_used = {}\n", memory_used);
    });

    string_t str[expected_intern_count];
    for (u32 i = 0; i < expected_intern_count; ++i) {
        string::init(str[i], context::top()->alloc);
        string::reserve(str[i], 64);
        string::append(str[i], "aaaaaAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBCCCCCCC");
        str[i] = str[i] + string::random(2);
    }

    stopwatch_t intern_time{};
    stopwatch::start(intern_time);

    for (u32 i = 0; i < expected_intern_count; ++i) {
        auto r = intern::pool::intern(pool, slice::make(str[i]));
        REQUIRE(r.status == intern::status_t::ok);
        REQUIRE(r.slice.data);
        REQUIRE(r.slice.length > 0);
        array::append(interned_list, r);
    }

    stopwatch::stop(intern_time);
    stopwatch::print_elapsed("total intern time"_ss, 40, stopwatch::elapsed(intern_time));

    REQUIRE(interned_list.size == expected_intern_count);

    std::sort(
        interned_list.begin(),
        interned_list.end(),
        [](const intern::result_t& lhs, const intern::result_t& rhs) {
            return lhs.id < rhs.id;
        });

//    format::print("\n");
//    for (const auto& r : interned_list) {
//        format::print("id = {:0>6}; hash = {}; slice = {}\n", r.id, r.hash, r.slice);
//    }

    u32 last_id{};
    for (const auto& r : interned_list) {
        REQUIRE(last_id <= r.id);
        auto existing_slice = intern::pool::intern(pool, r.slice);
        REQUIRE(existing_slice.slice == r.slice);
        last_id = r.id;
    }

    for (const auto& r : interned_list) {
        auto get_result = intern::pool::get(pool, r.id);
        REQUIRE(get_result.status == intern::status_t::ok);
        REQUIRE(get_result.id == r.id);
        REQUIRE(get_result.hash == r.hash);
        REQUIRE(get_result.slice == r.slice);
    }
}
