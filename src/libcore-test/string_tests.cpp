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
#include <basecode/core/string.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::string scope tests") {
    auto start_scope_id = string::interned::scope_id();

    string::interned::fold("one");
    string::interned::fold("two");
    string::interned::fold("three");
    string::interned::fold("four");
    string::interned::fold("five");
    string::interned::fold("six");
    string::interned::fold("seven");

    TIME_BLOCK(
        "string: total intern/scope time"_ss,
        auto scope_id = string::interned::scope_id();
        REQUIRE(scope_id == start_scope_id + 7);

        intern::result_t r{};
        r = string::interned::fold_for_result("junk1");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk2");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk3");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk4");
        REQUIRE(r.new_value);

        r = string::interned::fold_for_result("junk1");
        REQUIRE(!r.new_value);
        r = string::interned::fold_for_result("junk2");
        REQUIRE(!r.new_value);
        r = string::interned::fold_for_result("junk3");
        REQUIRE(!r.new_value);
        r = string::interned::fold_for_result("junk4");
        REQUIRE(!r.new_value);

        string::interned::scope_id(scope_id);
        scope_id = string::interned::scope_id();

        r = string::interned::fold_for_result("junk1");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk2");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk3");
        REQUIRE(r.new_value);
        r = string::interned::fold_for_result("junk4");
        REQUIRE(r.new_value);

        string::interned::scope_id(scope_id);
        REQUIRE(start_scope_id + 7 == scope_id);
    );
}
