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

TEST_CASE("basecode::getopt basics", "[getopt]") {
    const char* argv[] = {
        "C:\\temp\\tar.exe",
        "-xvfz",
        "test.tar.gz"
    };

    stopwatch_t timer{};
    stopwatch::start(timer);

    getopt_t cl{};
    getopt::init(cl, 3, argv);
    defer(getopt::free(cl));
    getopt::program_description(
        cl,
        "GNU 'tar' saves many files together into a single\n"
        "tape or disk archive, and can restore"
        "individual files from the archive."_ss);

    getopt::make_option(cl)
        .type(arg_type_t::flag)
        .short_name('x')
        .description("extract files from an archive"_ss)
        .build();

    getopt::make_option(cl)
        .type(arg_type_t::flag)
        .short_name('v')
        .long_name("verbose"_ss)
        .description("verbosely list files processed"_ss)
        .build();

    getopt::make_option(cl)
        .min_required(1)
        .max_allowed(64)
        .type(arg_type_t::file)
        .short_name('f')
        .long_name("file"_ss)
        .value_name("ARCHIVE"_ss)
        .description("use archive file or device ARCHIVE"_ss)
        .build();

    getopt::make_option(cl)
        .type(arg_type_t::flag)
        .short_name('z')
        .long_name("uncompress"_ss)
        .description("filter the archive through compress"_ss)
        .build();

    getopt::make_option(cl)
        .short_name('?')
        .long_name("help"_ss)
        .description("give this help list"_ss)
        .build();

    str_t buf{};
    str::init(buf);
    defer(str::free(buf));

    getopt::format_help(cl, buf);
    format::print("{}\n", buf);

    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt setup & parse"_ss, 40, timer);
}
