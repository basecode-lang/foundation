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
#include <basecode/core/str.h>
#include <basecode/core/defer.h>
#include <basecode/core/error.h>
#include <basecode/core/format.h>

using namespace basecode;

TEST_CASE("string::slice_t formatting") {
    format::print("{:<20}", "test with alignment\n"_ss);

    source::buffer_t buf{};
    source::buffer::init(buf);

    error::print(stderr, buf, "test: {}", 10);
}
