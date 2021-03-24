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
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::wasm basics", "[wasm]") {
    wasm_t wasm{};
    wasm::init(wasm);
    defer(wasm::free(wasm));

    stopwatch_t timer{};
    stopwatch::start(timer);

    module_t* test_mod;
    {
        stopwatch_t load_timer{};
        stopwatch::start(load_timer);
        auto file_path = "C:/src/libs/emsdk/hello.wasm"_path;
        test_mod = wasm::load_module(wasm, file_path);
        if (!test_mod) {
            REQUIRE(test_mod);
        } else {
            if (test_mod->path != "C:/src/libs/emsdk/hello.wasm"_ss)
                REQUIRE(test_mod->path == "C:/src/libs/emsdk/hello.wasm"_ss);
        }
        path::free(file_path);
        stopwatch::stop(load_timer);
        stopwatch::print_elapsed("wasm load time"_ss, 40, load_timer);
    }
    {
        stopwatch_t decode_timer{};
        stopwatch::start(decode_timer);
        if (!OK(wasm::module::decode(*test_mod)))
            REQUIRE(false);
        if (test_mod->magic != 0x6d736100)
            REQUIRE(test_mod->magic == 0x6d736100);
        if (test_mod->version != 0x01)
            REQUIRE(test_mod->version == 0x01);
        stopwatch::stop(decode_timer);
        stopwatch::print_elapsed("wasm decode time"_ss, 40, decode_timer);
    }
    stopwatch::stop(timer);
    stopwatch::print_elapsed("wasm total time"_ss, 40, timer);
}
