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
#include <basecode/core/profiler.h>
#include <basecode/core/memory/memory.h>
#include <basecode/core/memory/proxy_system.h>

using namespace basecode;

s32 main(s32 argc, const s8** argv) {
    memory::system::initialize();

    auto ctx = context::make(memory::system::default_alloc());
    context::push(&ctx);

    if (!OK(profiler::initialize()))        return 1;
    if (!OK(memory::proxy::initialize()))   return 1;

    defer({
        profiler::shutdown();
        memory::proxy::shutdown();
        memory::system::shutdown();
        context::pop();
    });

    return Catch::Session().run(argc, argv);
}
