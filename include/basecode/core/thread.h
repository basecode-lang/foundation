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

#pragma once

#include <utility>
#include <pthread.h>
#include <basecode/core/str.h>

namespace basecode::thread {
    namespace system {
        u0 fini();

        u32 num_cores();

        proc_base_t* alloc_proc();

        u0 free_proc(proc_base_t* proc);

        status_t start(thread_t& thread);

        status_t status_from_errno(s32 err);

        status_t join(thread_t& thread, u0** ret);

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    s32 thread_id();

    thread_t& self();

    b8 is_main_thread();

    s32 main_thread_id();

    status_t free(thread_t& thread);

    status_t cancel(thread_t& thread);

    status_t detach(thread_t& thread);

    template <typename Ret>
    status_t join(thread_t& thread, Ret& ret_val) {
        if (!thread.joinable)
            return status_t::not_joinable;
        if (thread.detached)
            return status_t::already_detached;
        if (thread.joined)
            return status_t::already_joined;
        u0* ret{};
        auto status = system::join(thread, &ret);
        if (OK(status))
            ret_val = **((Ret**)ret);
        return status;
    }

    template <String_Concept T>
    status_t init(thread_t& thread, const T& name) {
        if (name.length > 15)
            return status_t::name_too_long;
        thread.pad        = 0;
        thread.proc       = {};
        thread.name       = (str::slice_t) name;
        thread.state      = thread_state_t::created;
        thread.joined     = false;
        thread.canceled   = false;
        thread.detached   = false;
        thread.joinable   = true;
        thread.cancelable = true;
        return status_t::ok;
    }

    template <typename Proc, typename... Args>
    status_t start(thread_t& thread, Proc proc, Args&&... args) {
        if (thread.state != thread_state_t::created)
            return status_t::invalid_state;
        auto mem = system::alloc_proc();
        thread.proc = new (mem) thread_proc_t(&thread,
                                              proc,
                                              std::forward_as_tuple(args...));
        return system::start(thread);
    }
}
