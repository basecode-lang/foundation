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

#pragma once

#include <basecode/core/types.h>
#include <basecode/core/profiler.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/memory/system/temp.h>

namespace basecode {
    struct test_suite_t final {
        alloc_t*                temp_alloc;
        Catch::Session          session;
        const s8**              argv;
        s32                     argc;
        s32                     extra_idx;
        u32                     repeat_count;
        u8                      no_scm:         1;
        u8                      pad:            7;
    };

    namespace test_suite {
        inline u0 print_time(u64 e) {
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

        inline s32 run(s32 argc, const s8** argv, suite_runner_t runner) {
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
                s32 rc = runner(suite);
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
    }
}
