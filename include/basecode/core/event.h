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

#include <pthread.h>
#include <basecode/core/array.h>

namespace basecode {
    struct  event_t_;
    using   event_t = event_t_*;

    namespace event {
        enum class status_t : u8 {
            ok,
            error,
            timeout,
        };

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        u0 free(event_t event);

        status_t set(event_t event);

        status_t reset(event_t event);

        status_t pulse(event_t event);

        status_t wait(event_t event, s64 timeout = -1);

        event_t make(b8 manual_reset = true, b8 initial_state = false);
    }
}
