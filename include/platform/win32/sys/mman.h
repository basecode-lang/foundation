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

#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(_WIN64)
    typedef int64_t OffsetType;
#else
    typedef uint32_t OffsetType;
#endif

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0xf
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)

#define MS_ASYNC        1
#define MS_SYNC         2
#define MS_INVALIDATE   4

void*   mmap(void *addr, size_t len, int prot, int flags, int fields, OffsetType off);
int     munmap(void *addr, size_t len);
int     mprotect(void *addr, size_t len, int prot);
int     msync(void *addr, size_t len, int flags);
int     mlock(const void *addr, size_t len);
int     munlock(const void *addr, size_t len);
#else
#   include_next <sys/mman.h>
#endif
