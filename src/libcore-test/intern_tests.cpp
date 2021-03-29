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
#include <basecode/core/intern.h>
#include <basecode/core/format.h>
#include <basecode/core/str_array.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::intern simple") {
    intern_t pool{};
    intern::init(pool);
    defer(intern::free(pool));

    auto r1 = intern::fold(pool, "test1");
    REQUIRE(r1.status == intern::status_t::ok);
    REQUIRE(r1.slice.data);
    REQUIRE(r1.slice.length > 0);

    auto r2 = intern::fold(pool, "test2"_ss);
    REQUIRE(r2.status == intern::status_t::ok);
    REQUIRE(r2.slice.data);
    REQUIRE(r2.slice.length > 0);

    auto r3 = intern::fold(pool, "test3");
    REQUIRE(r3.status == intern::status_t::ok);
    REQUIRE(r3.slice.data);
    REQUIRE(r3.slice.length > 0);

    auto r4 = intern::fold(pool, "test1"_ss);
    REQUIRE(r4.id == r1.id);
    REQUIRE(r4.hash == r1.hash);
    REQUIRE(r4.slice == r1.slice);
    REQUIRE(r4.status == r1.status);
}

TEST_CASE("basecode::intern") {
    constexpr auto expected_intern_count = 200;

    intern_t pool{};
    intern::init(pool);
    defer(intern::free(pool));
    intern::reserve(pool, expected_intern_count);

    array_t<intern::result_t> interned_list{};
    array::init(interned_list);
    array::reserve(interned_list, expected_intern_count);
    defer(array::free(interned_list));

    str_array_t strings{};
    str_array::init(strings);
    str_array::reserve_index(strings, expected_intern_count);
    str_array::reserve_data(strings, expected_intern_count * 65);

    str_t temp{};
    str::init(temp);
    str::reserve(temp, 64);
    defer(str::free(temp); str_array::free(strings));

    for (u32 i = 0; i < expected_intern_count; ++i) {
        str::append(temp, "aaaaaAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBCCCCCCC");
        str::random(temp, 2);
        str_array::append(strings, temp);
        str::reset(temp);
    }

    const auto allocated_before = context::top()->alloc->total_allocated;

    TIME_BLOCK(
        "intern: total time"_ss,
        for (u32 i = 0; i < expected_intern_count; ++i) {
            auto r = intern::fold(pool, strings[i]);
            if (r.status != intern::status_t::ok)   REQUIRE(false);
            if (!r.slice.data)                      REQUIRE(false);
            if (r.slice.length == 0)                REQUIRE(false);
            array::append(interned_list, r);
        });

    const auto memory_used = context::top()->alloc->total_allocated - allocated_before;
    format::print("memory_used = {}\n", memory_used);

    REQUIRE(interned_list.size == expected_intern_count);

    std::sort(
        interned_list.begin(),
        interned_list.end(),
        [](const intern::result_t& lhs, const intern::result_t& rhs) {
            return lhs.id < rhs.id;
        });

    u32 last_id{};
    for (const auto& r : interned_list) {
        if (last_id > r.id) REQUIRE(false);
        auto existing_slice = intern::fold(pool, r.slice);
        if (existing_slice.slice != r.slice) REQUIRE(false);
        last_id = r.id;
    }

    for (const auto& r : interned_list) {
        auto get_result = intern::get(pool, r.id);
        if (get_result.status != intern::status_t::ok)  REQUIRE(false);
        if (get_result.id != r.id)                      REQUIRE(false);
        if (get_result.hash != r.hash)                  REQUIRE(false);
        if (get_result.slice != r.slice)                REQUIRE(false);
    }
}
