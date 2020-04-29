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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/core/memory/proxy.h>
#include <basecode/core/memory/system.h>
#include <basecode/core/profiler/system.h>

using namespace basecode;

int main(int argc, const char** argv) {
    memory::initialize();

    auto ctx = context::make(memory::default_allocator());
    context::push(&ctx);

    if (!OK(profiler::initialize()))        return 1;
    if (!OK(memory::proxy::initialize()))   return 1;

    defer({
        profiler::shutdown();
        memory::proxy::shutdown();
        memory::shutdown();
        context::pop();
    });

    return Catch::Session().run(argc, argv);
}