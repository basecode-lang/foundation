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

namespace basecode {
    struct job_task_base_t {
        virtual ~job_task_base_t() = default;

        virtual u0 run() = 0;

        virtual u0* get_ret_val() = 0;

        virtual u0 set_job(job_t* job) = 0;
    };

    template <typename Proc, typename... Args>
    struct job_task_t : job_task_base_t {
        using Result_Type       = std::invoke_result_t<Proc, Args...>;
        static
        constexpr b8 Is_Void    = std::is_void_v<Result_Type>;

        using Proc_Type         = Proc;
        using Args_Type         = std::tuple<Args...>;
        using Return_Type       = typename std::conditional<Is_Void,
                                                            s32,
                                                            Result_Type>::type;

        job_t*                  job;
        Proc_Type               proc;
        Args_Type               args;
        Return_Type             ret_val;

        job_task_t(Proc_Type proc, Args_Type args) : job(),
                                                     proc(proc),
                                                     args(std::move(args)),
                                                     ret_val() {
        }

        u0 run() override {
            if constexpr (Is_Void) {
                std::apply(proc, args);
                ret_val = {};
            } else {
                ret_val = std::apply(proc, args);
            }
        }

        u0* get_ret_val() override          { return &ret_val;   }
        u0 set_job(job_t* value) override   { job = value;       }
    };

    // N.B. job subsystem uses a slab allocator to store instances
    //      of job_task_t.  because job_task_t's size varies based on
    //      the types passed at compile time, we don't know the exact
    //      memory footprint.
    //
    //      assume there's 32 bytes of free memory per job_task_t to store
    //      the parameters passed.
    constexpr u32 job_task_buffer_size = 64;

    struct job_t final {
        job_task_base_t*        task;
        event_t                 finished;
        stopwatch_t             time;
        job_id_list_t           children;
        job_id                  parent;
        job_id                  id;
        u32                     label_id;
        job_state_t             state;

        b8 operator==(const job_t& other) const { return id == other.id; };
    };
    static_assert(sizeof(job_t) <= 72,
                  "sizeof(job_t) is now greater than 72 bytes!");

    namespace job {
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
}
