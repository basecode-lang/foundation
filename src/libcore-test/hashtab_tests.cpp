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
#include <basecode/core/hashtab.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/str_array.h>
#include <basecode/core/slice_utils.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::hashtab names", "[baby_names]") {
    auto path = "../etc/ut.txt"_path;
    auto buf = buf::make();
    REQUIRE(OK(buf::load(buf, path)));
    TIME_BLOCK(
        "hashtab: buf index time"_ss,
        buf::index(buf));

    hashtab_t<str::slice_t, baby_name_t> table{};
    hashtab::init(table);

    array_t<str::slice_t> fields{};
    array::init(fields);

    array_t<name_record_t> records{};
    array::init(records);

    defer({
              buf::free(buf);
              path::free(path);
              array::free(fields);
              array::free(records);
              hashtab::free(table);
          });

    buf::each_line(
        buf,
        [&fields, &records](str::slice_t line) {
            auto& record = array::append(records);
            record.idx = fields.size;
            slice::to_fields(line, fields);
            record.len = fields.size - record.idx;
            return true;
        });

    stopwatch_t emplace_time{};
    stopwatch::start(emplace_time);

    u32 count{};
    TIME_BLOCK(
        "hashtab: total emplace time"_ss,
        for (const auto& rec : records) {
            auto[name, is_new] = hashtab::emplace2(table, fields[rec.idx + 3]);
            if (is_new) {
                ++count;
                name->sex = fields[rec.idx + 1][0];
                std::memcpy(name->year, fields[rec.idx + 2].data, 4);
                name->year[4] = '\0';
                std::memcpy(name->state, fields[rec.idx + 0].data, 2);
                name->state[2] = '\0';
            }
        });

    if (count != table.size)
        REQUIRE(count == table.size);

    format::print("table.size = {}, table.capacity = {}\n", table.size, table.capacity);
}

TEST_CASE("basecode::hashtab payload with random string keys") {
    hashtab_t<str::slice_t, payload_t> table{};
    hashtab::init(table);

    str_array_t strings{};
    str_array::init(strings);
    str_array::reserve_index(strings, 4096);
    str_array::reserve_data(strings, 4096 * 33);

    str_t temp{};
    str::init(temp);
    str::reserve(temp, 32);
    defer({
              str::free(temp);
              str_array::free(strings);
              hashtab::free(table);
          });

    for (u32 i = 0; i < 4096; ++i) {
        str::random(temp, 32);
        str_array::append(strings, temp);
        str::reset(temp);
    }

    TIME_BLOCK("hashtab: payload + string key"_ss,
               for (u32 i = 0; i < 4096; ++i) {
                   auto payload = hashtab::emplace(table, strings[i]);
                   payload->ptr    = {};
                   payload->offset = i * 100;
               });
}

TEST_CASE("basecode::hashtab payload with integer keys") {
    hashtab_t<u32, payload_t> table{};
    hashtab::init(table);
    defer(hashtab::free(table));

    TIME_BLOCK(
        "hashtab: payload + int key"_ss,
        for (u32 i = 0; i < 4096; ++i) {
            auto payload = hashtab::emplace(table, i * 4096);
            payload->ptr    = {};
            payload->offset = i * 100;
        }

        for (u32 i = 0; i < 4096; ++i) {
            auto payload = hashtab::find(table, i * 4096);
            if (!payload) {
                REQUIRE(payload != nullptr);
            } else {
                if (payload->offset != i * 100)
                    REQUIRE(payload->offset == i * 100);
            }
        });
}

TEST_CASE("basecode::hashtab basics") {
    TIME_BLOCK(
        "hashtab: insert + find"_ss,
        hashtab_t<s32, str::slice_t> table{};
        hashtab::init(table);
        defer(hashtab::free(table));

        const auto one   = "one"_ss;
        const auto two   = "two"_ss;
        const auto three = "three"_ss;
        const auto four  = "four"_ss;
        const auto five  = "five"_ss;
        const auto six   = "six"_ss;
        const auto seven = "seven"_ss;

        hashtab::insert(table, 1, one);
        hashtab::insert(table, 2, two);
        hashtab::insert(table, 3, three);
        hashtab::insert(table, 4, four);
        hashtab::insert(table, 5, five);
        hashtab::insert(table, 6, six);
        hashtab::insert(table, 7, seven);
        REQUIRE(table.size == 7);

        str::slice_t* s;

        s = hashtab::find(table, 5);
        REQUIRE(s);
        REQUIRE(*s == five);

        s = hashtab::find(table, 1);
        REQUIRE(s);
        REQUIRE(*s == one);

        s = hashtab::find(table, 7);
        REQUIRE(s);
        REQUIRE(*s == seven);

        s = hashtab::find(table, 3);
        REQUIRE(s);
        REQUIRE(*s == three);

        s = hashtab::find(table, 6);
        REQUIRE(s);
        REQUIRE(*s == six);

        s = hashtab::find(table, 2);
        REQUIRE(s);
        REQUIRE(*s == two);

        s = hashtab::find(table, 4);
        REQUIRE(s);
        REQUIRE(*s == four););
}
