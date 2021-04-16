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
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/core/configure.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/cxx.h>
#include <basecode/core/scm/modules/log.h>
#include <basecode/core/scm/modules/basic.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>

using namespace basecode;

struct test_suite_t final {
    Catch::Session          session;
    const s8**              argv;
    s32                     argc;
    s32                     extra_idx;
    u32                     repeat_count;
    u8                      no_scm:         1;
    u8                      pad:            7;
};

static u0 print_time(u64 e) {
    if (e == 0) {
        fprintf(stdout, "---\n");
    } else if (e < 1000) {
        fprintf(stdout, "%lld ns\n", e);
    } else {
        const auto us = e / 1000;
        if (us >= 1000) {
            fprintf(stdout, "%lld ms\n", us / 1000);
        } else {
            fprintf(stdout, "%lld us\n", us);
        }
    }
}

s32 run(test_suite_t& suite) {
    s32 rc;

    alloc_t* alloc; {
        auto status = memory::system::init(alloc_type_t::dlmalloc);
        if (!OK(status)) {
            fmt::print(stderr,
                       "memory::system::init error: {}\n",
                       memory::status_name(status));
            return (s32) status;
        }

        alloc = memory::system::default_alloc();
    }
    auto ctx = context::make(suite.argc, suite.argv, alloc);
    context::push(&ctx);

    TIME_BLOCK("memory::proxy::init"_ss,
               if (!OK(memory::proxy::init()))
                   return 1);

    TIME_BLOCK("log::system::init"_ss,
        default_config_t dft_config{};
        dft_config.file        = stderr;
        dft_config.process_arg = suite.argv[0];
        auto status = log::system::init(logger_type_t::default_,
                                        &dft_config,
                                        log_level_t::debug,
                                        memory::system::default_alloc());
        if (!OK(status)) {
            format::print(stderr,
                          "log::system::init error: {}\n",
                          log::status_name(status));
        }

        context::top()->logger = log::system::default_logger(););

    TIME_BLOCK("term::system::init"_ss,
               if (!OK(term::system::init(true)))
                   return 1);

    TIME_BLOCK("locale::system::init"_ss,
               if (!OK(locale::system::init()))
                   return 1);

    TIME_BLOCK("buf_pool::system::init"_ss,
               if (!OK(buf_pool::system::init()))
                   return 1);

    TIME_BLOCK("string::system::init"_ss,
               if (!OK(string::system::init()))
                   return 1);

    TIME_BLOCK("error::system::init"_ss,
               if (!OK(error::system::init()))
                   return 1);

    TIME_BLOCK("event::system::init"_ss,
               if (!OK(event::system::init()))
                   return 1);

    TIME_BLOCK("thread::system::init"_ss,
               if (!OK(thread::system::init()))
                   return 1);

    TIME_BLOCK("job::system::init"_ss,
               if (!OK(job::system::init()))
                   return 1);

    TIME_BLOCK("ffi::system::init"_ss,
               if (!OK(ffi::system::init()))
                   return 1);

    TIME_BLOCK("filesys::init"_ss,
               if (!OK(filesys::init()))
                   return 1);

    TIME_BLOCK("network::init"_ss,
               if (!OK(network::system::init()))
                   return 1);

    if (!suite.no_scm) {
        TIME_BLOCK("scm::system::init"_ss,
                   if (!OK(scm::system::init(256 * 1024)))
                       return 1);

        TIME_BLOCK("scm::module::basic::init"_ss,
                   if (!OK(scm::module::basic::system::init(scm::system::global_ctx())))
                       return 1);

        TIME_BLOCK("scm::module::log::init"_ss,
                   if (!OK(scm::module::log::system::init(scm::system::global_ctx())))
                       return 1);

        TIME_BLOCK("scm::module::cxx::init"_ss,
                   if (!OK(scm::module::cxx::system::init(scm::system::global_ctx())))
                       return 1);
    }

    TIME_BLOCK("config::system::init"_ss,
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

                   cvar_t* var{};
                   config::cvar::add("*enable-console-color*"_ss,
                                     cvar_type_t::flag,
                                     &var);
                   config::cvar::set(var, true);

                   config::cvar::add("*log-path*"_ss,
                                     cvar_type_t::string,
                                     &var);
                   config::cvar::set(var, "/var/log"_ss);

                   config::cvar::add("*magick-weight*"_ss,
                                     cvar_type_t::real,
                                     &var);
                   config::cvar::set(var, 47.314f);

                   auto   load_path = "../etc/first.scm"_path;
                   path_t config_path{};
                   filesys::bin_rel_path(config_path, load_path);
                   scm::obj_t* result{};
                   if (!OK(scm::system::eval(config_path, &result))) {
                       format::print(stderr,
                                     "{}\n",
                                     scm::printable_t{scm::system::global_ctx(),
                                                      result});
                       return 1;
                   }

                   path::free(config_path);
                   path::free(load_path));

    TIME_BLOCK("catch2 session::run"_ss, rc = suite.session.run(suite.argc,
                                                                suite.argv));

    log::notice("shutdown test program");

    if (!suite.no_scm) {
        TIME_BLOCK("scm::module::cxx::system::fini"_ss,
                   scm::module::cxx::system::fini());

        TIME_BLOCK("scm::module::log::system::fini"_ss,
                   scm::module::log::system::fini());

        TIME_BLOCK("scm::module::basic::system::fini"_ss,
                   scm::module::basic::system::fini());

        TIME_BLOCK("scm::system::fini"_ss,
                   scm::system::fini());
    }

    TIME_BLOCK("network::system::fini"_ss,              network::system::fini());
    TIME_BLOCK("filesys::fini"_ss,                      filesys::fini());
    TIME_BLOCK("ffi::system::fini"_ss,                  ffi::system::fini());
    TIME_BLOCK("job::system::fini"_ss,                  job::system::fini());
    TIME_BLOCK("thread::system::fini"_ss,               thread::system::fini());
    TIME_BLOCK("config::system::fini"_ss,               config::system::fini());
    TIME_BLOCK("string::system::fini"_ss,               string::system::fini());
    TIME_BLOCK("error::system::fini"_ss,                error::system::fini());
    TIME_BLOCK("buf_pool::system::fini"_ss,             buf_pool::system::fini());
    TIME_BLOCK("locale::system::fini"_ss,               locale::system::fini());
    TIME_BLOCK("log::system::fini"_ss,                  log::system::fini());
    TIME_BLOCK("term::system::fini"_ss,                 term::system::fini());
    TIME_BLOCK("event::system::fini"_ss,                event::system::fini());
    TIME_BLOCK("memory::proxy::fini"_ss,                memory::proxy::fini());
    TIME_BLOCK("memory::system::fini"_ss,               memory::system::fini());
    TIME_BLOCK_ALLOC("context::pop"_ss,                 alloc, context::pop());

    return rc;
}

