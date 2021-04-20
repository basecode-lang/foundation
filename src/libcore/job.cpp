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

#include <atomic>
#include <basecode/core/log.h>
#include <basecode/core/job.h>
#include <basecode/core/queue.h>
#include <basecode/core/thread.h>
#include <basecode/core/string.h>
#include <basecode/core/memory/system/dl.h>
#include <basecode/core/memory/system/slab.h>
#include <basecode/core/memory/system/proxy.h>
#include <basecode/core/memory/system/scratch.h>

namespace basecode::job {
    struct worker_t final {
        context_t*              parent_ctx;
        event_t                 wake;
        event_t                 started;
        mutex_t                 pending_mutex;
        thread_t                thread;
        job_id_list_t           active;
        job_id_list_t           pending;
        std::atomic<u8>         flags;
        s32                     id;
    };

    using worker_list_t         = array_t<worker_t>;

    struct system_t final {
        alloc_t*                alloc;
        alloc_t*                task_pool;
        job_list_t              jobs;
        worker_list_t           workers;
        mutex_t                 jobs_mutex;
        std::atomic<u32>        worker_idx;
        u32                     num_cores;
        u32                     num_workers;
    };

    system_t                    g_job_sys{};

    namespace worker {
        [[maybe_unused]] static constexpr u8 none           = 0b00000000;
        [[maybe_unused]] static constexpr u8 exit           = 0b00000001;
        [[maybe_unused]] static constexpr u8 has_pending    = 0b00000010;
        [[maybe_unused]] static constexpr u8 batch          = 0b00000100;
        [[maybe_unused]] static constexpr u8 interactive    = 0b00001000;
        [[maybe_unused]] static constexpr u8 started        = 0b00010000;

        b8 flag(const worker_t& worker, u8 flag) {
            return (worker.flags & flag) == flag;
        }

        u0 flag(worker_t& worker, u8 flag, b8 value) {
            if (value) {
                worker.flags |= flag;
            } else {
                worker.flags &= ~flag;
            }
        }

        static u0 run(worker_t& worker) {
            dl_config_t dl_cfg{};
            dl_cfg.heap_size = 4 * 1024 * 1024;

            scratch_config_t scratch_cfg{};
            scratch_cfg.buf_size = 256 * 1024;

            system_config_t sys_cfg{};
            sys_cfg.main = &dl_cfg;
            sys_cfg.scratch = &scratch_cfg;

            memory::system::init(&sys_cfg);
            auto ctx = context::make(worker.parent_ctx->argc,
                                     worker.parent_ctx->argv);
            ctx.logger        = worker.parent_ctx->logger;
            ctx.alloc.main    = memory::system::main_alloc();
            ctx.alloc.temp    = memory::system::temp_alloc();
            ctx.alloc.scratch = memory::system::scratch_alloc();
            context::push(&ctx);
            memory::proxy::init();
            array::init(worker.active);
            array::reserve(worker.active, 128);
            array::init(worker.pending);
            array::reserve(worker.pending, 128);
            worker::flag(worker, worker::started, true);
            event::set(worker.started);
            while (!worker::flag(worker, worker::exit)) {
                event::wait(worker.wake);
                if (worker::flag(worker, worker::has_pending)) {
                    scoped_lock_t lock(&worker.pending_mutex);
                    array::append(worker.active, worker.pending);
                    array::reset(worker.pending);
                    worker::flag(worker, worker::has_pending, false);
                }
                while (!array::empty(worker.active)) {
                    job_t* job{};
                    auto id = *array::back(worker.active);
                    array::pop(worker.active);
                    if (!OK(job::get(id, &job))) {
                        // XXX: log that we couldn't find a job
                        continue;
                    }
                    if (job && job->state == job_state_t::queued) {
                        job->state = job_state_t::running;
                        stopwatch::start(job->time);
                        job->task->run();
                        stopwatch::stop(job->time);
                        job->state = job_state_t::finished;
                        event::set(job->finished);
                    }
                }
            }
            worker::flag(worker, worker::started, false);
            array::free(worker.active);
            array::free(worker.pending);
            memory::proxy::fini();
            memory::system::fini();
            context::pop();
        }
    }

    namespace system {
        u0 fini() {
            for (auto& worker : g_job_sys.workers) {
                worker::flag(worker, worker::exit, true);
                event::set(worker.wake);
                thread::free(worker.thread);
                event::free(worker.wake);
                event::free(worker.started);
                mutex::free(worker.pending_mutex);
            }
            while (!stable_array::empty(g_job_sys.jobs))
                job::free(stable_array::back(g_job_sys.jobs)->id);
            array::free(g_job_sys.workers);
            mutex::free(g_job_sys.jobs_mutex);
            stable_array::free(g_job_sys.jobs);
            memory::system::free(g_job_sys.task_pool);
        }

        s32 number_of_workers() {
            return g_job_sys.num_workers;
        }

        job_task_base_t* alloc_task() {
            return (job_task_base_t*) memory::alloc(g_job_sys.task_pool);
        }

