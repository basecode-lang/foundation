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
#include <basecode/core/utf.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::utf basics") {
    utf8_str_t str8{};
    utf::init(str8);
    defer(utf::free(str8));

    utf::append(str8, u"this is a utf-16 string; \u263a");
    utf::append(str8, U"this is a utf-32 string. \u263a");

    REQUIRE(utf::length(str8) == 52);
}
