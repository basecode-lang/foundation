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

#include <chrono>
#include <x86intrin.h>
#if __APPLE__
#   include <mach/mach_time.h>
#endif
#include <basecode/core/types.h>

#if defined _WIN32                                                              \
    || defined __CYGWIN__                                                       \
    || ((defined __i386                                                         \
        || defined _M_IX86                                                      \
        || defined __x86_64__                                                   \
        || defined _M_X64)                                                      \
    && !defined __ANDROID__)                                                    \
    || __ARM_ARCH >= 6
#  define HW_TIMER
#endif

namespace basecode::profiler {
    enum class status_t : u8 {
        ok                              = 0,
        no_cpu_rtdscp_support           = 164,
        no_cpu_invariant_tsc_support    = 165
    };

    u0 fini();

    status_t init();

    u64 timer_resolution();

    f64 calibration_mult();

    FORCE_INLINE u64 get_time() {
        using namespace std::chrono;
#ifdef HW_TIMER
#  if TARGET_OS_IOS == 1
        return mach_absolute_time();
#  elif __ARM_ARCH >= 6
#    ifdef CLOCK_MONOTONIC_RAW
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return s64(ts.tv_sec) * 1000000000ll + s64(ts.tv_nsec);
#    else
        return duration_cast<nanoseconds>(high_resolution_clock::now()
                                            .time_since_epoch())
                                         .count();
#    endif
#  elif defined _WIN32 || defined __CYGWIN__
        LARGE_INTEGER li{};
        QueryPerformanceCounter(&li);
        return li.QuadPart;
#  elif defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64
        u32 eax, edx;
        _mm_mfence();
        asm volatile ( "rdtscp" : "=a" (eax), "=d" (edx)::"%ecx" );
        _mm_lfence();
        return (u64(edx) << (u8) 32) + u64(eax);
#  endif
#else
        return duration_cast<nanoseconds>(high_resolution_clock::now()
                                            .time_since_epoch())
                                         .count();
#endif
    }
}

