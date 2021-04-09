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

using namespace basecode;

TEST_CASE("basecode::term basics", "[term]") {
    term_t term{};
    term::init(term);
    defer(term::free(term));

    str_t str{};
    str::init(str);

    {
        str_buf_t buf(&str);

        term::set_style(term, term::style::bold);
        term::refresh(term, buf);
        format::format_to(buf, "test bold\n");

        term::set_style(term, term::style::dim);
        term::refresh(term, buf);
        format::format_to(buf, "test dim\n");

        term::set_style(term, term::style::italic);
        term::refresh(term, buf);
        format::format_to(buf, "test italic\n");

        term::set_style(term, term::style::underline);
        term::refresh(term, buf);
        format::format_to(buf, "test underline\n");

        term::set_style(term, term::style::slow_blink);
        term::refresh(term, buf);
        format::format_to(buf, "test slow blinken\n");

        term::set_style(term, term::style::fast_blink);
        term::refresh(term, buf);
        format::format_to(buf, "test fast blinken\n");

        term::set_style(term, term::style::reverse);
        term::refresh(term, buf);
        format::format_to(buf, "test reverse video");

        term::set_style(term, term::style::hidden);
        term::refresh(term, buf);
        format::format_to(buf, "\nha! ha! you can't see this!\n");

        term::set_style(term, term::style::strike);
        term::refresh(term, buf);
        format::format_to(buf, "test strike-thru\n");

        term::set_style(term, term::style::double_underline);
        term::refresh(term, buf);
        format::format_to(buf, "test double underline\n");

        term::reset_all(term);
        term::refresh(term, buf);
    }

    format::print("{}", str);
}
