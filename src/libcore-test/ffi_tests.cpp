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
#include <basecode/core/ffi.h>
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

extern "C" u32 simple(u32 a, u32 b) {
    return a * b;
}

TEST_CASE("basecode::ffi basics") {
    path_t proc_path{};
    path::init(proc_path, slice::make(context::top()->argv[0]));

    lib_t* proc_lib{};
    auto status = ffi::lib::load(proc_path, &proc_lib);
    REQUIRE(OK(status));
    defer(ffi::lib::unload(proc_lib));

    proto_t* simple_proto{};
    status = ffi::proto::make(proc_lib, "simple"_ss, &simple_proto);
    REQUIRE(OK(status));

    auto u32_type = ffi::param::make_type(param_cls_t::int_, param_size_t::dword);
    simple_proto->ret_type = u32_type;
    ffi::proto::append(simple_proto, ffi::param::make("a"_ss, u32_type));
    ffi::proto::append(simple_proto, ffi::param::make("b"_ss, u32_type));

    ffi_t vm{};
    ffi::init(vm);
    defer(ffi::free(vm));

    stopwatch_t time{};
    stopwatch::start(time);

    ffi::reset(vm);
//    ffi::push(vm, ffi::arg(5));
//    ffi::push(vm, ffi::arg(6));
    ffi::push(vm, 5);
    ffi::push(vm, 6);
    param_alias_t ret{};
    status = ffi::call(vm, simple_proto, ret);
    REQUIRE(OK(status));
    REQUIRE(ret.dw == 30);

    stopwatch::stop(time);
    stopwatch::print_elapsed("ffi call simple function"_ss, 40, stopwatch::elapsed(time));
}
