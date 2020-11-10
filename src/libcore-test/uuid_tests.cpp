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
#include <basecode/core/uuid.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

// Exemplar: {44332211-1234-ABCD-EFEF-001122334455}
TEST_CASE("basecode::uuid make") {
    stopwatch_t timer{};
    stopwatch::start(timer);

    auto u1 = uuid::make();

    stopwatch::stop(timer);
    stopwatch::print_elapsed("uuid::make time"_ss, 40, timer);

    auto str = format::format("{}", u1);
    REQUIRE(str.length == 38);
    REQUIRE(str[0] == '{');
    REQUIRE(str[9] == '-');
    REQUIRE(str[14] == '-');
    REQUIRE(str[19] == '-');
    REQUIRE(str[24] == '-');
    REQUIRE(str[37] == '}');

    for (u32 i = 0; i < str.length; ++i) {
        if (i == 0 || i == 9 || i == 14 || i == 19 || i == 24 || i == 37)
            continue;
        REQUIRE(isxdigit(str[i]));
    }
}

TEST_CASE("basecode::uuid parse") {
    const auto exemplar = "{44332211-1234-ABCD-EFEF-001122334455}"_ss;

    stopwatch_t timer{};
    stopwatch::start(timer);

    basecode::uuid_t u{};
    REQUIRE(OK(uuid::parse(exemplar, &u)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("uuid::parse time"_ss, 40, timer);

    auto str = format::format("{}", u);
    REQUIRE(str == exemplar);
}
