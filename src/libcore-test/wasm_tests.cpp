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
#include <basecode/core/wasm.h>

using namespace basecode;

TEST_CASE("basecode::wasm basics") {
    wasm_t wasm{};
    wasm::init(wasm);
    defer(wasm::free(wasm));

    auto file_path = "C:/src/libs/emsdk/hello.wasm"_path;
    auto test_mod = wasm::load_module(wasm, file_path);
    REQUIRE(test_mod);
    REQUIRE(test_mod->path == "C:/src/libs/emsdk/hello.wasm"_ss);
    REQUIRE(OK(wasm::module::decode(*test_mod)));
    REQUIRE(test_mod->magic == 0x6d736100);
    REQUIRE(test_mod->version == 0x01);
    path::free(file_path);
}
