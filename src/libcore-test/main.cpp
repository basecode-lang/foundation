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
#include <catch.hpp>
#include <basecode/core/log.h>
#include <basecode/core/ffi.h>
#include <basecode/core/job.h>
#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/locale.h>
#include <basecode/core/thread.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/core/configure.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/cxx.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>

using namespace basecode;

s32 main(s32 argc, const s8** argv) {
    s32 rc;

    // N.B. must init the profiler first so the stopwatch_t
    //      gives us meaningful results.
    //
    // !!! IMPORTANT !!!
    //
    // keep profiler dependency free!
    //
    if (!OK(profiler::init()))
        return 1;

    auto status = memory::system::init(alloc_type_t::dlmalloc);
    if (!OK(status)) {
        fmt::print(stderr,
                   "memory::system::init error: {}\n",
                   memory::status_name(status));
        return (s32) status;
    }
    auto ctx = context::make(argc,
                             argv,
                             memory::system::default_alloc());
    context::push(&ctx);

    rc = stopwatch::time_block("log::system::init"_ss, [&argv]() -> s32 {
        default_config_t dft_config{};
        dft_config.file        = stderr;
        dft_config.process_arg = argv[0];
        auto status = log::system::init(logger_type_t::default_,
                                        &dft_config,
                                        log_level_t::debug,
                                        memory::system::default_alloc());
        if (!OK(status)) {
            format::print(stderr,
                          "log::system::init error: {}\n",
                          log::status_name(status));
        }

        context::top()->logger = log::system::default_logger();

        return 0;
    });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("term::system::init"_ss,
                               []() -> s32 { return s32(term::system::init(true)); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("locale::system::init"_ss,
                               []() -> s32 { return s32(locale::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("buf_pool::system::init"_ss,
                               []() -> s32 { return s32(buf_pool::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("string::system::init"_ss,
                               []() -> s32 { return s32(string::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("error::system::init"_ss,
                               []() -> s32 { return s32(error::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("memory::system::init"_ss,
                               []() -> s32 { return s32(memory::proxy::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("event::system::init"_ss,
                               []() -> s32 { return s32(event::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("thread::system::init"_ss,
                               []() -> s32 { return s32(thread::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("job::system::init"_ss,
                               []() -> s32 { return s32(job::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("ffi::system::init"_ss,
                               []() -> s32 { return s32(ffi::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("filesys::init"_ss,
                               []() -> s32 { return s32(filesys::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("network::init"_ss,
                               []() -> s32 { return s32(network::system::init()); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("scm::system::init"_ss,
                               []() -> s32 { return s32(scm::system::init(256 * 1024)); });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("config::system::init"_ss,  []() -> s32 {
        config_settings_t settings{};
        settings.ctx           = scm::system::global_ctx();
        settings.product_name  = string::interned::fold(CORE_PRODUCT_NAME);
        settings.build_type    = string::interned::fold(CORE_BUILD_TYPE);
        settings.version.major = CORE_VERSION_MAJOR;
        settings.version.minor = CORE_VERSION_MINOR;
        settings.test_runner   = true;
        auto status = config::system::init(settings);
        if (!OK(status)) {
            format::print(stderr, "config::system::init error\n");
            return 1;
        }

        cvar_t* cvar{};
        config::cvar::add(1, "enable-console-color", cvar_type_t::flag);
        config::cvar::get(1, &cvar);
        cvar->value.flag = true;

        config::cvar::add(2, "log-path", cvar_type_t::string);
        config::cvar::get(2, &cvar);
        cvar->value.ptr = (u8*) string::interned::fold("/var/log"_ss).data;

        config::cvar::add(3, "magick-weight", cvar_type_t::real);
        config::cvar::get(3, &cvar);
        cvar->value.real = 47.314f;

        auto core_config_path = "../etc/core.scm"_path;
        path_t config_path{};
        filesys::bin_rel_path(config_path, core_config_path);
        scm::obj_t* result{};
        if (!OK(scm::system::eval(config_path, &result)))
            return 1;

        path::free(config_path);
        path::free(core_config_path);

        return 0;
    });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("scm::module::cxx::init"_ss, []() -> s32 {
        scm::module::cxx::system::init(scm::system::global_ctx());
        return 0;
    });
    if (!OK(rc)) return rc;

    rc = stopwatch::time_block("Catch::Session().run"_ss, [argc, &argv]() -> s32 {
        return Catch::Session().run(argc, argv);
    });

    log::notice("shutdown test program");

    stopwatch::time_block("network::system::fini"_ss,
                          []() -> s32 { network::system::fini(); return 0; });

    stopwatch::time_block("filesys::fini"_ss,
                          []() -> s32 { filesys::fini(); return 0; });

    stopwatch::time_block("ffi::system::fini"_ss,
                          []() -> s32 { ffi::system::fini(); return 0; });

    stopwatch::time_block("job::system::fini"_ss,
                          []() -> s32 { job::system::fini(); return 0; });

    stopwatch::time_block("thread::system::fini"_ss,
                          []() -> s32 { thread::system::fini(); return 0; });

    stopwatch::time_block("scm::module::cxx::system::fini"_ss,
                          []() -> s32 { scm::module::cxx::system::fini(); return 0; });

    stopwatch::time_block("config::system::fini"_ss,
                          []() -> s32 { config::system::fini(); return 0; });

    stopwatch::time_block("scm::system::fini"_ss,
                          []() -> s32 { scm::system::fini(); return 0; });

    stopwatch::time_block("string::system::fini"_ss,
                          []() -> s32 { string::system::fini(); return 0; });

    stopwatch::time_block("error::system::fini"_ss,
                          []() -> s32 { error::system::fini(); return 0; });

    stopwatch::time_block("buf_pool::system::fini"_ss,
                          []() -> s32 { buf_pool::system::fini(); return 0; });

    stopwatch::time_block("locale::system::fini"_ss,
                          []() -> s32 { locale::system::fini(); return 0; });

    stopwatch::time_block("log::system::fini"_ss,
                          []() -> s32 { log::system::fini(); return 0; });

    stopwatch::time_block("term::system::fini"_ss,
                          []() -> s32 { term::system::fini(); return 0; });

    stopwatch::time_block("event::system::fini"_ss,
                          []() -> s32 { event::system::fini(); return 0; });

    stopwatch::time_block("memory::proxy::fini"_ss,
                          []() -> s32 { memory::proxy::fini(); return 0; });

    stopwatch::time_block("memory::system::fini"_ss,
                          []() -> s32 { memory::system::fini(); return 0; });

    stopwatch::time_block("context::pop"_ss,
                          []() -> s32 { context::pop(); return 0; });

    stopwatch::time_block("profiler::fini"_ss,
                          []() -> s32 { profiler::fini(); return 0; });

    return rc;
}
