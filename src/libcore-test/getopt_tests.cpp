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

static u0 usage(getopt_t& opt) {
    str_t buf{};
    str::init(buf);
    defer(str::free(buf));
    getopt::format_help(opt, buf);
    format::print("{}\n", buf);
}

TEST_CASE("basecode::getopt integer arguments", "[getopt]") {
    const char* argv[] = {
        "C:\\temp\\add-two.exe",
        "--addend=10",
        "-e",
        "20"
    };

    stopwatch_t timer{};
    stopwatch::start(timer);

    getopt_t cl{};
    getopt::init(cl, 4, argv);
    defer(getopt::free(cl));
    getopt::program_description(
        cl,
        "The 'add-two' program takes in two arguments and adds them."_ss);

    getopt::make_option(cl)
        .min_required(1)
        .type(arg_type_t::integer)
        .short_name('a')
        .long_name("addend"_ss)
        .description("the addend"_ss)
        .build();

    getopt::make_option(cl)
        .min_required(1)
        .type(arg_type_t::integer)
        .short_name('e')
        .long_name("augend"_ss)
        .description("the augend"_ss)
        .build();

    getopt::make_option(cl)
        .short_name('?')
        .long_name("help"_ss)
        .description("give this help list"_ss)
        .build();

    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt setup"_ss, 40, timer);

    stopwatch::start(timer);
    auto status = getopt::parse(cl);
    REQUIRE(OK(status));
    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt parse"_ss, 40, timer);
//    getopt::format(cl);
}

TEST_CASE("basecode::getopt immediate argument value", "[getopt]") {
    const char* argv[] = {
        "C:\\temp\\tar.exe",
        "-xvz",
        "-f=test.tar.gz",
        "--",
        "-D",
        "--foo-mode",
        "-s=Welcome"
    };

    stopwatch_t timer{};
    stopwatch::start(timer);

    getopt_t cl{};
    getopt::init(cl, 7, argv);
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

    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt setup"_ss, 40, timer);

    stopwatch::start(timer);
    auto status = getopt::parse(cl);
    REQUIRE(OK(status));
    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt parse"_ss, 40, timer);
//    getopt::format(cl);
}

TEST_CASE("basecode::getopt deferred argument value", "[getopt]") {
    const char* argv[] = {
        "C:\\temp\\tar.exe",
        "-xvfz",
        "test.tar.gz",
        "--",
        "-D",
        "--foo-mode",
        "-s=Welcome"
    };

    stopwatch_t timer{};
    stopwatch::start(timer);

    getopt_t cl{};
    getopt::init(cl, 7, argv);
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

    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt setup"_ss, 40, timer);

    usage(cl);

    stopwatch::start(timer);
    auto status = getopt::parse(cl);
    REQUIRE(OK(status));
    stopwatch::stop(timer);
    stopwatch::print_elapsed("getopt parse"_ss, 40, timer);
//    getopt::format(cl);
}
