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

#pragma once

#include <basecode/core/profiler/system.h>

namespace basecode::timers {

    struct timer_t;

    using timer_callback_t = b8 (*)(timer_t*, u0*);

    struct timer_t final {
        b8 active;
        u0* context;
        s64 expiry;
        s64 duration;
        timer_callback_t callback;
    };

    ///////////////////////////////////////////////////////////////////////////

    enum class init_result_t {
        ok,
    };

    u0 shutdown();

    timer_t* start(
        s64 duration,
        timer_callback_t callback,
        u0* context = {});

    u0 update(u0* ctx = {});

    u0 stop(timer_t* timer);

    init_result_t initialize();

}

