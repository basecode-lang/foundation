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
#include <basecode/core/str.h>
#include <basecode/core/defer.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/hashtable.h>

using namespace basecode;

struct payload_t final {
    u0*     ptr;
    u32     offset;
};

TEST_CASE("basecode::hashtable payload with random string keys") {
    hashtable_t<string::slice_t, payload_t> table{};
    hashtable::init(table, context::top()->alloc, .65f);
    defer(hashtable::free(table));
    //hashtable::reserve(table, 1024);

    string_t str[4096];
    for (u32 i = 0; i < 4096; ++i) {
        string::init(str[i], context::top()->alloc);
        string::reserve(str[i], 32);
        str[i] = string::random(32);
    }

    stopwatch_t time{};
    stopwatch::start(time);

    for (u32 i = 0; i < 4096; ++i) {
        auto payload = hashtable::emplace(table, slice::make(str[i]));
        payload->ptr = {};
        payload->offset = i * 100;
    }

    stopwatch::stop(time);
    stopwatch::print_elapsed("hashtable payload + string key"_ss, 40, stopwatch::elapsed(time));

//    auto keys = hashtable::keys(table);
//    defer(array::free(keys));
//    for (auto key : keys) {
//        format::print("key = {}\n", *key);
//    }
}

TEST_CASE("basecode::hashtable payload with integer keys") {
    hashtable_t<u32, payload_t> table{};
    hashtable::init(table, context::top()->alloc, .65f);
    defer(hashtable::free(table));
    //hashtable::reserve(table, 1024);

    stopwatch_t time{};
    stopwatch::start(time);

    for (u32 i = 0; i < 4096; ++i) {
        auto payload = hashtable::emplace(table, i * 4096);
        payload->ptr = {};
        payload->offset = i * 100;
    }

    stopwatch::stop(time);
    stopwatch::print_elapsed("hashtable payload + int key"_ss, 40, stopwatch::elapsed(time));

//    auto keys = hashtable::keys(table);
//    defer(array::free(keys));
//    for (auto key : keys) {
//        format::print("key = {}\n", *key);
//    }
}

TEST_CASE("basecode::hashtable basics") {
    stopwatch_t time{};
    stopwatch::start(time);

    hashtable_t<s32, string::slice_t> table{};
    hashtable::init(table);
    defer(hashtable::free(table));

    const auto one = "one"_ss;
    const auto two = "two"_ss;
    const auto three = "three"_ss;
    const auto four = "four"_ss;
    const auto five = "five"_ss;
    const auto six = "six"_ss;
    const auto seven = "seven"_ss;

    hashtable::insert(table, 1, one);
    hashtable::insert(table, 2, two);
    hashtable::insert(table, 3, three);
    hashtable::insert(table, 4, four);
    hashtable::insert(table, 5, five);
    hashtable::insert(table, 6, six);
    hashtable::insert(table, 7, seven);
    REQUIRE(table.size == 7);
    REQUIRE(table.capacity == 32);

    string::slice_t* s;

    s = hashtable::find(table, 5);
    REQUIRE(s);
    REQUIRE(*s == five);

    s = hashtable::find(table, 1);
    REQUIRE(s);
    REQUIRE(*s == one);

    s = hashtable::find(table, 7);
    REQUIRE(s);
    REQUIRE(*s == seven);

    s = hashtable::find(table, 3);
    REQUIRE(s);
    REQUIRE(*s == three);

    s = hashtable::find(table, 6);
    REQUIRE(s);
    REQUIRE(*s == six);

    s = hashtable::find(table, 2);
    REQUIRE(s);
    REQUIRE(*s == two);

    s = hashtable::find(table, 4);
    REQUIRE(s);
    REQUIRE(*s == four);

    stopwatch::stop(time);
    stopwatch::print_elapsed("hashtable insert + find"_ss, 40, stopwatch::elapsed(time));
}