s32 main(s32 argc, const s8** argv) {
    test_suite_t suite{};
    suite.argc         = argc;
    suite.argv         = argv;
    suite.extra_idx    = -1;
    suite.repeat_count = 1;

    // N.B. must init the profiler first so the stopwatch_t
    //      gives us meaningful results.
    //
    // !!! IMPORTANT !!!
    //
    // keep profiler dependency free!
    //
    if (!OK(profiler::init()))
        return 1;

    stopwatch_t watch{};
    stopwatch::start(watch);

    for (s32 i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--") == 0)
            suite.extra_idx = i;
    }

    if (suite.extra_idx != -1) {
        suite.argc = suite.extra_idx;
        for (s32 i = suite.extra_idx + 1; i < argc; ++i) {
            if (strcmp(argv[i], "--repeat") == 0) {
                suite.repeat_count = atoi(argv[i + 1]);
                if (suite.repeat_count < 1)
                    suite.repeat_count = 1;
                ++i;
            } else if (strcmp(argv[i], "--no-scm") == 0) {
                suite.no_scm = true;
            }
        }
    }

    for (s32 i = 0; i < suite.repeat_count; ++i) {
        s32 rc = run(suite);
        if (rc)
            return rc;
    }

    stopwatch::stop(watch);
    fprintf(stdout, "\n\n=============================================================================\n");
    fprintf(stdout, "total execution cycles ...................................... %d\n", suite.repeat_count);
    fprintf(stdout, "total elapsed time .......................................... ");
    print_time(stopwatch::elapsed(watch));
    fprintf(stdout, "average elapsed time per cycle .............................. ");
    print_time(stopwatch::elapsed(watch) / suite.repeat_count);
    fprintf(stdout, "\nNOTE: the profiler burns ~250ms at process start to calibrate\n"
                    "      to the machine.\n\n"
                    "              *** this time overhead is excluded from these numbers! ***\n\n"
                    "      to capture this overhead plus other platform specific process start costs,\n"
                    "      use *time* as a wrapper on this process and subtract the\n"
                    "      'total elapsed time' value from what *time* reports.\n\n");

    profiler::fini();

    return 0;
}
