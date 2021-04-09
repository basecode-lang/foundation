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
#include <basecode/core/term.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::term basics", "[term]") {
    str_t str{};
    str::init(str);

    {
        str_buf_t buf(&str);

        term::set_style(buf, term::style::bold);
        format::format_to(buf, "test bold\n");

        term::set_style(buf, term::style::dim);
        format::format_to(buf, "test dim\n");

        term::set_style(buf, term::style::italic);
        format::format_to(buf, "test italic\n");

        term::set_style(buf, term::style::underline);
        format::format_to(buf, "test underline\n");

        term::set_style(buf, term::style::slow_blink);
        format::format_to(buf, "test slow blinken\n");

        term::set_style(buf, term::style::fast_blink);
        format::format_to(buf, "test fast blinken\n");

        term::set_style(buf, term::style::reverse);
        format::format_to(buf, "test reverse video");

        term::set_style(buf, term::style::hidden);
        format::format_to(buf, "\nha! ha! you can't see this!\n");

        term::set_style(buf, term::style::strike);
        format::format_to(buf, "test strike-thru\n");

        term::set_style(buf, term::style::double_underline);
        format::format_to(buf, "test double underline\n");
    }

    format::print("{}", str);
}
