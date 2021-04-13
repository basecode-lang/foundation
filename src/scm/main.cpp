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
#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/getopt.h>
#include <basecode/core/locale.h>
#include <basecode/core/thread.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/scm/configure.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/cxx.h>
#include <basecode/core/scm/modules/log.h>
#include <basecode/core/scm/modules/basic.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>

using namespace basecode;

static getopt_t                 s_cl        {};
static jmp_buf                  s_top_level {};
static u8                       s_buf[4096] {};

static u0 on_error(scm::ctx_t* ctx, const s8* msg, scm::obj_t* cl) {
    UNUSED(ctx);
    UNUSED(cl);
    format::print(stderr, "{}", msg);
    longjmp(s_top_level, -1);
}

static u0 usage(getopt_t& cl) {
    str_t buf{};
    str::init(buf);
    defer(str::free(buf));
    getopt::format_help(cl, buf);
    format::print("{}\n", buf);
}

static s32 repl(scm::ctx_t* ctx, s32 argc, const s8** argv) {
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
            scm::obj_t* obj{};
            path_t load_path{};
            path::init(load_path, file_arg->subclass.string);
            defer(path::free(load_path));
            if (!OK(scm::system::eval(load_path, &obj)))
                return EXIT_FAILURE;
            format::print(stdout,
                          "{}\n",
                          scm::printable_t{ctx, obj, true});
        }
    }

    FILE* fp = stdin;
    scm::set_error_handler(ctx, on_error);

    auto gc = scm::save_gc(ctx);

    setjmp(s_top_level);

    while (true) {
        scm::restore_gc(ctx, gc);
        format::print(stdout, "> ");
        u32 paren       {0};
        u32 count       {0};
        u32 pos         {0};
        s32 tmp;
        u8  ch;
        b8  expr_ready  {};
        do {
            tmp = fgetc(fp);
            ++count;
            switch (tmp) {
                case 4:
                    if (fgetc(fp) == 10)
                        goto exit;
                    break;
                case EOF:
                    goto exit;
                case '(':
                case '[':
                    ++paren;
                    break;
                case ')':
                case ']':
                    if (paren > 0) {
                        --paren;
                    }
                    break;
                case '\n':
                    expr_ready = paren == 0 && count > 0;
                    if (!expr_ready && paren > 0) {
                        format::print(stdout,
                                      "{:<{}}",
                                      " ",
                                      paren * 2);
                        fflush(stdout);
                    }
                    break;
                default:
                    break;
            }
            ch = tmp;
            s_buf[pos++] = ch;
        } while (!expr_ready);
        scm::obj_t* obj = scm::nil(ctx);
        scm::system::eval(s_buf, pos, &obj);
        format::print(stdout,
                      "{}\n",
                      scm::printable_t{ctx, obj, true});
    }
exit:
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

    {
        auto status = memory::system::init(alloc_type_t::dlmalloc);
        if (!OK(status)) {
            fmt::print(stderr,
                       "memory::system::init error: {}\n",
                       memory::status_name(status));
            return s32(status);
        }
        auto ctx = context::make(argc,
                                 argv,
                                 memory::system::default_alloc());
        context::push(&ctx);
    }

    {
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
        settings.product_name  = string::interned::fold(SCM_PRODUCT_NAME);
        settings.build_type    = string::interned::fold(SCM_BUILD_TYPE);
        settings.version.major = SCM_VERSION_MAJOR;
        settings.version.minor = SCM_VERSION_MINOR;
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
        if (!OK(scm::system::eval(config_path, &result)))
            return 1;

        path::free(config_path);
        path::free(load_path);
    }

    rc = repl(scm::system::global_ctx(), argc, argv);

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
