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

#include <catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/gap_buf.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::gap_buf basics") {
    gap_buf_t buf{};
    gap_buf::init(buf);
    defer(gap_buf::free(buf));

    TIME_BLOCK(
        "gap_buf: edit test"_ss,
        gap_buf::caret_insert(
                buf,
                "this is a test of the gap buffer! i'm hoping it works out alright!"_ss);
            gap_buf::caret_move(buf, 10);
            gap_buf::gap_to_caret(buf);
            gap_buf::caret_put(buf, 's');
            gap_buf::caret_put(buf, 'p');
            gap_buf::caret_put(buf, 'e');
            gap_buf::caret_put(buf, 'c');
            gap_buf::caret_put(buf, 'i');
            gap_buf::caret_put(buf, 'a');
            gap_buf::caret_put(buf, 'l');
            gap_buf::caret_delete(buf, 5);
            gap_buf::caret_put(buf, ' ');
            gap_buf::caret_move(buf, 37);
            gap_buf::gap_to_caret(buf);
            gap_buf::caret_delete(buf, 4););

    gap_buf::print(buf);
    format::print("\n");
}
