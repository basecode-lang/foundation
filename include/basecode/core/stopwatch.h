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

#include <basecode/core/defer.h>
#include <basecode/core/format.h>

#define TIME_BLOCK_ALLOC(slug, alloc, ...)                                      \
    do {                                                                        \
        stopwatch_t timer{};                                                    \
        stopwatch::start(timer);                                                \
        defer(stopwatch::stop(timer);                                           \
              stopwatch::print_elapsed(                                         \
                  (alloc) ? (alloc) : memory::system::main_alloc(),             \
                  stdout,                                                       \
                  slug,                                                         \
                  60,                                                           \
                  timer));                                                      \
        __VA_ARGS__;                                                            \
    } while (false)
#define TIME_BLOCK(slug, ...)                                                   \
    do {                                                                        \
        stopwatch_t timer{};                                                    \
        stopwatch::start(timer);                                                \
        defer(stopwatch::stop(timer);                                           \
              stopwatch::print_elapsed(slug, 60, timer));                       \
        __VA_ARGS__;                                                            \
    } while (false)

namespace basecode {
    struct stopwatch_t final {
        u64                     end;
        u64                     start;
    };

    namespace stopwatch {
        u0 init(stopwatch_t& w);

        u0 stop(stopwatch_t& w);

        u0 start(stopwatch_t& w);

        u64 elapsed(stopwatch_t& w);

        u0 print_elapsed(alloc_t* alloc,
                         FILE* file,
                         const String_Concept auto& label,
                         s32 width,
                         stopwatch_t& w) {
            BC_ASSERT(s32(width - label.length) > 5);
            const auto sv_label = (std::string_view) label;
            const auto e = elapsed(w);
            if (e == 0) {
                format::print_ellipsis(alloc, file, sv_label, width, "---\n");
            } else if (e < 1000) {
                format::print_ellipsis(alloc, file, sv_label, width, "{}ns\n", e);
            } else {
                const auto us = e / 1000;
                if (us >= 1000) {
                    format::print_ellipsis(alloc, file, sv_label, width, "{}ms\n", us / 1000);
                } else {
                    format::print_ellipsis(alloc, file, sv_label, width, "{}us\n", us);
                }
            }
        }

        u0 print_elapsed(const String_Concept auto& label,
                         s32 width,
                         stopwatch_t& w) {
            print_elapsed(context::top()->alloc.scratch, stdout, label, width, w);
        }

        s32 time_block(const String_Concept auto& slug,
                       const timed_block_callable_t& cb, alloc_t* alloc = {}) {
            stopwatch_t timer{};
            start(timer);
            auto rc = cb();
            stop(timer);
            print_elapsed(alloc ? alloc : memory::system::scratch_alloc(),
                          stdout,
                          slug,
                          60,
                          timer);
            return rc;
        }
    }
}
