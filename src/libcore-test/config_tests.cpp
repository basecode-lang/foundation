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

#include <catch.hpp>
#include <basecode/core/buf.h>
#include <basecode/core/string.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/config.h>

using namespace basecode;

static u0 validate_cvar(scm::ctx_t* ctx, const s8* name, u32 id, b8 expected) {
    auto binding = scm::get(ctx, scm::make_symbol(ctx, name));
    auto cvar_value = scm::cdr(ctx, binding);
    if (expected) {
        if (scm::type(cvar_value) != scm::obj_type_t::fixnum)
            REQUIRE(false);
        auto native_value = scm::to_fixnum(cvar_value);
        if (native_value != id)
            REQUIRE(false);
    } else {
        if (cvar_value != scm::nil(ctx))
            REQUIRE(false);
    }

    auto source = format::format("(if (is {} {}) #t #f)", id, name);
    scm::obj_t* obj{};
    scm::system::eval(source, &obj);
    if (!obj)
        REQUIRE(false);

    auto buf = scm::to_string(ctx, obj);
    if (expected) {
        if (buf != "#t"_ss) REQUIRE(false);
    } else {
        if (buf != "#f"_ss) REQUIRE(false);
    }
}

TEST_CASE("basecode::config cvar add & remove", "[!hide]") {
    const auto cvar_id                       = 20;
    const auto enable_console_color          = "enable-console-color"_ss;
    const auto internal_enable_console_color = "*enable-console-color*";

    if (!OK(config::cvar::add(cvar_id, enable_console_color, cvar_type_t::flag)))
        REQUIRE(false);
    scm::ctx_t* ctx = config::system::context();
    validate_cvar(ctx, internal_enable_console_color, cvar_id, true);

    cvar_t* cvar{};
    if (!OK(config::cvar::get(cvar_id, &cvar)))
        REQUIRE(false);
    if (cvar == nullptr)
        REQUIRE(false);

    if (!OK(config::cvar::remove(cvar_id)))
        REQUIRE(false);
    validate_cvar(ctx, internal_enable_console_color, cvar_id, false);

    if (config::cvar::get(cvar_id, &cvar) != config::status_t::cvar_not_found)
        REQUIRE(false);
    if (cvar)
        REQUIRE(false);
}

TEST_CASE("basecode::config find localized strings", "[!hide]") {
    str::slice_t* value{};

    if (!OK(string::localized::find(0, &value)))
        REQUIRE(false);
    if (*value != "ok"_ss)
        REQUIRE(false);

    if (!OK(string::localized::find(5000, &value)))
        REQUIRE(false);
    if (*value != "US: test localized string: 0={} 1={} 2={}"_ss)
        REQUIRE(false);

    if (!OK(string::localized::find(5001, &value)))
        REQUIRE(false);
    if (*value != "duplicate cvar"_ss)
        REQUIRE(false);

    if (!OK(string::localized::find(5002, &value)))
        REQUIRE(false);
    if (*value != "cvar not found"_ss)
        REQUIRE(false);

    if (!OK(string::localized::find(5003, &value)))
        REQUIRE(false);
    if (*value != "invalid modification of constant: {}"_ss)
        REQUIRE(false);

    if (!OK(string::localized::find(5004, &value, "en_GB"_ss)))
        REQUIRE(false);
    if (*value != "GB: test localized string: 0={} 1={} 2={}"_ss)
        REQUIRE(false);

    if (string::localized::find(0, &value, "es_MX"_ss) != string::status_t::localized_not_found)
        REQUIRE(false);
}

TEST_CASE("basecode::config terp eval") {
    TIME_BLOCK(
        "scm test program"_ss,
    const auto source = R"(
(begin
    (define result 50)
    result)
)"_ss;

    scm::obj_t* obj{};
    scm::system::eval(source, &obj);
    if (!obj)
        REQUIRE(false);
    if (scm::type(obj) != scm::obj_type_t::fixnum)
        REQUIRE(false);
    auto value = scm::to_fixnum(obj);
    if (value != 50)
        REQUIRE(false);
    );
}
