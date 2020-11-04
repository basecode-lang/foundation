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
#include <basecode/binfmt/io.h>
#include <basecode/binfmt/ar.h>
#include <basecode/core/defer.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::binfmt ar read test") {
    using namespace binfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

    auto lib_path = R"(C:\src\basecode-lang\foundation\build\msvc\debug\lib\basecode-binfmt.lib)"_path;

    ar_t ar{};
    ar::init(ar);
    defer({
        ar::free(ar);
        path::free(lib_path);
    });

    REQUIRE(OK(ar::read(ar, lib_path)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("binfmt ar read time"_ss, 40, timer);
}

