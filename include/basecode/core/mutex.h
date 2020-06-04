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
#include <basecode/core/str.h>
#include <basecode/core/types.h>

namespace basecode {
    struct mutex_t;

    namespace mutex {
        enum class status_t : u8 {
            ok,
            busy,
            error,
            invalid_mutex,
            out_of_memory,
            create_mutex_failure,
            insufficient_privilege,
            thread_already_owns_lock,
        };

        status_t free(mutex_t& mutex);

        status_t init(mutex_t& mutex);

        status_t lock(mutex_t& mutex);

        status_t unlock(mutex_t& mutex);

        status_t try_lock(mutex_t& mutex);

        str::slice_t status_name(status_t status);
    }

    struct mutex_t final {
        pthread_mutex_t             handle;
        b8                          locked;
    };

    struct scoped_lock_t final {
        mutex_t&                    mutex;

        explicit scoped_lock_t(mutex_t& m) : mutex(m) { mutex::lock(mutex);   }
        ~scoped_lock_t()                              { mutex::unlock(mutex); }
    };
}
