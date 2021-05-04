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
#include <basecode/core/format.h>
#include <basecode/core/variant.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

struct visitor_t {
    template <typename T>
    u0 operator()(T&& t) {
        format::print("{}\n", t);
    }
};

TEST_CASE("basecode::variant basics") {
    variant_array_t variants{};
    variant::init(variants);
    defer(variant::free(variants));

    variant::append(variants, s32(1));
    variant::append(variants, u8(2));
    variant::append(variants, f32(3.14));

    visitor_t visitor{};
    variant::visit(variants, visitor);
}
