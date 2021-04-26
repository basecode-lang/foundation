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

#include <cerrno>
#if defined(__APPLE__)
#   include <zconf.h>
#   include <syscall.h>
#elif defined(__linux__)
#   include <sys/types.h>
#endif
#include <pthread.h>
#include <basecode/core/thread.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::thread {
    thread_local thread_t*      t_self{};
    pthread_t                   g_main_thread{};
    s32                         g_main_thread_id{};

    static u0* bootstrap(u0* arg) {
        auto* proc = (proc_base_t*) arg;
        auto thread = proc->self();
        t_self = thread;
        auto name = thread->name;
        WITH_SLICE_AS_CSTR(name, pthread_setname_np(thread->handle, name););
        u0* ret = proc->invoke();
        thread->state = thread_state_t::exited;
        pthread_exit(ret);
        return nullptr;
    }

    namespace system {
        struct system_t final {
            alloc_t*            alloc;
            alloc_t*            proc_pool;
            u32                 num_cores;
        };

        system_t                g_system{};

        u0 fini() {
            memory::system::free(g_system.proc_pool);
            g_system.proc_pool = {};
            g_system.alloc     = {};
            g_main_thread      = {};
            g_main_thread_id   = {};
        }

        u32 num_cores() {
            return g_system.num_cores;
        }

        proc_base_t* alloc_proc() {
            return (proc_base_t*) memory::alloc(g_system.proc_pool);
        }

        status_t init(alloc_t* alloc) {
            g_system.alloc = alloc;
            g_main_thread    = pthread_self();
#if defined(_MSC_VER)
            g_main_thread_id = pthread_getthreadid_np();
#elif defined(__MINGW64__)
            g_main_thread_id = (s32) g_main_thread;
#elif defined(__linux__)
            g_main_thread_id = gettid();
#else
            static_assert(false, "pthread support is lacking on this platform");
#endif
            slab_config_t slab_config{};
            slab_config.name          = "thread::worker_slab";
            slab_config.buf_size      = 128;        // FIXME!
            slab_config.buf_align     = 8;
            slab_config.num_pages     = DEFAULT_NUM_PAGES;
            slab_config.backing.alloc = g_system.alloc;
            g_system.proc_pool = memory::system::make(&slab_config);
            g_system.num_cores = sysconf(_SC_NPROCESSORS_ONLN);
            return status_t::ok;
        }

        u0 free_proc(proc_base_t* proc) {
            proc->~proc_base_t();
            memory::free(g_system.proc_pool, proc);
        }

        status_t start(thread_t& thread) {
            auto rc = pthread_create(&thread.handle,
                                     nullptr,
                                     &bootstrap,
                                     thread.proc);
            if (rc == 0)
                thread.state = thread_state_t::running;
            return status_from_errno(rc);
        }

        status_t status_from_errno(s32 err) {
            switch (err) {
                case 0:             return status_t::ok;
                case EINVAL:
                case ESRCH:         return status_t::invalid_thread;
                case EAGAIN:        return status_t::create_thread_failure;
                case EPERM:         return status_t::insufficient_privilege;
                case EDEADLK:       return status_t::deadlock;
                default:            return status_t::error;
            }
        }

        status_t join(thread_t& thread, u0** ret) {
            auto rc = pthread_join(thread.handle, ret);
            if (rc == 0) {
                thread.joined = true;
                system::free_proc(thread.proc);
                thread.proc = {};
            }
            return system::status_from_errno(rc);
        }
    }

    s32 thread_id() {
#ifdef _MSC_VER
        return pthread_getthreadid_np();
#elif defined(__linux__)
        return gettid();
#elif defined(__APPLE__) || defined(__MINGW64__)
        return (s32) pthread_self();
#else
        static_assert(false, "pthread support is lacking on this platform");
#endif
    }

    thread_t& self() {
        return *t_self;
    }

    b8 is_main_thread() {
        return pthread_equal(pthread_self(), g_main_thread);
    }

    s32 main_thread_id() {
        return g_main_thread_id;
    }

    status_t free(thread_t& thread) {
        if (thread.state == thread_state_t::running
        && (thread.joinable && !thread.joined)) {
            u0* ret{};
            auto err = pthread_join(thread.handle, &ret);
            if (err != 0)
                return system::status_from_errno(err);
        }
        system::free_proc(thread.proc);
        thread.proc = {};
        return status_t::ok;
    }

    status_t cancel(thread_t& thread) {
        if (!thread.cancelable)  return status_t::not_cancelable;
        if (thread.canceled)     return status_t::already_canceled;
        auto rc = pthread_cancel(thread.handle);
        if (rc == 0) {
            thread.canceled = true;
            system::free_proc(thread.proc);
            thread.proc = {};
        }
        return system::status_from_errno(rc);
    }

    status_t detach(thread_t& thread) {
        if (thread.detached)
            return status_t::already_detached;
        auto rc = pthread_detach(thread.handle);
        if (rc == 0)
            thread.detached = true;
        return system::status_from_errno(rc);
    }
}
