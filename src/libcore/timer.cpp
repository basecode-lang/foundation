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

#include <cstring>
#include <basecode/core/timer.h>

namespace basecode::timer {
    static constexpr u32        timer_max_count = 32;
    thread_local timer_t        t_timers[timer_max_count];
    thread_local timer_t*       t_available_timer = &t_timers[0];

    u0 fini() {
        t_available_timer = {};
        for (u32 i = 0; i < timer_max_count; ++i)
            t_timers[i].active = false;
    }

    status_t init() {
        std::memset(t_timers, 0, timer_max_count * sizeof(timer_t));
        return status_t::ok;
    }

    u0 stop(timer_t* timer) {
        if (!timer)
            return;
        timer->active = false;
        t_available_timer = timer;
    }

    timer_t* start(s64 ticks,
                   s64 duration,
                   timer_callback_t callback,
                   u0* user) {
        if (!t_available_timer)
            return {};
        auto timer = t_available_timer;
        timer->user     = user;
        timer->active   = true;
        timer->expiry   = ticks + duration;
        timer->duration = duration;
        timer->callback = callback;
        t_available_timer = {};
        for (u32 i = 0; i < timer_max_count; ++i) {
            if (!t_timers[i].active) {
                t_available_timer = &t_timers[i];
                break;
            }
        }
        return timer;
    }

    u0 update(s64 ticks, u0* user) {
        for (u32 i = 0; i < timer_max_count; ++i) {
            auto timer = &t_timers[i];
            if (!timer->active || ticks < timer->expiry || !timer->callback)
                continue;

            const auto eff_user = timer->user ? timer->user : user;
            auto kill = !timer->callback(timer, eff_user);
            if (kill) {
                timer->active = false;
                t_available_timer = timer;
            } else {
                timer->expiry = ticks + timer->duration;
            }
        }
    }
}
