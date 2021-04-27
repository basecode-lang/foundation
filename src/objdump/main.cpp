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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/log.h>
#include <basecode/core/ffi.h>
#include <basecode/core/job.h>
#include <basecode/core/env.h>
#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/getopt.h>
#include <basecode/core/thread.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/scm/system.h>
#include <basecode/objdump/configure.h>
#include <basecode/core/scm/modules/cxx.h>
#include <basecode/core/scm/modules/log.h>
#include <basecode/core/memory/system/dl.h>
#include <basecode/core/scm/modules/basic.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>
#include <basecode/core/memory/system/scratch.h>

using namespace basecode;

static getopt_t                 s_cl        {};
static u8                       s_buf[4096] {};

static u0 usage(getopt_t& cl) {
    str_t buf{};
    str::init(buf);
    defer(str::free(buf));
    getopt::format_help(cl, buf);
    format::print("{}\n", buf);
}

static s32 run(s32 argc, const s8** argv) {
    getopt::init(s_cl, argc, argv);
    defer(getopt::free(s_cl));
    {
        getopt::program_description(
            s_cl,
            "The 'scm' program provides a "
            "REPL for the core scheme terp."_ss);
        auto status = getopt::make_option(s_cl)
            .min_required(0)
            .max_allowed(1)
            .type(arg_type_t::file)
            .short_name('f')
            .long_name("file"_ss)
            .value_name("SCRIPT"_ss)
            .description("use file SCRIPT instead of "
                         "reading from stdin"_ss)
            .build();
        if (!OK(status))
            return EXIT_FAILURE;

        status = getopt::make_option(s_cl)
            .short_name('?')
            .long_name("help"_ss)
            .description("give this help list"_ss)
            .build();
        if (!OK(status))
            return EXIT_FAILURE;

        status = getopt::parse(s_cl);
        if (!OK(status)) {
            usage(s_cl);
            return EXIT_FAILURE;
        }

        auto help_arg = getopt::find_arg(s_cl, '?');
        if (help_arg) {
            usage(s_cl);
            return EXIT_SUCCESS;
        }

        auto file_arg = getopt::find_arg(s_cl, 'f');
        if (file_arg) {
            // XXX:
        }
    }

    return EXIT_SUCCESS;
}

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

    alloc_t* main_alloc;
    alloc_t* temp_alloc;
    alloc_t* scratch_alloc; {
        dl_config_t dl_cfg{};
        dl_cfg.heap_size = MB(32);

        scratch_config_t scratch_cfg{};
        scratch_cfg.buf_size = KB(256);

        system_config_t sys_cfg{};
        sys_cfg.main = &dl_cfg;
        sys_cfg.scratch = &scratch_cfg;

        auto status = memory::system::init(&sys_cfg);
        if (!OK(status)) {
            fmt::print(stderr,
                       "memory::system::init error: {}\n",
                       memory::status_name(status));
            return (s32) status;
        }

        main_alloc    = memory::system::main_alloc();
        temp_alloc    = memory::system::temp_alloc();
        scratch_alloc = memory::system::scratch_alloc();
    }

    auto ctx = context::make(argc, argv);
    ctx.alloc.main    = main_alloc;
    ctx.alloc.temp    = temp_alloc;
    ctx.alloc.scratch = scratch_alloc;
    context::push(&ctx);

    {
        default_config_t dft_config{};
        dft_config.file        = stderr;
        dft_config.process_arg = argv[0];
        auto status = log::system::init(logger_type_t::default_,
                                        &dft_config,
                                        log_level_t::debug,
                                        main_alloc);
        if (!OK(status)) {
            format::print(stderr,
                          "log::system::init error: {}\n",
                          log::status_name(status));
            return s32(status);
        }

        context::top()->logger = log::system::default_logger();
    }

    if (!OK(term::system::init(true)))
        return 1;
    if (!OK(locale::system::init()))
        return 1;
    if (!OK(buf_pool::system::init()))
        return 1;
    if (!OK(string::system::init()))
        return 1;
    if (!OK(env::system::init()))
        return 1;
    if (!OK(error::system::init()))
        return 1;
    if (!OK(memory::proxy::init()))
        return 1;
    if (!OK(event::system::init()))
        return 1;
    if (!OK(thread::system::init()))
        return 1;
    if (!OK(job::system::init()))
        return 1;
    if (!OK(ffi::system::init()))
        return 1;
    if (!OK(filesys::init()))
        return 1;
    if (!OK(network::system::init()))
        return 1;
    if (!OK(scm::system::init(256 * 1024)))
        return 1;
    if (!OK(scm::module::basic::system::init(scm::system::global_ctx())))
        return 1;
    if (!OK(scm::module::log::system::init(scm::system::global_ctx())))
        return 1;
    if (!OK(scm::module::cxx::system::init(scm::system::global_ctx())))
        return 1;

    {
        config_settings_t settings{};
        settings.ctx           = scm::system::global_ctx();
        settings.product_name  = string::interned::fold(OBJDUMP_PRODUCT_NAME);
        settings.build_type    = string::interned::fold(OBJDUMP_BUILD_TYPE);
        settings.version.major = OBJDUMP_VERSION_MAJOR;
        settings.version.minor = OBJDUMP_VERSION_MINOR;
        settings.test_runner   = true;
        auto status = config::system::init(settings);
        if (!OK(status)) {
            format::print(stderr, "config::system::init error\n");
            return 1;
        }

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
        path::free(load_path);
    }

    auto status = binfmt::system::init();
    if (!OK(status)) {
        format::print(stderr,
                      "binfmt::system::init error: {}\n",
                      string::localized::status_name(status));
        return (s32) status;
    }

    memory::system::mark_initialized();

    rc = run(argc, argv);

    binfmt::system::fini();
    network::system::fini();
    filesys::fini();
    ffi::system::fini();
    job::system::fini();
    thread::system::fini();
    config::system::fini();
    scm::module::cxx::system::fini();
    scm::module::log::system::fini();
    scm::module::basic::system::fini();
    scm::system::fini();
    env::system::fini();
    string::system::fini();
    error::system::fini();
    buf_pool::system::fini();
    locale::system::fini();
    log::system::fini();
    term::system::fini();
    event::system::fini();
    memory::proxy::fini();
    memory::system::fini();
    context::pop();
    profiler::fini();

    return rc;
}