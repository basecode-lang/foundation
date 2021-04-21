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

#include <basecode/core/mutex.h>

namespace basecode::mutex {
    static status_t status_from_errno(s32 err) {
        switch (err) {
            case 0:             return status_t::ok;
            case EBUSY:         return status_t::busy;
            case EINVAL:        return status_t::invalid_mutex;
            case ENOMEM:        return status_t::out_of_memory;
            case EAGAIN:        return status_t::create_mutex_failure;
            case EPERM:         return status_t::insufficient_privilege;
            case EDEADLK:       return status_t::thread_already_owns_lock;
            default:            return status_t::error;
        }
    }

    status_t free(mutex_t& mutex) {
        if (mutex.locked) {
            auto r = pthread_mutex_unlock(&mutex.handle);
            if (r != 0)
                return status_from_errno(r);
            mutex.locked = false;
        }
        return status_from_errno(pthread_mutex_destroy(&mutex.handle));
    }

    status_t init(mutex_t& mutex) {
        return status_from_errno(pthread_mutex_init(&mutex.handle, nullptr));
    }

    status_t lock(mutex_t& mutex) {
        auto r = pthread_mutex_lock(&mutex.handle);
        mutex.locked = r == 0;
        return status_from_errno(r);
    }

    status_t unlock(mutex_t& mutex) {
        auto r = pthread_mutex_unlock(&mutex.handle);
        if (r == 0)
            mutex.locked = false;
        return status_from_errno(r);
    }

    status_t try_lock(mutex_t& mutex) {
        auto r = pthread_mutex_trylock(&mutex.handle);
        mutex.locked = r == 0;
        return status_from_errno(r);
    }
}
