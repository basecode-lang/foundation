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
#include <basecode/core/buf.h>
#include <basecode/core/config.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::config basics") {
    stopwatch_t time{};
    stopwatch::start(time);

    const auto source = R"(
(do
    (let result 50)
    result)
)"_ss;

    fe_Object* obj{};
    fe_Context* ctx = config::system::context();

    config::eval(source, &obj);
    REQUIRE(obj);
    REQUIRE(fe_type(ctx, obj) == FE_TNUMBER);
    auto value = fe_tonumber(ctx, obj);
    REQUIRE(value == 50);

    stopwatch::stop(time);
    stopwatch::print_elapsed("fe test program"_ss, 40, time);
}
