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

#include <sys/time.h>
#include <basecode/core/mutex.h>
#include <basecode/core/event.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    struct waiter_t final {
        pthread_cond_t                  cvar;
        mutex_t                         mutex;
        s32                             ref_count;
        union {
            s32                         fired_event;
            s32                         events_left;
        }                               status;
        u8                              wait_all:       1;
        u8                              still_waiting:  1;
        u8                              pad:            6;
    };

    struct waiter_info_t final {
        waiter_t                        waiter;
        s32                             index;
    };
    using waiter_array_t                = array_t<waiter_info_t>;

    struct event_t_ final {
        pthread_cond_t                  cvar;
        mutex_t                         mutex;
        waiter_array_t                  waiters;
        u8                              auto_reset:     1;
        u8                              state:          1;
        u8                              pad:            6;
    };
    static_assert(sizeof(event_t_) <= 152, "sizeof(event_t_) is now greater than 152 bytes!");

    using event_array_t                 = array_t<event_t>;

    namespace event {
        struct system_t final {
            alloc_t*                    alloc;
            alloc_t*                    event_pool;
            event_array_t               events;
        };

        system_t                        g_system{};

        namespace system {
            u0 fini() {
                event_array_t temp{};
                array::copy(temp, g_system.events);
                for (auto event : temp)
                    event::free(event);
                array::free(temp);
                array::free(g_system.events);
                memory::system::free(g_system.event_pool);
            }

            status_t init(alloc_t* alloc) {
                g_system.alloc = alloc;
                array::init(g_system.events, g_system.alloc);

                slab_config_t slab_config{};
                slab_config.backing   = g_system.alloc;
                slab_config.buf_size  = sizeof(event_t_);
                slab_config.buf_align = alignof(event_t_);
                slab_config.num_pages = 1;
                g_system.event_pool = memory::system::make(alloc_type_t::slab, &slab_config);

                return status_t::ok;
            }
        }

        static inline u0 get_timeout(timespec& ts, int64_t timeout) {
            if (timeout == -1) return;
            timeval tv{};
            gettimeofday(&tv, nullptr);
            u64 ns = u64(tv.tv_sec) * 1000 * 1000 * 1000 + timeout * 1000 * 1000 + u64(tv.tv_usec) * 1000;
            ts.tv_sec  = ns / 1000 / 1000 / 1000;
            ts.tv_nsec = s64(ns - (u64(ts.tv_sec) * 1000 * 1000 * 1000));
        }

        static status_t unlocked_wait(event_t event, int64_t timeout) {
            s32 result{};

            if (!event->state) {
                if (timeout == 0)
                    return status_t::timeout;

                timespec ts{};
                get_timeout(ts, timeout);

                do {
                    if (timeout != -1) {
                        result = pthread_cond_timedwait(&event->cvar, &event->mutex.handle, &ts);
                    } else {
                        result = pthread_cond_wait(&event->cvar, &event->mutex.handle);
                    }
                } while (result == 0 && !event->state);

                if (result == 0 && event->auto_reset) {
                    event->state = false;
                }
            } else if (event->auto_reset) {
                result = 0;
                event->state = false;
            }

            return result == 0 ? status_t::ok : status_t::timeout;
        }

        u0 free(event_t event) {
            pthread_cond_destroy(&event->cvar);
            mutex::free(event->mutex);
            array::free(event->waiters);
            array::erase(g_system.events, event);
            memory::free(g_system.event_pool, event);
        }

        status_t set(event_t event) {
            mutex::lock(event->mutex);
            event->state = true;
            if (event->auto_reset) {
                while (!array::empty(event->waiters)) {
                    waiter_info_t* info = array::back(event->waiters);
                    mutex::lock(info->waiter.mutex);
                    --info->waiter.ref_count;
                    if (!info->waiter.still_waiting) {
                        b8 destroy = info->waiter.ref_count == 0;
                        mutex::unlock(info->waiter.mutex);
                        if (destroy) {
                            pthread_cond_destroy(&info->waiter.cvar);
                            mutex::free(info->waiter.mutex);
                        }
                        array::pop(event->waiters);
                        continue;
                    }
                    event->state = false;
                    if (info->waiter.wait_all) {
                        --info->waiter.status.events_left;
                    } else {
                        info->waiter.status.fired_event = info->index;
                        info->waiter.still_waiting      = false;
                    }
                    mutex::unlock(info->waiter.mutex);
                    pthread_cond_signal(&info->waiter.cvar);
                    array::pop(event->waiters);
                    mutex::unlock(event->mutex);
                    return status_t::ok;
                }
                if (event->state) {
                    mutex::unlock(event->mutex);
                    pthread_cond_signal(&event->cvar);
                    return status_t::ok;
                }
            } else {
                for (u32 i = 0; i < event->waiters.size; ++i) {
                    waiter_info_t* info = &event->waiters[i];
                    mutex::lock(info->waiter.mutex);
                    --info->waiter.ref_count;
                    if (!info->waiter.still_waiting) {
                        b8 destroy = info->waiter.ref_count == 0;
                        mutex::unlock(info->waiter.mutex);
                        if (destroy) {
                            pthread_cond_destroy(&info->waiter.cvar);
                            mutex::free(info->waiter.mutex);
                        }
                        continue;
                    }
                    if (info->waiter.wait_all) {
                        --info->waiter.status.events_left;
                    } else {
                        info->waiter.status.fired_event = info->index;
                        info->waiter.still_waiting      = false;
                    }
                    mutex::unlock(info->waiter.mutex);
                    pthread_cond_signal(&info->waiter.cvar);
                }
                array::reset(event->waiters);
                mutex::unlock(event->mutex);
                pthread_cond_broadcast(&event->cvar);
            }
            return status_t::ok;
        }

        status_t reset(event_t event) {
            scoped_lock_t lock(event->mutex);
            event->state = false;
            return status_t::ok;
        }

        status_t pulse(event_t event) {
            if (OK(set(event)))
                return reset(event);
            return status_t::error;
        }

        status_t wait(event_t event, s64 timeout) {
            if (timeout == 0) {
                auto status = mutex::try_lock(event->mutex);
                if (status == mutex::status_t::busy)
                    return status_t::timeout;
            } else {
                mutex::lock(event->mutex);
            }
            auto status = unlocked_wait(event, timeout);
            mutex::unlock(event->mutex);
            return status;
        }

        event_t make(b8 manual_reset, b8 initial_state) {
            auto event = (event_t) memory::alloc(g_system.event_pool);
            pthread_cond_init(&event->cvar, 0);
            mutex::init(event->mutex);
            event->state      = initial_state;
            event->auto_reset = !manual_reset;
            array::init(event->waiters);
            if (initial_state)
                set(event);
            array::append(g_system.events, event);
            return event;
        }
    }
}
