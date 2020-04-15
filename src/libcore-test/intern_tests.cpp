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
#include <basecode/core/string/intern.h>
#include <basecode/core/format/system.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/string/formatters.h>
#include <basecode/core/profiler/stopwatch.h>

using namespace basecode;
using namespace basecode::string;

TEST_CASE("basecode::intern") {
    constexpr auto expected_intern_count = 200;

    const auto allocated_before = context::current()->allocator->total_allocated;

    intern::pool_t pool{};
    intern::pool::init(pool);
    defer(intern::pool::free(pool));

    array::array_t<intern::result_t> interned_list;
    array::init(interned_list);
    array::reserve(interned_list, expected_intern_count, false);
    defer(array::free(interned_list));

    defer({
        const auto memory_used = context::current()->allocator->total_allocated - allocated_before;
        format::print("memory_used = {}\n", memory_used);
    });

    profiler::stopwatch_t intern_time{};
    profiler::start(intern_time);

    string::ascii_t str;
    for (u32 i = 0; i < expected_intern_count; ++i) {
        string::reset(str);
        string::append(str, "aaaaaAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBCCCCCCC");
        str = str + string::random(2);
        auto r = intern::pool::intern(pool, string::make_slice(str));
        REQUIRE(r.status == intern::status_t::ok);
        REQUIRE(r.slice.data);
        REQUIRE(r.slice.length > 0);
        array::append(interned_list, r);
    }

    profiler::stop(intern_time);
    profiler::print_elapsed_time("total intern time"_ss, 40, profiler::elapsed(intern_time));

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

    id_t last_id{};
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
