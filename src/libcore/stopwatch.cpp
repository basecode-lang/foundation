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

    u64 elapsed(stopwatch_t& w) {
        if (w.start > w.end) return 0;
        f64 delta = w.end - w.start;
        return delta * profiler::calibration_mult();
    }
}
