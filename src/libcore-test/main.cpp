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
#include <basecode/core/defer.h>
#include <basecode/core/memory.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
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

    spdlog_color_config_t spdlog_config{};
    spdlog_config.color_type = spdlog_color_type_t::out;

    logger_t spdlog{};
    spdlog.name = "console"_ss;
    log::init(&spdlog, logger_type_t::spdlog, &spdlog_config);
    log::append_child(context::top()->logger, &spdlog);

    syslog_config_t syslog_config{};
    syslog_config.ident    = "bc-libcore-tests";
    syslog_config.opts     = opt_pid | opt_ndelay | opt_cons;
    syslog_config.facility = facility_local0;

    logger_t syslog{};
    log::init(&syslog, logger_type_t::syslog, &syslog_config);
    log::append_child(context::top()->logger, &syslog);

    log::info("example log message!");
    log::warn("oh, shit!");
    log::error("this should really stick out");

    if (!OK(profiler::init()))          return 1;
    if (!OK(memory::proxy::init()))     return 1;
    if (!OK(ffi::system::init()))       return 1;
    if (!OK(filesys::init()))           return 1;
    if (!OK(network::system::init()))   return 1;

    auto rc = Catch::Session().run(argc, argv);

    log::notice("shutdown test program");

    network::system::fini();
    filesys::fini();
    ffi::system::fini();
    profiler::fini();
    memory::proxy::fini();
    log::system::fini();
    memory::system::fini();
    context::pop();

    return rc;
}
