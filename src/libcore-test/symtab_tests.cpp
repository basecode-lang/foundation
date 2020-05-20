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
#include <basecode/core/defer.h>
#include <basecode/core/slice.h>
#include <basecode/core/format.h>
#include <basecode/core/symtab.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/slice_utils.h>
#include "test.h"

using namespace basecode;

template <typename V>
[[maybe_unused]] static u0 format_nodes(symtab_t<V>& symtab) {
    u32 n = 1;
    format::print("symtab: size = {}, nodes.size = {}, values.size = {}\n",
                  symtab.size,
                  symtab.nodes.size,
                  symtab.values.size);
    for (const auto& node : symtab.nodes) {
        s8 c = (s8) node.sym;
        format::print("{:>4}: sym: {} type: {} next: {:>4} child: {:>4} value: {:>4}\n", n++, isprint(c) ? c : '.', node.type, node.next, node.child, node.value);
    }
}

template <typename V, typename B=std::remove_pointer_t<V>>
[[maybe_unused]] static u0 format_pairs(symtab_t<V>& symtab, str::slice_t prefix = {}) {
    assoc_array_t<B*> pairs{};
    assoc_array::init(pairs);
    symtab::find_prefix(symtab, pairs, prefix);
    defer(assoc_array::free(pairs));
    for (u32 i = 0; i < pairs.size; ++i) {
        auto pair = pairs[i];
        format::print("{:<20}: {}\n", pair.key, *pair.value);
    }
}

TEST_CASE("basecode::symtab_t remove key") {
    symtab_t<s32> symbols{};
    symtab::init(symbols);
    defer(symtab::free(symbols));

    {
        stopwatch_t time{};
        stopwatch::start(time);

        symtab::insert(symbols, "jeff"_ss, 33);
        symtab::insert(symbols, "jeffrey"_ss, 54);
        symtab::insert(symbols, "jerry"_ss, 44);
        REQUIRE(symbols.size == 3);

        stopwatch::stop(time);
        stopwatch::print_elapsed("add key/values step 1"_ss, 40, stopwatch::elapsed(time));
        format_pairs(symbols);
    }

    {
        stopwatch_t time{};
        stopwatch::start(time);

        REQUIRE(symtab::remove(symbols, "jerry"_ss));
        REQUIRE(symtab::remove(symbols, "jeffrey"_ss));
        REQUIRE(symtab::remove(symbols, "jeff"_ss));
        REQUIRE(symbols.size == 0);

        stopwatch::stop(time);
        stopwatch::print_elapsed("remove key/values from step 1"_ss, 40, stopwatch::elapsed(time));
        format_pairs(symbols);
    }

    {
        stopwatch_t time{};
        stopwatch::start(time);

        symtab::insert(symbols, "john"_ss, 10);
        symtab::insert(symbols, "johnna"_ss, 20);
        symtab::insert(symbols, "joshua"_ss, 30);
        symtab::insert(symbols, "jacob"_ss, 40);
        REQUIRE(symbols.size == 4);

        stopwatch::stop(time);
        stopwatch::print_elapsed("add key/values step 2"_ss, 40, stopwatch::elapsed(time));
        format_pairs(symbols);
    }
}

TEST_CASE("basecode::symtab_t names") {
    auto path = "../etc/ut.txt"_path;
    auto buf = buf::make();
    REQUIRE(OK(buf::load(buf, path)));

    symtab_t<baby_name_t> symbols{};
    symtab::init(symbols);

    array_t<name_record_t> records{};
    array::init(records);
    defer({
              path::free(path);
              for (auto& record : records)
                  array::free(record.fields);
              array::free(records);
              buf::free(buf);
              symtab::free(symbols);
          });

    buf::each_line(
        buf,
        [&records](const str::slice_t& line) {
            auto& record = array::append(records);
            array::init(record.fields);
            slice::to_fields(line, record.fields);
            return true;
        });

    stopwatch_t emplace_time{};
    stopwatch::start(emplace_time);

    for (const auto& rec : records) {
        const auto& key   = rec.fields[3];
        baby_name_t* name{};
        if (symtab::emplace(symbols, key, &name)) {
            const auto& state = rec.fields[0];
            const auto& year  = rec.fields[2];
            name->sex = rec.fields[1][0];
            name->year[0]  = year.data[0];
            name->year[1]  = year.data[1];
            name->year[2]  = year.data[2];
            name->year[3]  = year.data[3];
            name->year[4]  = '\0';
            name->state[0] = state.data[0];
            name->state[1] = state.data[1];
            name->state[2] = '\0';
        }
    }

    stopwatch::stop(emplace_time);

    format::print("table.size = {}, table.nodes.size = {}\n", symbols.size, symbols.nodes.size);
    stopwatch::print_elapsed("total symtab emplace time"_ss, 40, stopwatch::elapsed(emplace_time));
}