        status_t init(alloc_t* alloc) {
            g_job_sys.alloc       = alloc;
            g_job_sys.worker_idx  = {};

            slab_config_t slab_config{};
            slab_config.buf_size      = job_task_buffer_size;
            slab_config.buf_align     = alignof(job_task_base_t*);
            slab_config.num_pages     = DEFAULT_NUM_PAGES;
            slab_config.backing.alloc = g_job_sys.alloc;
            g_job_sys.task_pool = memory::system::make(&slab_config);

            mutex::init(g_job_sys.jobs_mutex);
            stable_array::init(g_job_sys.jobs, g_job_sys.alloc);
            array::init(g_job_sys.workers, g_job_sys.alloc);
            g_job_sys.num_cores   = thread::system::num_cores();
            g_job_sys.num_workers = std::max<s32>(g_job_sys.num_cores - 2, 2);
            array::reserve(g_job_sys.workers, g_job_sys.num_workers);
            log::debug("job::system, number of worker threads: {}",
                       g_job_sys.num_workers);
            str_t temp{};
            str::init(temp, g_job_sys.alloc);
            str::reserve(temp, 32);
            for (u32 i = 0; i < g_job_sys.num_workers; ++i) {
                auto& worker = array::append(g_job_sys.workers);
                worker.id         = i + 1;
                worker.flags      = worker::none;
                worker.parent_ctx = context::top();
                worker.started    = event::make(true);
                worker.wake       = event::make(false);
                mutex::init(worker.pending_mutex);
                str::reset(temp);
                {
                    str_buf_t buf(&temp);
                    format::format_to(buf, "job worker:{}", worker.id);
                }
                thread::init(worker.thread, string::interned::fold(temp));
                thread::start(worker.thread, &worker::run, worker);
            }
            str::free(temp);
            return status_t::ok;
        }

        u0 free_task(job_task_base_t* task) {
            memory::free(g_job_sys.task_pool, task);
        }

        status_t start(job_id id, job_task_base_t* task) {
            scoped_lock_t lock(&g_job_sys.jobs_mutex);
            if (id == 0 || id > g_job_sys.jobs.size)
                return status_t::invalid_job_id;
            auto job = &g_job_sys.jobs[id - 1];
            if (job->state != job_state_t::created)
                return status_t::invalid_job_state;
            task->set_job(job);
            job->task  = task;
            job->state = job_state_t::queued;
            g_job_sys.worker_idx++;
            const auto worker_idx = g_job_sys.worker_idx % g_job_sys.num_workers;
            auto& worker = g_job_sys.workers[worker_idx];
            scoped_lock_t pending_lock(&worker.pending_mutex);
            event::wait(worker.started);
            array::append(worker.pending, id);
            worker::flag(worker, worker::has_pending, true);
            event::set(worker.wake);
            return status_t::ok;
        }

        status_t make(const s8* label, s32 len, job_id& id, job_id parent_id) {
            scoped_lock_t lock(&g_job_sys.jobs_mutex);
            job_t* parent_job{};
            if (parent_id) {
                if (parent_id > g_job_sys.jobs.size)
                    return status_t::invalid_job_id;
                parent_job = &g_job_sys.jobs[parent_id - 1];
            }
            intern::result_t ir{};
            if (label) {
                ir = string::interned::fold_for_result(slice::make(label,
                                                                   len));
                if (!OK(ir.status))
                    return status_t::label_intern_failure;
            }
            auto job = &stable_array::append(g_job_sys.jobs);
            id = g_job_sys.jobs.size;
            job::init(*job, id, ir.id, parent_id);
            if (parent_job)
                array::append(parent_job->children, id);
            return status_t::ok;
        }
    }

    u0 free(job_id id) {
        scoped_lock_t lock(&g_job_sys.jobs_mutex);
        if (id == 0 || id > g_job_sys.jobs.size)
            return;
        job_t& job = g_job_sys.jobs[id - 1];
        event::free(job.finished);
        array::free(job.children);
        memory::free(g_job_sys.task_pool, job.task);
        stable_array::erase(g_job_sys.jobs, &job);
    }

    status_t stop(job_id id) {
        scoped_lock_t lock(&g_job_sys.jobs_mutex);
        job_t* job{};
        if (!OK(get(id, &job)))
            return status_t::invalid_job_id;
        if (job->state == job_state_t::finished)
            return status_t::invalid_job_state;
        job->state = job_state_t::finished;
        return status_t::ok;
    }

    str::slice_t label(job_id id) {
        scoped_lock_t lock(&g_job_sys.jobs_mutex);
        if (id == 0 || id > g_job_sys.jobs.size)
            return {};
        const auto& job = g_job_sys.jobs[id - 1];
        auto result = string::interned::get(job.label_id);
        return OK(result.status) ? result.slice :
               string::localized::status_name(result.status);
    }

    u0 all(array_t<const job_t*>& list) {
        scoped_lock_t lock(&g_job_sys.jobs_mutex);
        array::reset(list);
        for (const auto job : g_job_sys.jobs)
            array::append(list, job);
    }

    status_t get(job_id id, job_t** job) {
        scoped_lock_t lock(&g_job_sys.jobs_mutex);
        if (id == 0 || id > g_job_sys.jobs.size)
            return status_t::invalid_job_id;
        *job = &g_job_sys.jobs[id - 1];
        return status_t::ok;
    }

    status_t wait(job_id id, s64 timeout) {
        event_t e;
        {
            scoped_lock_t lock(&g_job_sys.jobs_mutex);
            if (id == 0 || id > g_job_sys.jobs.size)
                return status_t::invalid_job_id;
            const auto& job = g_job_sys.jobs[id - 1];
            e = job.finished;
        }
        return OK(event::wait(e, timeout)) ? status_t::ok :
               status_t::busy;
    }

    str::slice_t state_name(job_state_t state) {
        str::slice_t* s{};
        string::localized::find(u32(state), &s);
        return *s;
    }

    u0 init(job_t& job, job_id id, u32 label_id, job_id parent_id) {
        job.id       = id;
        job.task     = {};
        job.label_id = label_id;
        job.parent   = parent_id;
        job.finished = event::make();
        job.state    = job_state_t::created;
        stopwatch::init(job.time);
        array::init(job.children, g_job_sys.alloc);
    }
}
