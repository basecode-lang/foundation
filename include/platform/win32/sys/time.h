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

#ifdef _MSC_VER
#include <WinSock2.h>
#include <ctime>
#include <cstdint>
#include <corecrt.h>

enum {
    ITIMER_REAL,
    ITIMER_VIRTUAL,
    ITIMER_PROF
};

struct itimerval {
    struct timeval it_interval; /* Interval for periodic timer */
    struct timeval it_value;    /* Time until next expiration */
};

inline int getitimer(int which, struct itimerval* curr_value) {
    return 0;
}

inline int setitimer(int which, const struct itimerval* new_value, struct itimerval* old_value) {
    return 0;
}

inline char* ctime_r(const time_t* t, char* result) {
    const unsigned bufsize = 26;
    const errno_t  err     = ctime_s(result, bufsize, t);
    return err ? nullptr : result;
}

inline char* asctime_r(const struct tm* tm, char* result) {
    const unsigned bufsize = 26;
    const errno_t  err     = asctime_s(result, bufsize, tm);
    return err ? nullptr : result;
}

inline struct tm* gmtime_r(const time_t* t, struct tm* result) {
    const errno_t err = gmtime_s(result, t);
    return err ? nullptr : result;
}

inline struct tm* localtime_r(const time_t* t, struct tm* result) {
    const errno_t err = localtime_s(result, t);
    return err ? nullptr : result;
}

inline int gettimeofday(struct timeval* tp, struct timezone* tzp) {
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME   file_time;
    uint64_t   time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t) file_time.dwLowDateTime);
    time += (uint64_t) file_time.dwHighDateTime << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}

#else
#   include_next <sys/time.h>
#endif
