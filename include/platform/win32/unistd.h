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

#ifdef _WIN32
#include <stdio.h>

#define _SC_PAGE_SIZE           1
#define _SC_NPROCESSORS_ONLN    2

long sysconf(int name);

FILE* fmemopen(void* buf, size_t len, const char* type);
#endif

#ifdef _MSC_VER
#if _MSC_VER == 1900
#include <vcruntime.h>
#include <corecrt_io.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <winnt.h>
#include <profileapi.h>
#include <cstdlib>
#include <io.h>
#include <process.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/sys_types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <combaseapi.h>

#define PATH_MAX            260

#define strdup              _strdup
#define unlink              _unlink
#define mkdir               _mkdir
#define rmdir               _rmdir
#define lstat               stat
#define lseek               _lseek
#define isatty              _isatty
#define getcwd              _getcwd
#define dup2                _dup2
#define dup                 _dup
#define open                _open
#define close               _close
#define chdir               _chdir
#define getpid              _getpid
#define access              _access
#define fileno              _fileno
#define STDIN_FILENO        _fileno(stdin)
#ifndef strcasecmp
#   define strcasecmp       _stricmp
#endif
#define strncasecmp         _strnicmp
#define strtok_r            strtok_s
#define bzero(address,size) memset(address,0,size)
#define pow10(x)            pow(x,10)
#define alloca              _alloca
#define srandom             srand
#define random              rand
#define isatty              _isatty

typedef unsigned long long useconds_t;

__forceinline int read(int const fd, void * const buffer, unsigned const buffer_size) {
    return _read(fd, buffer, buffer_size);
}

__forceinline int write(int fd, const void* buffer, unsigned int buffer_size) {
    return _write(fd, buffer, buffer_size);
}

__forceinline int usleep(useconds_t usec) {
    LARGE_INTEGER time1;
    LARGE_INTEGER time2;
    LARGE_INTEGER freq;
    time1.QuadPart = 0;
    time2.QuadPart = 0;
    freq.QuadPart  = 0;
    QueryPerformanceCounter(&time1);
    QueryPerformanceFrequency(&freq);
    do {
        QueryPerformanceCounter(&time2);
    } while ((time2.QuadPart - time1.QuadPart) < usec);
    return 0;
}
#else
#   include_next <unistd.h>
#endif
