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
#include <basecode/core/buf.h>
#include <basecode/core/format.h>
#include <basecode/core/symtab.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/slice_utils.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::symtab_t remove key") {
    symtab_t<s32> symbols{};
    symtab::init(symbols);
    defer(symtab::free(symbols));

    TIME_BLOCK(
        "symtab: add key/values step 1"_ss,
        symtab::insert(symbols, "jeff"_ss, 33);
        symtab::insert(symbols, "jeffrey"_ss, 54);
        symtab::insert(symbols, "jerry"_ss, 44);
        REQUIRE(symbols.size == 3););

    symtab::format_pairs(symbols);

    TIME_BLOCK(
        "symtab: remove key/values from step 1"_ss,
        REQUIRE(symtab::remove(symbols, "jerry"_ss));
        REQUIRE(symtab::remove(symbols, "jeffrey"_ss));
        REQUIRE(symtab::remove(symbols, "jeff"_ss));
        REQUIRE(symbols.size == 0););

    symtab::format_pairs(symbols);

    TIME_BLOCK(
        "symtab: add key/values step 2"_ss,
        symtab::insert(symbols, "john"_ss, 10);
        symtab::insert(symbols, "johnna"_ss, 20);
        symtab::insert(symbols, "joshua"_ss, 30);
        symtab::insert(symbols, "jacob"_ss, 40);
        REQUIRE(symbols.size == 4););
    symtab::format_pairs(symbols);
}

TEST_CASE("basecode::symtab_t names") {
    auto path = "../etc/ut.txt"_path;
    auto buf = buf::make();
    REQUIRE(OK(buf::load(buf, path)));
    TIME_BLOCK(
        "symtab: buf index time"_ss,
        buf::index(buf));

    symtab_t<baby_name_t> symbols{};
    symtab::init(symbols);

    array_t<str::slice_t> fields{};
    array::init(fields);

    array_t<name_record_t> records{};
    array::init(records);
    defer({
              buf::free(buf);
              path::free(path);
              array::free(fields);
              array::free(records);
              symtab::free(symbols);
          });

    buf::each_line(
        buf,
        [&fields, &records](const str::slice_t& line) {
            auto& record = array::append(records);
            record.idx = fields.size;
            slice::to_fields(line, fields);
            record.len = fields.size - record.idx;
            return true;
        });

    stopwatch_t emplace_time{};
    stopwatch::start(emplace_time);

    TIME_BLOCK(
        "symtab: total emplace time"_ss,
        for (const auto& rec : records) {
            const auto& key = fields[rec.idx + 3];
            baby_name_t* name{};
            if (symtab::emplace(symbols, key, &name)) {
                const auto& state = fields[rec.idx + 0];
                const auto& year  = fields[rec.idx + 2];
                name->sex = fields[rec.idx + 1][0];
                name->year[0]  = year.data[0];
                name->year[1]  = year.data[1];
                name->year[2]  = year.data[2];
                name->year[3]  = year.data[3];
                name->year[4]  = '\0';
                name->state[0] = state.data[0];
                name->state[1] = state.data[1];
                name->state[2] = '\0';
            }
        });

    format::print("table.size = {}, table.nodes.size = {}\n",
                  symbols.size,
                  symbols.nodes.size);
}
