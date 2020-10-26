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
#include <basecode/core/string.h>
#include <basecode/core/format.h>
#include <basecode/core/filesys.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::ffi basics") {
#ifdef _MSC_VER
    auto lib_filename = "ffi-test-kernel.dll"_path;
#elif _WIN32
    auto lib_filename = "libffi-test-kernel.dll"_path;
#else
    auto lib_filename = "lib/libffi-test-kernel.so"_path;
#endif

    path_t proc_path{};
    path::init(proc_path, slice::make(context::top()->argv[0]));
    if (!path::absolute(proc_path))
        filesys::mkabs(proc_path, proc_path);

    path::parent_path(proc_path, proc_path);
#ifndef _WIN32
    path::parent_path(proc_path, proc_path);
#endif
    path::append(proc_path, lib_filename);
    format::print_ellipsis("ffi test library path", 40, "{}\n", proc_path.str);

    lib_t* proc_lib{};
    auto status = ffi::lib::load(proc_path, &proc_lib);
    defer({
        ffi::lib::unload(proc_lib);
        path::free(proc_path);
        path::free(lib_filename);
    });
    REQUIRE(OK(status));

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
    ffi::push(vm, 5);
    ffi::push(vm, 6);
    param_alias_t ret{};
    status = ffi::call(vm, simple_proto, ret);
    REQUIRE(OK(status));
    REQUIRE(ret.dw == 30);

    stopwatch::stop(time);
    stopwatch::print_elapsed("ffi call simple function"_ss, 40, time);
}

TEST_CASE("basecode::ffi status names") {
    REQUIRE(string::localized::status_name(ffi::status_t::ok) == "ok"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::address_null) == "ffi: address null"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::prototype_null) == "ffi: prototype null"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::lib_not_loaded) == "ffi: lib not loaded"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::symbol_not_found) == "ffi: symbol not found"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::invalid_int_size) == "ffi: invalid int size"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::invalid_float_size) == "ffi: invalid float size"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::load_library_failure) == "ffi: load library failure"_ss);
    REQUIRE(string::localized::status_name(ffi::status_t::struct_by_value_not_implemented) == "ffi: struct by value not implemented"_ss);
}
