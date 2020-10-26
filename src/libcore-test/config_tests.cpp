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
#include <basecode/core/config.h>
#include <basecode/core/format.h>
#include <basecode/core/string.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

static u0 validate_cvar(fe_Context* ctx, const s8* name, u32 id, b8 expected) {
    auto binding = fe_get(ctx, fe_symbol(ctx, name), fe_nil());
    auto cvar_value = fe_cdr(ctx, binding);
    if (expected) {
        REQUIRE(fe_type(ctx, cvar_value) == FE_TNUMBER);
        auto native_value = u32(fe_tonumber(ctx, cvar_value));
        REQUIRE(native_value == id);
    } else {
        REQUIRE(cvar_value == fe_nil());
    }

    auto source = format::format("(if (is {} {}) #t #f)", id, name);
    fe_Object* obj{};
    config::eval(source, &obj);
    REQUIRE(obj);

    str_t buf{};
    str::init(buf);
    str::reserve(buf, 64);
    buf.length = fe_tostring(ctx, obj, (s8*) buf.data, buf.capacity);
    if (expected)
        REQUIRE(buf == "#t");
    else
        REQUIRE(buf == "nil");
}

TEST_CASE("basecode::config cvar add & remove") {
    const auto cvar_id                       = 20;
    const auto enable_console_color          = "enable-console-color"_ss;
    const auto internal_enable_console_color = "cvar:enable-console-color";

    REQUIRE(OK(config::cvar::add(cvar_id, enable_console_color, cvar_type_t::flag)));
    fe_Context* ctx = config::system::context();
    validate_cvar(ctx, internal_enable_console_color, cvar_id, true);

    cvar_t* cvar{};
    REQUIRE(OK(config::cvar::get(cvar_id, &cvar)));
    REQUIRE(cvar != nullptr);

    REQUIRE(OK(config::cvar::remove(cvar_id)));
    validate_cvar(ctx, internal_enable_console_color, cvar_id, false);

    REQUIRE(config::cvar::get(cvar_id, &cvar) == config::status_t::cvar_not_found);
    REQUIRE(!cvar);
}

TEST_CASE("basecode::config find localized strings") {
    str::slice_t* value{};
    REQUIRE(OK(string::localized::find(0, &value)));
    REQUIRE(*value == "ok"_ss);

    REQUIRE(OK(string::localized::find(5000, &value)));
    REQUIRE(*value == "US: test localized string: 0={} 1={} 2={}"_ss);

    REQUIRE(OK(string::localized::find(5001, &value)));
    REQUIRE(*value == "duplicate cvar"_ss);

    REQUIRE(OK(string::localized::find(5002, &value)));
    REQUIRE(*value == "cvar not found"_ss);

    REQUIRE(OK(string::localized::find(5003, &value)));
    REQUIRE(*value == "invalid modification of constant: {}"_ss);

    REQUIRE(OK(string::localized::find(5004, &value, "en_GB"_ss)));
    REQUIRE(*value == "GB: test localized string: 0={} 1={} 2={}"_ss);

    REQUIRE(string::localized::find(0, &value, "es_MX"_ss) == string::status_t::localized_not_found);
}

TEST_CASE("basecode::config terp eval") {
    stopwatch_t time{};
    stopwatch::start(time);

    const auto source = R"(
(do
    (let result 50)
    result)
)"_ss;

    fe_Object* obj{};
    fe_Context* ctx = config::system::context();

    config::eval(source, &obj);
    REQUIRE(obj);
    REQUIRE(fe_type(ctx, obj) == FE_TNUMBER);
    auto value = fe_tonumber(ctx, obj);
    REQUIRE(value == 50);

    stopwatch::stop(time);
    stopwatch::print_elapsed("fe test program"_ss, 40, time);
}
