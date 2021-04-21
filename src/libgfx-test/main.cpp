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
#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/locale.h>
#include <basecode/core/thread.h>
#include <basecode/core/filesys.h>
#include <basecode/core/network.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/profiler.h>
#include <basecode/gfx/configure.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/cxx.h>
#include <basecode/core/scm/modules/log.h>
#include <basecode/core/memory/system/dl.h>
#include <basecode/core/scm/modules/basic.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/proxy.h>
#include <basecode/core/memory/system/scratch.h>
#include "app.h"

using namespace basecode;

s32 main(s32 argc, const s8** argv) {
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
        dl_cfg.heap_size = 32 * 1024 * 1024;

        scratch_config_t scratch_cfg{};
        scratch_cfg.buf_size = 256 * 1024;

        system_config_t sys_cfg{};
        sys_cfg.main    = &dl_cfg;
        sys_cfg.temp    = {};
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

    TIME_BLOCK("memory::proxy::init"_ss,
               if (!OK(memory::proxy::init()))
                   return 1);

    TIME_BLOCK("log::system::init"_ss,
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

    TIME_BLOCK("config::system::init"_ss,
               config_settings_t settings{};
                   settings.ctx           = scm::system::global_ctx();
                   settings.product_name  = string::interned::fold(GFX_PRODUCT_NAME);
                   settings.build_type    = string::interned::fold(GFX_BUILD_TYPE);
                   settings.version.major = GFX_VERSION_MAJOR;
                   settings.version.minor = GFX_VERSION_MINOR;
                   settings.test_runner   = false;
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
                   path::free(load_path));

    s32 rc = run(argc, argv);

    TIME_BLOCK("scm::module::cxx::system::fini"_ss,
               scm::module::cxx::system::fini());

    TIME_BLOCK("scm::module::log::system::fini"_ss,
               scm::module::log::system::fini());

    TIME_BLOCK("scm::module::basic::system::fini"_ss,
               scm::module::basic::system::fini());

    TIME_BLOCK("scm::system::fini"_ss,                  scm::system::fini());
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
    TIME_BLOCK_ALLOC("context::pop"_ss,                 main_alloc, context::pop());

    profiler::fini();

    return rc;
}