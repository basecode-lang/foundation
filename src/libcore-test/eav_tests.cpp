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
#include <basecode/core/eav.h>
#include <basecode/core/defer.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::eav") {
    stopwatch_t timer{};
    stopwatch::start(timer);

    auto data_file = "test.data"_path;
    db_t db{};
    defer({
        eav::free(db);
        path::free(data_file);
    });
    REQUIRE(OK(eav::init(db, data_file)));

    {
        txn_t tx{};
        eav::status_t status;
        status = eav::txn::begin(db, tx);
        REQUIRE(OK(status));

        entity_t nil{};
        status = eav::symbol::find(tx, "nil"_ss, nil);
        if (status == eav::status_t::not_found) {
            status = eav::entity::make(tx, 0, nil);
            REQUIRE(OK(status));

            status  = eav::symbol::bind(tx, "nil"_ss, nil);
            REQUIRE(OK(status));
        }

        entity_t term{};
        status = eav::symbol::find(tx, "term"_ss, term);
        if (status == eav::status_t::not_found) {
            entity_t some_term{};
            entity_t type_attr{};

            status = eav::entity::make(tx, nil, term);
            REQUIRE(OK(status));

            status  = eav::symbol::bind(tx, "term"_ss, term);
            REQUIRE(OK(status));

            status = eav::entity::make(tx, nil, type_attr);
            REQUIRE(OK(status));

            status  = eav::symbol::bind(tx, "term.type"_ss, type_attr);
            REQUIRE(OK(status));

            status = eav::entity::make(tx, term, some_term);
            REQUIRE(OK(status));

            auto tuple = eav::tuple::number(type_attr, 32767);
            status = eav::tuple::set(tx, some_term, tuple);
            REQUIRE(OK(status));
            REQUIRE(tuple.rowid > 0);
            eav::tuple::free(tuple);
        }

        tuple_list_t tuples{};
        array::init(tuples);
        defer(array::free(tuples));
        status = eav::tuple::get(tx, 4, tuples);
        REQUIRE(OK(status));
        REQUIRE(tuples.size == 1);
        REQUIRE(tuples[0].attr == 3);
        REQUIRE(tuples[0].value.type == value_type_t::integer);
        REQUIRE(tuples[0].value == 32767);

        entity_t hello_world{};
        status = eav::entity::make(tx, nil, hello_world);
        REQUIRE(OK(status));

        auto msg_tuple = eav::tuple::string(4, "Hello World!"_ss);
        status = eav::tuple::set(tx, hello_world, msg_tuple);
        REQUIRE(OK(status));
        eav::tuple::free(msg_tuple);

        status = eav::txn::commit(db, tx);
        REQUIRE(OK(status));
    }

    stopwatch::stop(timer);
    stopwatch::print_elapsed("total eav time"_ss, 40, timer);
}

