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

#include <thread>
#include <cpuid.h>
#include <cassert>
#include <basecode/core/profiler.h>

namespace basecode::profiler {
    static s64      s_resolution = 0;
    static f64      s_timer_multiplier = 1.0;

    static f64 calibrate() {
#ifdef HW_TIMER
#   if !defined TARGET_OS_IOS && __ARM_ARCH >= 6
        return 1.0f;
#   else
        using namespace std::chrono;

        std::atomic_signal_fence(std::memory_order_acq_rel);
        const auto t0 = high_resolution_clock::now();
        const auto r0 = get_time();
        std::atomic_signal_fence(std::memory_order_acq_rel);
        std::this_thread::sleep_for(milliseconds(200));
        std::atomic_signal_fence(std::memory_order_acq_rel);
        const auto t1 = high_resolution_clock::now();
        const auto r1 = get_time();
        std::atomic_signal_fence(std::memory_order_acq_rel);

        const auto dt = duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        const auto dr = r1 - r0;
        return f64(dt) / f64(dr);
#   endif
#else
        return 1.0f;
#endif
    }

    static u0 cpuid(u32* regs, u32 leaf) {
        __get_cpuid(leaf, regs, regs + 1, regs + 2, regs + 3);
    }

    ///////////////////////////////////////////////////////////////////////////

    u0 fini() {
    }

    status_t init() {
        u32 regs[4];

        cpuid(regs, 0x80000001);
        if (!(regs[3] & (1 << 27)))
            return status_t::no_cpu_rtdscp_support;

        cpuid(regs, 0x80000007);
        if (!(regs[3] & (1 << 8)))
            return status_t::no_cpu_invariant_tsc_support;

        s_timer_multiplier = calibrate();

        auto min_diff = std::numeric_limits<int64_t>::max();
        for (s32 i = 0; i < 50000 * 10; i++) {
            const auto t0i = get_time();
            const auto t1i = get_time();
            const auto dti = t1i - t0i;
            if (dti > 0 && dti < min_diff)
                min_diff = dti;
        }

        s_resolution = min_diff;

        return status_t::ok;
    }

    s64 timer_resolution() {
        return s_resolution;
    }

    f64 calibration_mult() {
        return s_timer_multiplier;
    }
}
