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

#include <utility>
#include <pthread.h>
#include <basecode/core/types.h>
#include <basecode/core/slice.h>

namespace basecode {
    struct thread_t;

    enum class thread_state_t : u8 {
        exited,
        created,
        running,
        canceled,
    };

    struct proc_base_t {
        virtual ~proc_base_t() = default;

        virtual u0* invoke() = 0;

        virtual thread_t* self() = 0;
    };

    template <typename Proc, typename... Args>
    struct thread_proc_t final : public proc_base_t {
        static constexpr b8 is_void = std::is_void_v<std::invoke_result_t<Proc, Args...>>;

        using proc_t        = Proc;
        using args_t        = std::tuple<Args...>;
        using return_t      = typename std::conditional<is_void, int, std::invoke_result_t<Proc, Args...>>::type;

        thread_t*           thread;
        proc_t              proc;
        args_t              args;
        return_t            ret_val;

        thread_proc_t(thread_t* thread, proc_t proc, args_t args) : thread(thread), proc(proc), args(std::move(args)) {
        }

        u0* invoke() override {
            if constexpr (is_void) {
                std::apply(proc, args);
                ret_val = {};
                return nullptr;
            } else {
                ret_val = std::apply(proc, args);
                return &ret_val;
            }
        }

        thread_t* self() override { return thread; }
    };

    struct thread_t final {
        pthread_t           handle;
        proc_base_t*        proc;
        str::slice_t        name;
        thread_state_t      state;
        u8                  joined:     1;
        u8                  detached:   1;
        u8                  joinable:   1;
        u8                  canceled:   1;
        u8                  cancelable: 1;
        u8                  pad:        3;

        b8 operator==(const thread_t& other) const {
            return pthread_equal(handle, other.handle) != 0;
        }
    };

    namespace thread {
        enum class status_t : u8 {
            ok,
            error,
            deadlock,
            not_joinable,
            invalid_state,
            name_too_long,
            invalid_thread,
            already_joined,
            not_cancelable,
            already_canceled,
            already_detached,
            create_thread_failure,
            insufficient_privilege,
        };

        namespace system {
            u0 fini();

            u32 num_cores();

            proc_base_t* alloc_proc();

            u0 free_proc(proc_base_t* proc);

            status_t start(thread_t& thread);

            status_t status_from_errno(s32 err);

            status_t join(thread_t& thread, u0** ret);

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        thread_t& self();

        b8 is_main_thread();

        status_t free(thread_t& thread);

        status_t cancel(thread_t& thread);

        status_t detach(thread_t& thread);

        template <typename Ret>
        status_t join(thread_t& thread, Ret& ret_val) {
            if (!thread.joinable)   return status_t::not_joinable;
            if (thread.detached)    return status_t::already_detached;
            if (thread.joined)      return status_t::already_joined;
            u0* ret{};
            auto status = system::join(thread, &ret);
            if (OK(status))
                ret_val = **((Ret**)ret);
            return status;
        }

        status_t init(thread_t& thread, const String_Concept auto& name) {
            if (name.length > 15)
                return status_t::name_too_long;
            thread.pad        = 0;
            thread.proc       = {};
            thread.joined     = false;
            thread.canceled   = false;
            thread.detached   = false;
            thread.joinable   = true;
            thread.cancelable = true;
            thread.name       = (str::slice_t) name;
            thread.state      = thread_state_t::created;
            return status_t::ok;
        }

        template <typename Proc, typename... Args> status_t start(thread_t& thread, Proc proc, Args&&... args) {
            if (thread.state != thread_state_t::created)
                return status_t::invalid_state;
            auto mem = system::alloc_proc();
            thread.proc = new (mem) thread_proc_t(&thread, proc, std::forward_as_tuple(args...));
            return system::start(thread);
        }
    }
}
