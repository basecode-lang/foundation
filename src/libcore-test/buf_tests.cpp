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
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::buf basics") {
    auto path = "../etc/instructions.ig"_path;
    auto buf = buf::make();
    REQUIRE(OK(buf::load(buf, path)));
    defer({
              buf::free(buf);
              path::free(path);
          });

    TIME_BLOCK("index buf"_ss, buf::index(buf););
//    u32 lineno{};
//    for (const auto& line : buf.lines) {
//        format::print("{:04}: {}\n", ++lineno, slice::make(buf.data + line.pos, line.len));
//    }
    format::print("indexed lines: {}\n", buf.lines.size);
}

TEST_CASE("basecode::buf extended indexing") {
    str_t src{};
    str::init(src);
    str::append(src, "foo(x=22, y='some string') {\r\n}\r\n");

    auto buf = buf::make();
    REQUIRE(OK(buf::load(buf, src)));
    defer({
              buf::free(buf);
              str::free(src);
          });

    TIME_BLOCK("index buf"_ss, buf::index(buf););

//    u32 lineno{};
//    for (const auto& line : buf.lines) {
//        format::print("{:04}: {}\n", ++lineno, slice::make(buf.data + line.pos, line.len));
//    }
    format::print("indexed lines: {}\n", buf.lines.size);
}
