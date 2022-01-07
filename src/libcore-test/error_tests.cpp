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
#include <basecode/core/buf.h>
#include <basecode/core/error.h>
#include <basecode/core/string.h>

using namespace basecode;

TEST_CASE("basecode::error add & find") {
    const auto error_id = 6000;
    const auto en_us_lc = string::interned::fold("en_US"_ss);
    const auto en_gb_lc = string::interned::fold("en_GB"_ss);
    const auto test_code = string::interned::fold("B001"_ss);

    REQUIRE(OK(error::localized::add(error_id, 5000, en_us_lc, test_code)));
    REQUIRE(OK(error::localized::add(error_id, 5004, en_gb_lc, test_code)));

    error_def_t* def{};
    REQUIRE(OK(error::localized::find(error_id, &def)));
    REQUIRE(def->id == error_id);
    REQUIRE(def->lc_str_id == 5000);
    REQUIRE(def->code == test_code);
    REQUIRE(def->locale == en_us_lc);

    REQUIRE(OK(error::localized::find(error_id, &def, en_gb_lc)));
    REQUIRE(def->id == error_id);
    REQUIRE(def->lc_str_id == 5004);
    REQUIRE(def->code == test_code);
    REQUIRE(def->locale == en_gb_lc);

    error::report::add(error_id,
                       error_report_level_t::error,
                       "hello world!",
                       13,
                       "foo"_ss);
    error::report::add(error_id,
                       error_report_level_t::error,
                       "test",
                       42.111,
                       "bar"_ss);

    error::report::print_range(0, error::report::count());
}

TEST_CASE("basecode::error source formatted", "[source_formatted]") {
    const auto source = R"(core :: module("../modules/core");

#run {
    addr: ^u8 := alloc(size_of(u16) * 4096);
    'label:
    defer free(addr);

    core::print("%s\n", foo);
    core::print("addr := $%08x\n", addr);

    {
        defer core::print("a\n");
        defer core::print("b\n");
        defer core::print("c\n");
    };
};
)"_ss;

    buf_t buf{};
    buf::init(buf);
    defer(buf::free(buf));
    REQUIRE(OK(buf::load(buf, source)));
    buf::index(buf);

    {
        source_info_t src_info{};
        src_info.pos   = source_pos_t{150, 148};
        src_info.start = source_loc_t{7, 24};
        src_info.end   = source_loc_t{7, 26};
        error::report::add_src(5000,
                               error_report_level_t::error,
                               &buf,
                               src_info,
                               "foo");
    }
    auto start_id = error::report::count() - 1;

    {
        source_info_t src_info{};
        src_info.pos   = source_pos_t{303, 211};
        src_info.start = source_loc_t{11, 8};
        src_info.end   = source_loc_t{13, 33};
        error::report::add_src(5000,
                               error_report_level_t::error,
                               &buf,
                               src_info,
                               "defer");
    }

    str_t fmt_buf{};
    str::init(fmt_buf);
    error::report::format_range(fmt_buf, start_id, start_id + 2);
    format::print("{}", fmt_buf);
}
