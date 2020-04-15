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
#include <basecode/core/string/ascii_string.h>
#include <basecode/core/hashtable/hashtable.h>

using namespace basecode;
using namespace basecode::string;

TEST_CASE("basecode::hashtable basics") {
    hashtable::table_t<s32, string::slice_t> table;
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
    REQUIRE(table.capacity == 16);

    string::slice_t* s{};

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
}
