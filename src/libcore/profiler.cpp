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

#include <thread>
#include <basecode/core/profiler.h>
#include <cpuid.h>
#ifdef _WIN32
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#endif

namespace basecode::profiler {
    static s64 s_resolution     = 0;
    static f64 s_timer_mult     = 1.0;

    static f64 calibrate() {
#ifdef HW_TIMER
#   if !defined TARGET_OS_IOS && __ARM_ARCH >= 6
        return 1.0f;
#   elif defined(TSC_USING_QPC)
        LARGE_INTEGER li{};
        QueryPerformanceFrequency(&li);
        return f64(li.QuadPart) / f64(100000);
#   else
        using namespace std::chrono;

        _mm_mfence();
        const auto t0 = high_resolution_clock::now();
        const auto r0 = get_time();
        _mm_lfence();

#   ifdef _WIN32
        HANDLE mutex = CreateEventEx(0, 0, 0, EVENT_ALL_ACCESS);
        WaitForSingleObjectEx(mutex, 50, FALSE);
#   else
        std::this_thread::sleep_for(milliseconds(50));
#   endif

        _mm_mfence();
        const auto t1 = high_resolution_clock::now();
        const auto r1 = get_time();
        _mm_lfence();

        const auto dt = duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        const auto dr = r1 - r0;
        return f64(dt) / f64(dr);
#   endif
#else
        return 1.0f;
#endif
    }

    u0 fini() {
    }

    status_t init() {
        u32 regs[4];
        u32* rp = regs;

        __get_cpuid(0x80000001, rp, rp + 1, rp + 2, rp + 3);
        if (!(regs[3] & (1 << 27)))
            return status_t::no_cpu_rtdscp_support;

        __get_cpuid(0x80000007, rp, rp + 1, rp + 2, rp + 3);
        if (!(regs[3] & (1 << 8)))
            return status_t::no_cpu_invariant_tsc_support;

        s_timer_mult = calibrate();

        const auto iterations = 50000;
        auto min_diff = std::numeric_limits<int64_t>::max();
        for (s32 i = 0; i < iterations * 10; i++) {
            const auto t0i = get_time();
            const auto t1i = get_time();
            const auto dti = t1i - t0i;
            if (dti > 0 && dti < min_diff)
                min_diff = dti;
        }

        s_resolution = min_diff;

        return status_t::ok;
    }

    u64 timer_resolution() { return s_resolution; }

    f64 calibration_mult() { return s_timer_mult; }
}
