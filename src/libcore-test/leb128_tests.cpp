// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <catch2/catch.hpp>
#include <basecode/core/leb128.h>

using namespace basecode;

TEST_CASE("basecode::leb128 basics") {
    auto i32 = leb::encode(s32(336));
    s32  v1  = leb::decode<s32>(i32);
    REQUIRE(v1 == 336);

    auto i32_max = leb::encode(std::numeric_limits<u32>::max());
    u32  v2      = leb::decode<u32>(i32_max);
    REQUIRE(v2 == std::numeric_limits<u32>::max());

    auto i64_min = leb::encode(std::numeric_limits<s64>::min());
    s64  v3      = leb::decode<s64>(i64_min);
    REQUIRE(v3 == std::numeric_limits<s64>::min());

    auto i64_max = leb::encode(std::numeric_limits<u64>::max());
    u64  v4      = leb::decode<u64>(i64_max);
    REQUIRE(v4 == std::numeric_limits<u64>::max());
}
