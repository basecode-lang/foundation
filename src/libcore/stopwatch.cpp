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

#include <basecode/core/format.h>
#include <basecode/core/profiler.h>
#include <basecode/core/stopwatch.h>

namespace basecode::stopwatch {
    u0 init(stopwatch_t& w) {
        w.start = w.end = {};
    }

    u0 stop(stopwatch_t& w) {
        w.end = profiler::get_time();
    }

    u0 start(stopwatch_t& w) {
        w.end = {};
        w.start = profiler::get_time();
    }

    s64 elapsed(stopwatch_t& w) {
        auto delta = w.end - w.start;
        return delta * profiler::calibration_mult();
    }

    u0 print_elapsed(str::slice_t label, s32 width, stopwatch_t& w) {
        const auto sv_label = (std::string_view) label;
        const auto e = elapsed(w);
        if (e <= 0) {
            format::print_ellipsis(sv_label, width, "---\n", e);
        } else if (e < 1000) {
            format::print_ellipsis(sv_label, width, "{}ns\n", e);
        } else {
            const auto us = e / 1000;
            if (us >= 1000) {
                format::print_ellipsis(sv_label, width, "{}ms\n", (f64) us / 1000);
            } else {
                format::print_ellipsis(sv_label, width, "{}us\n", us);
            }
        }
    }
}
