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

#include <setjmp.h>
#include <basecode/core/log.h>
#include <basecode/core/ffi.h>
#include <basecode/core/job.h>
#include <basecode/core/scm.h>
#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/locale.h>
#include <basecode/core/config.h>
#include <basecode/core/thread.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/scm/configure.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>

using namespace basecode;

static jmp_buf  s_top_level;
static s8       s_buf[64000];

static u0 on_error(scm::ctx_t* ctx, const s8* msg, scm::obj_t* cl) {
    UNUSED(ctx);
    UNUSED(cl);
    fprintf(stderr, "error: %s\n", msg);
    longjmp(s_top_level, -1);
}

static s32 repl(s32 argc, const s8** argv) {
    FILE* fp = stdin;
    auto ctx = scm::init(s_buf, sizeof(s_buf));

    if (argc > 1) {
        fp = fopen(argv[1], "rb");
        if (!fp)
            scm::error(ctx, "could not open input file");
    }

    if (fp == stdin)
        scm::handlers(ctx)->error = on_error;

    auto gc = scm::save_gc(ctx);

    setjmp(s_top_level);

    scm::obj_t* obj;

    while (true) {
        scm::restore_gc(ctx, gc);
        if (fp == stdin) {
            fprintf(stdout, "> ");
            fflush(stdout);
        }
        obj = scm::read_fp(ctx, fp);
        if (!obj)
            break;
        obj = scm::eval(ctx, obj);
        if (fp == stdin) {
            scm::write_fp(ctx, obj, stdout);
            fprintf(stdout, "\n");
            fflush(stdout);
        }
    }

    scm::free(ctx);

    return EXIT_SUCCESS;
}

s32 main(s32 argc, const s8** argv) {
    {
        auto status = memory::system::init(alloc_type_t::dlmalloc);
        if (!OK(status)) {
            format::print(stderr, "memory::system::init error: {}\n",
                          memory::status_name(status));
            return (s32) status;
        }
    }
    {
        default_config_t dft_config{};
        dft_config.file = stderr;
        dft_config.process_arg = argv[0];
        auto status = log::system::init(logger_type_t::default_,
                                        &dft_config,
                                        log_level_t::debug,
                                        memory::system::default_alloc());
        if (!OK(status)) {
            format::print(stderr, "log::system::init error: {}\n",
                          log::status_name(status));
        }
    }

    auto ctx = context::make(argc,
                             argv,
                             memory::system::default_alloc(),
                             log::system::default_logger());
    context::push(&ctx);

    if (!OK(term::system::init(true)))  return 1;
    if (!OK(locale::system::init()))    return 1;
    if (!OK(buf_pool::system::init()))  return 1;
    if (!OK(string::system::init()))    return 1;
    if (!OK(error::system::init()))     return 1;

    {
        config_settings_t settings{};
        settings.product_name  = string::interned::fold(SCM_PRODUCT_NAME);
        settings.build_type    = string::interned::fold(SCM_BUILD_TYPE);
        settings.version.major = SCM_VERSION_MAJOR;
        settings.version.minor = SCM_VERSION_MINOR;
        settings.test_runner   = false;
        auto status = config::system::init(settings);
        if (!OK(status)) {
            format::print(stderr, "config::system::init error\n");
            return 1;
        }
    }

    auto core_config_path = "../etc/core.scm"_path;
    path_t config_path{};
    filesys::bin_rel_path(config_path, core_config_path);
    scm::obj_t* result{};
    if (!OK(config::eval(config_path, &result))) return 1;

    if (!OK(profiler::init()))          return 1;
    if (!OK(memory::proxy::init()))     return 1;
    if (!OK(event::system::init()))     return 1;
    if (!OK(thread::system::init()))    return 1;
    if (!OK(job::system::init()))       return 1;
    if (!OK(ffi::system::init()))       return 1;
    if (!OK(filesys::init()))           return 1;
    if (!OK(network::system::init()))   return 1;

    s32 rc = repl(argc, argv);

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
    string::system::fini();
    error::system::fini();
    buf_pool::system::fini();
    locale::system::fini();
    log::system::fini();
    term::system::fini();
    memory::proxy::fini();
    memory::system::fini();
    context::pop();

    return rc;
}
