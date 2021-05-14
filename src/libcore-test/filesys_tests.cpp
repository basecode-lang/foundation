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
#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/filesys.h>
#include <basecode/core/stopwatch.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::filesys basics") {
    auto xdg_env = env::system::get("xdg"_ss);
    if (xdg_env) {
        format_env(xdg_env);
    }
}

#ifdef _WIN32
TEST_CASE("basecode::filesys glob") {
    glob_result_t r{};
    filesys::glob::init(r);
    defer(filesys::glob::free(r));

    TIME_BLOCK("glob for c:\\temp\\*.png"_ss,
               REQUIRE(OK(filesys::glob::find(r, "c:\\\\temp\\\\*.png"_ss)));
               for (const auto& path : r.paths)
                   format::print("{}\n", path));
}
#endif
