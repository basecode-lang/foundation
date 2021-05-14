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

#include <basecode/core/str.h>
#include <basecode/core/array.h>
#include <basecode/core/event.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/stable_array.h>

namespace basecode::job {
    namespace system {
        u0 fini();

        s32 number_of_workers();

        job_task_base_t* alloc_task();

        u0 free_task(job_task_base_t* task);

        status_t make(const s8* label,
                      s32 len,
                      job_id& id,
                      job_id parent_id = {});

        status_t start(job_id id, job_task_base_t* task);

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    u0 free(job_id id);

    status_t stop(job_id id);

    str::slice_t label(job_id id);

    u0 all(array_t<const job_t*>& list);

    status_t get(job_id id, job_t** job);

    str::slice_t state_name(job_state_t state);

    status_t wait(job_id id, s64 timeout = -1);

    template <typename T>
    status_t return_value(job_id id, T& value) {
        job_t* job{};
        if (!OK(get(id, &job)))
            return status_t::invalid_job_id;
        value = *((T*)job->task->get_ret_val());
        return status_t::ok;
    }

    template <typename Proc, typename... Args>
    status_t start(job_id id, Proc proc, Args&&... args) {
        auto mem    = system::alloc_task();
        auto task   = new(mem) job_task_t(proc,
                                          std::forward_as_tuple(args...));
        auto status = system::start(id, task);
        if (!OK(status))
            system::free_task(task);
        return status;
    }

    status_t make_child(job_id parent_id,
                        job_id& id,
                        const String_Concept auto& label = {}) {
        if (label.length > 0) {
            return system::make(label.data, label.length, id, parent_id);
        } else {
            return system::make(nullptr, 0, id, parent_id);
        }
    }

    u0 init(job_t& job, job_id id, u32 label_id, job_id parent_id);

    status_t make(job_id& id, const String_Concept auto& label = {}) {
        if (label.length > 0) {
            return system::make((const s8*) label.data, label.length, id);
        } else {
            return system::make(nullptr, 0, id);
        }
    }
}
