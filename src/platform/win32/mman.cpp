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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cerrno>
#include <io.h>
#include <sys/mman.h>

#ifndef FILE_MAP_EXECUTE
#   define FILE_MAP_EXECUTE    0x0020
#endif

static int __map_mman_error(const DWORD err, const int deferrer) {
    if (err == 0)
        return 0;
    //TODO: implement
    return err;
}

static DWORD __map_mmap_prot_page(const int prot) {
    DWORD protect = 0;

    if (prot == PROT_NONE)
        return protect;

    if ((prot & PROT_EXEC) != 0) {
        protect = ((prot & PROT_WRITE) != 0) ?
                  PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    } else {
        protect = ((prot & PROT_WRITE) != 0) ?
                  PAGE_READWRITE : PAGE_READONLY;
    }

    return protect;
}

static DWORD __map_mmap_prot_file(const int prot) {
    DWORD desiredAccess = 0;

    if (prot == PROT_NONE)
        return desiredAccess;

    if ((prot & PROT_READ) != 0)
        desiredAccess |= FILE_MAP_READ;
    if ((prot & PROT_WRITE) != 0)
        desiredAccess |= FILE_MAP_WRITE;
    if ((prot & PROT_EXEC) != 0)
        desiredAccess |= FILE_MAP_EXECUTE;

    return desiredAccess;
}

void* mmap(void* addr, size_t len, int prot, int flags, int fields, OffsetType off) {
    HANDLE fm, h;

    void* map = MAP_FAILED;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4293)
#endif

    const auto dwFileOffsetLow  = (DWORD) (off & 0xFFFFFFFFL);
    const auto dwFileOffsetHigh = (DWORD) ((off >> 32) & 0xFFFFFFFFL);
    const DWORD protect          = __map_mmap_prot_page(prot);
    const DWORD desiredAccess    = __map_mmap_prot_file(prot);

    const OffsetType maxSize = off + (OffsetType) len;

    const auto dwMaxSizeLow  = (DWORD) (maxSize & 0xFFFFFFFFL);
    const auto dwMaxSizeHigh = (DWORD) ((maxSize >> 32) & 0xFFFFFFFFL);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    errno = 0;

    if (len == 0 || prot == PROT_EXEC) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    h = ((flags & MAP_ANONYMOUS) == 0) ?
        (HANDLE) _get_osfhandle(fields) : INVALID_HANDLE_VALUE;

    if ((flags & MAP_ANONYMOUS) == 0 && h == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return MAP_FAILED;
    }

    fm = CreateFileMapping(h, nullptr, protect, dwMaxSizeHigh, dwMaxSizeLow, nullptr);

    if (fm == nullptr) {
        errno = __map_mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    if ((flags & MAP_FIXED) == 0) {
        map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);
    } else {
        map = MapViewOfFileEx(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len, addr);
    }

    CloseHandle(fm);

    if (map == nullptr) {
        errno = __map_mman_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }

    return map;
}

int munmap(void* addr, size_t len) {
    if (UnmapViewOfFile(addr))
        return 0;

    errno = __map_mman_error(GetLastError(), EPERM);

    return -1;
}

int mprotect(void* addr, size_t len, int prot) {
    DWORD newProtect = __map_mmap_prot_page(prot);
    DWORD oldProtect = 0;

    if (VirtualProtect(addr, len, newProtect, &oldProtect))
        return 0;

    errno = __map_mman_error(GetLastError(), EPERM);

    return -1;
}

int msync(void* addr, size_t len, int flags) {
    if (FlushViewOfFile(addr, len))
        return 0;

    errno = __map_mman_error(GetLastError(), EPERM);

    return -1;
}

int mlock(const void* addr, size_t len) {
    if (VirtualLock((LPVOID) addr, len))
        return 0;

    errno = __map_mman_error(GetLastError(), EPERM);

    return -1;
}

int munlock(const void* addr, size_t len) {
    if (VirtualUnlock((LPVOID) addr, len))
        return 0;

    errno = __map_mman_error(GetLastError(), EPERM);

    return -1;
}
