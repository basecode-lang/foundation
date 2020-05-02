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

#include "types.h"

namespace basecode {

    struct  timer_t;
    using   timer_callback_t = b8 (*)(timer_t*, u0*);

    struct timer_t final {
        u0*                 context;
        timer_callback_t    callback;
        s64                 expiry;
        s64                 duration;
        b8                  active;
    };

    namespace timer {
        enum class status_t : u8 {
            ok,
            error
        };

        u0 shutdown();

        status_t initialize();

        u0 stop(timer_t* timer);

        u0 update(s64 ticks, u0* ctx = {});

        timer_t* start(s64 ticks, s64 duration, timer_callback_t callback, u0* context = {});
    }
}

