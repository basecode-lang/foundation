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
#include <basecode/core/log.h>
#include <basecode/core/ffi.h>
#include <basecode/core/job.h>
#include <basecode/core/event.h>
#include <basecode/core/defer.h>
#include <basecode/core/config.h>
#include <basecode/core/thread.h>
#include <basecode/core/memory.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>

using namespace basecode;

s32 main(s32 argc, const s8** argv) {
    {
        auto status = memory::system::init(alloc_type_t::dlmalloc);
        if (!OK(status)) {
            format::print(stderr, "memory::system::init error: {}\n", memory::status_name(status));
            return (s32) status;
        }
    }
    {
        default_config_t dft_config{};
        dft_config.file = stderr;
        dft_config.process_arg = argv[0];
        auto status = log::system::init(logger_type_t::default_, &dft_config, log_level_t::debug, memory::system::default_alloc());
        if (!OK(status)) {
            format::print(stderr, "log::system::init error: {}\n", log::status_name(status));
        }
    }

    auto ctx = context::make(argc, argv, memory::system::default_alloc(), log::system::default_logger());
    context::push(&ctx);

    if (!OK(buf_pool::system::init()))  return 1;
    if (!OK(string::system::init()))    return 1;
    if (!OK(config::system::init()))    return 1;

    auto core_config_path = "../etc/core.fe"_path;
    path_t config_path{};
    path::init(config_path, slice::make(context::top()->argv[0]));
    path::parent_path(config_path, config_path);
    path::append(config_path, core_config_path);
    if (!config_path.is_abs)
        filesys::mkabs(config_path, config_path);
    fe_Object* result{};
    if (!OK(config::eval(config_path, &result))) return 1;

    if (!OK(profiler::init()))          return 1;
    if (!OK(memory::proxy::init()))     return 1;
    if (!OK(event::system::init()))     return 1;
    if (!OK(thread::system::init()))    return 1;
    if (!OK(job::system::init()))       return 1;
    if (!OK(ffi::system::init()))       return 1;
    if (!OK(filesys::init()))           return 1;
    if (!OK(network::system::init()))   return 1;

    auto rc = Catch::Session().run(argc, argv);

    log::notice("shutdown test program");

    path::free(config_path);
    path::free(core_config_path);

    network::system::fini();
    filesys::fini();
    ffi::system::fini();
    profiler::fini();
    job::system::fini();
    event::system::fini();
    thread::system::fini();
    config::system::fini();
    log::system::fini();
    string::system::fini();
    buf_pool::system::fini();
    memory::proxy::fini();
    memory::system::fini();
    context::pop();

    return rc;
}
