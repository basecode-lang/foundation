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
#include <foundation/defer.h>
#include <foundation/array/array.h>
#include <foundation/string/intern.h>
#include <foundation/format/system.h>
#include <foundation/string/ascii_string_formatters.h>

using namespace basecode;
using namespace basecode::string;

TEST_CASE("basecode::intern") {
    constexpr auto expected_intern_count = 200;

    const auto allocated_before = context::current()->allocator->total_allocated;

    intern::pool_t pool(context::current()->allocator);
    intern::init(pool);
    defer(intern::free(pool));

    array::array_t<string::slice_t> interned_list;
    array::init(interned_list);
    array::reserve(interned_list, expected_intern_count, false);
    defer(array::free(interned_list));

    defer({
        const auto memory_used = context::current()->allocator->total_allocated - allocated_before;
        format::print("memory_used = {}\n", memory_used);
    });

    string::ascii_t str;
    for (u32 i = 0; i < expected_intern_count; ++i) {
        string::reset(str);
        string::append(str, "aaaaaAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBCCCCCCC");
        str = str + string::random(2);
        auto slice = intern::intern(pool, string::make_slice(str));
        array::append(interned_list, slice);
    }

    REQUIRE(interned_list.size == expected_intern_count);

    for (const auto& slice : interned_list) {
        auto existing_slice = intern::intern(pool, slice);
        REQUIRE(existing_slice.data == slice.data);
        REQUIRE(existing_slice.length == slice.length);
    }
}
