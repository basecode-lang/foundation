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
#include "profiler.h"
#include "stopwatch.h"

namespace basecode::stopwatch {
    u0 stop(stopwatch_t& w) {
        w.end = profiler::get_time();
    }

    u0 start(stopwatch_t& w) {
        w.end = {};
        w.start = profiler::get_time();
    }

    s64 elapsed(stopwatch_t& w) {
        auto delta = w.end - w.start;
        return delta * profiler::get_calibration_multiplier();
    }

    u0 print_elapsed(string::slice_t label, s32 width, s64 elapsed) {
        format::print("{}", label);
        format::print("{:.<{}}", ".", width - label.length);
        if (elapsed < 1000) {
            format::print("{}ns\n", elapsed);
        } else {
            const auto us = elapsed / 1000;
            if (us >= 1000) {
                format::print("{}ms\n", (f64) us / 1000);
            } else {
                format::print("{}us\n", us);
            }
        }
    }
}
