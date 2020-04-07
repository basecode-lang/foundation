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

#include <chrono>
#include <foundation/types.h>
#if __APPLE__
#   include <mach/mach_time.h>
#endif

#if defined _WIN32 || defined __CYGWIN__ || ((defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64) && !defined __ANDROID__) || __ARM_ARCH >= 6
#  define HW_TIMER
#endif

namespace basecode::profiler {

    enum class init_result_t {
        ok,
        no_cpu_rtdscp_support,
        no_cpu_invariant_tsc_support
    };

    u0 shutdown();

    init_result_t initialize();

    s64 get_timer_resolution();

    f64 get_calibration_multiplier();

    [[maybe_unused]] static force_inline s64 get_time() {
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
        return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
#    endif
#  elif defined _WIN32 || defined __CYGWIN__
        unsigned int dontcare;
        const auto t = s64(__rdtscp(&dontcare));
        return t;
#  elif defined __i386 || defined _M_IX86 || defined __x86_64__ || defined _M_X64
        u32 eax, edx;
        asm volatile ( "rdtscp" : "=a" (eax), "=d" (edx)::"%ecx" );
        return (u64(edx) << (u8) 32) + u64(eax);
#  endif
#else
        return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
#endif
    }

}
