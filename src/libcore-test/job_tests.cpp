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

#include <catch.hpp>
#include <basecode/core/job.h>
#include <basecode/core/format.h>

using namespace basecode;

static s32 fib(s32 n) {
    s32 f[n + 2];
    f[0] = 0;
    f[1] = 1;
    for (s32 i = 2; i <= n; i++)
        f[i] = f[i - 1] + f[i - 2];
    return f[n];
}

TEST_CASE("basecode::job basics") {
    const auto num_workers = job::system::number_of_workers() * 2;

    job_id jobs[num_workers];
    s32    inputs[num_workers];
    s32    n = 2;
    str_t  temp{};
    str::init(temp, context::top()->alloc.temp);
    str::reserve(temp, 50);
    for (s32 i = 0; i < num_workers; ++i) {
        str::reset(temp);
        {
            str_buf_t buf(&temp);
            format::format_to(buf, "fib job {}", i + 1);
        }
        if (!OK(job::make(jobs[i], temp)))
            REQUIRE(false);
        inputs[i] = n++;
        if (!OK(job::start(jobs[i], fib, inputs[i])))
            REQUIRE(false);
    }

    s32 i = 0;
    while (i < num_workers) {
        if (OK(job::wait(jobs[i]))) {
            s32 result{};
            if (!OK(job::return_value(jobs[i], result)))
                REQUIRE(false);
            const auto label = job::label(jobs[i]);
            job_t* job{};
            if (!OK(job::get(jobs[i], &job)))
                REQUIRE(false);
            stopwatch::print_elapsed(label, 60, job->time);
            format::print_ellipsis((std::string_view) label,
                                   60,
                                   "result: {}\n",
                                   result);
            ++i;
        }
    }
}
