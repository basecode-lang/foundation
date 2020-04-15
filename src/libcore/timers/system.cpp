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

#include <cassert>
#include <basecode/core/profiler/system.h>
#include "system.h"

namespace basecode::timers {

    static constexpr u32 max_timers = 256;

    thread_local timer_t t_timers[max_timers];
    thread_local timer_t* t_available_timer = &t_timers[0];

    u0 shutdown() {
    }

    u0 update(u0* ctx) {
        const auto ticks = profiler::get_time() * profiler::get_calibration_multiplier();
        for (auto& timer : t_timers) {
            if (!timer.active || ticks < timer.expiry)
                continue;

            auto kill = !timer.callback(
                &timer,
                timer.context ? timer.context : ctx);
            if (kill) {
                timer.active = false;
                t_available_timer = &timer;
            } else {
                timer.expiry = ticks + timer.duration;
            }
        }
    }

    timer_t* start(
            s64 duration,
            timer_callback_t callback,
            u0* context) {
        assert(callback);
        auto timer = t_available_timer;
        timer->active = true;
        timer->context = context;
        timer->duration = duration;
        timer->callback = callback;
        timer->expiry = (profiler::get_time() * profiler::get_calibration_multiplier()) + duration;
        t_available_timer = nullptr;
        for (auto& t : t_timers) {
            if (!t.active) {
                t_available_timer = &t;
                break;
            }
        }
        assert(t_available_timer);
        return timer;
    }

    u0 stop(timer_t* timer) {
        if (!timer) return;
        timer->active = false;
        t_available_timer = timer;
    }

    init_result_t initialize() {
        return init_result_t::ok;
    }

}