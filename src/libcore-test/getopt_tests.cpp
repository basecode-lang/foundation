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
#include <basecode/core/getopt.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::getopt basics") {
    const char* argv[] = {
        "C:\\temp\\foo.exe",
        "-xvfz",
        "test.tar.gz"
    };

    stopwatch_t timer{};
    stopwatch::start(timer);

    getopt_t cl{};
    getopt::init(cl, 3, argv);
    defer(getopt::free(cl));

    str_t buf{};
    str::init(buf);
    defer(str::free(buf));

    getopt::format_help(cl, buf);
    format::print("{}\n", buf);

    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt setup & parse"_ss, 40, timer);
}
