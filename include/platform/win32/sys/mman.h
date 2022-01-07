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

#ifdef _WIN32

#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>

#define PROT_NONE       0U
#define PROT_READ       1U
#define PROT_WRITE      2U
#define PROT_EXEC       4U

#define MAP_FILE        0U
#define MAP_SHARED      1U
#define MAP_PRIVATE     2U
#define MAP_TYPE        0xfU
#define MAP_FIXED       0x10U
#define MAP_ANONYMOUS   0x20U
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)

#define MS_ASYNC        1U
#define MS_SYNC         2U
#define MS_INVALIDATE   4U

void*   mmap(void *addr, size_t len, int prot, int flags, int fd, uint64_t offset);
int     munmap(void *addr, size_t len);
int     mprotect(void *addr, size_t len, int prot);
int     msync(void *addr, size_t len, int flags);
int     mlock(const void *addr, size_t len);
int     munlock(const void *addr, size_t len);

#else
#   include_next <sys/mman.h>
#endif
