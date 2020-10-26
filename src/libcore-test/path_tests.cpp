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
#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

#ifdef _WIN32
TEST_CASE("basecode::path win32 parsing") {
    const auto expected_abs_path = R"(C:\temp\test.txt)"_ss;

    stopwatch_t time{};
    stopwatch::start(time);

    path_t path{};
    path::init(path, expected_abs_path);
    defer(path::free(path));

    REQUIRE(!path::empty(path));
    REQUIRE(path::length(path) == expected_abs_path.length);
    REQUIRE(path::absolute(path));
    REQUIRE(path::has_extension(path));

    auto ext = path::extension(path);
    REQUIRE(ext == ".txt"_ss);

    auto filename = path::filename(path);
    REQUIRE(filename == "test.txt"_ss);

    stopwatch::stop(time);
    stopwatch::print_elapsed("win32 path parsing"_ss, 40, time);
}
#endif

TEST_CASE("basecode::path basics") {
    const auto expected_path = "/Users/jeff/ut.txt"_ss;

    stopwatch_t time{};
    stopwatch::start(time);

    path_t path{};
    path::init(path, expected_path);
    defer(path::free(path));

    REQUIRE(!path::empty(path));
    REQUIRE(path::length(path) == expected_path.length);
    REQUIRE(path::absolute(path));
    REQUIRE(path::has_extension(path));

    auto ext = path::extension(path);
    REQUIRE(ext == ".txt"_ss);

    auto filename = path::filename(path);
    REQUIRE(filename == "ut.txt"_ss);

    stopwatch::stop(time);
    stopwatch::print_elapsed("path basic operations"_ss, 40, time);
}

TEST_CASE("basecode::path only a file name") {
    const auto expected_ext  = ".txt"_ss;
    const auto expected_stem = "ut"_ss;
    const auto expected_path = "ut.txt"_ss;

    stopwatch_t time{};
    stopwatch::start(time);

    path_t path{};
    path::init(path, expected_path);
    defer(path::free(path));

    REQUIRE(!path::empty(path));
    REQUIRE(path::length(path) == expected_path.length);
    REQUIRE(!path::absolute(path));
    REQUIRE(path::has_extension(path));
    REQUIRE(path::stem(path) == expected_stem);
    REQUIRE(path::extension(path) == expected_ext);
    REQUIRE(path::filename(path) == expected_path);

    stopwatch::stop(time);
    stopwatch::print_elapsed("path only a file name"_ss, 40, time);
}

TEST_CASE("basecode::path change extension") {
    const auto expected_ext     = ".txt"_ss;
    const auto expected_stem    = "ut"_ss;
    const auto expected_path    = "ut.txt"_ss;
    const auto new_expected_ext = ".text"_ss;

    stopwatch_t time{};
    stopwatch::start(time);

    path_t path{};
    path::init(path, expected_path);
    defer(path::free(path));

    REQUIRE(!path::empty(path));
    REQUIRE(path::length(path) == expected_path.length);
    REQUIRE(!path::absolute(path));
    REQUIRE(path::has_extension(path));
    REQUIRE(path::stem(path) == expected_stem);
    REQUIRE(path::extension(path) == expected_ext);
    REQUIRE(path::filename(path) == expected_path);

    REQUIRE(OK(path::replace_extension(path, new_expected_ext)));
    REQUIRE(path::extension(path) == new_expected_ext);

    stopwatch::stop(time);
    stopwatch::print_elapsed("path change extension"_ss, 40, time);
}


TEST_CASE("basecode::path append") {
    const auto expected_rel_path = "names/ut.txt"_ss;
#ifdef _WIN32
    const auto expected_combined_dir = R"(C:\Users\jeff\names)"_ss;
    const auto expected_base_path = R"(C:\Users\jeff)"_ss;
#else
    const auto expected_combined_dir = "/Users/jeff/names"_ss;
    const auto expected_base_path = "/Users/jeff"_ss;
#endif

    stopwatch_t time{};
    stopwatch::start(time);

    path_t rel_path{};
    path_t base_path{};
    path_t parent_path{};
    defer(
        path::free(base_path);
        path::free(rel_path);
        path::free(parent_path);
        );

    path::init(base_path, expected_base_path);
    REQUIRE(path::absolute(base_path));

    path::init(rel_path, expected_rel_path);
    REQUIRE(!path::absolute(rel_path));

    REQUIRE(OK(path::append(base_path, rel_path)));

    REQUIRE(path::directory(base_path) == expected_combined_dir);
    REQUIRE(path::length(base_path) == expected_base_path.length + expected_rel_path.length + 1);

    REQUIRE(path::extension(base_path) == ".txt"_ss);
    REQUIRE(path::filename(base_path) == "ut.txt"_ss);

    format::print("base_path = {}\n", base_path);

    path::init(parent_path, slice::make(base_path));

    while (OK(path::parent_path(parent_path, parent_path))) {
        format::print("parent_path = {}\n", parent_path);
    }

    stopwatch::stop(time);
    stopwatch::print_elapsed("path append operations"_ss, 40, time);
}
