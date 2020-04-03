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

#include "system.h"
#include "stopwatch.h"

namespace basecode::profiler {

    u0 stopwatch_t::stop() {
        _end = get_time();
    }

    u0 stopwatch_t::start() {
        _start = get_time();
    }

    s64 stopwatch_t::elapsed() const {
        f64 delta = _end - _start;
        return delta * get_calibration_multiplier();
    }

}