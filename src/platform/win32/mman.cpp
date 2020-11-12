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

#define DWORD_HI(x)             ((x) >> 32U)
#define DWORD_LO(x)             ((x) & 0xffffffffU)

struct mmap_tag_t final {
    HANDLE                      fdh     {};
    HANDLE                      fmh     {};
    void*                       addr    {};
    mmap_tag_t*                 next    {};
    bool                        free    {true};
};

thread_local mmap_tag_t         s_tags[256] {};
thread_local mmap_tag_t*        s_head_tag  {};

static mmap_tag_t* tag_alloc() {
    for (auto& s_tag : s_tags) {
        if (s_tag.free) {
            s_tag.free = false;
            return &s_tag;
        }
    }
    return nullptr;
}

static void tag_free(void* addr) {
    mmap_tag_t** prev = &s_head_tag;
    for (auto tag = *prev;
         tag != nullptr;
         prev = &tag->next, tag = *prev) {
        if (tag->addr == addr) {
            CloseHandle(tag->fmh);
            *prev = tag->next;
            tag->free = true;
            break;
        }
    }
}

static mmap_tag_t* tag_find(void* addr) {
    mmap_tag_t** prev = &s_head_tag;
    for (auto tag = *prev;
         tag != nullptr;
         prev = &tag->next, tag = *prev) {
        if (tag->addr == addr)
            return tag;
    }
    return nullptr;
}

int munmap(void* addr, size_t len) {
    (void) len;
    if (UnmapViewOfFile(addr)) {
        tag_free(addr);
        return 0;
    }
    errno = EPERM;
    return -1;
}

int mlock(const void* addr, size_t len) {
    if (VirtualLock((LPVOID) addr, len))
        return 0;
    errno = EPERM;
    return -1;
}

int munlock(const void* addr, size_t len) {
    if (VirtualUnlock((LPVOID) addr, len))
        return 0;
    errno = EPERM;
    return -1;
}

int msync(void* addr, size_t len, int flags) {
    DWORD fFlags = flags;
    if (FlushViewOfFile(addr, len)) {
        if ((fFlags & MS_SYNC) == MS_SYNC) {
            auto tag = tag_find(addr);
            if (tag) {
                if (!FlushFileBuffers(tag->fdh)) {
                    errno = EPERM;
                    return -1;
                }
            }
        }
        return 0;
    }
    errno = EPERM;
    return -1;
}

int mprotect(void* addr, size_t len, int prot) {
    DWORD flOldProtect{};
    DWORD flProtect;
    DWORD fProt = prot;
    if (fProt & PROT_WRITE) {
        if (fProt & PROT_EXEC)
            flProtect = PAGE_EXECUTE_READWRITE;
        else
            flProtect = PAGE_READWRITE;
    } else if (fProt & PROT_EXEC) {
        if (fProt & PROT_READ)
            flProtect = PAGE_EXECUTE_READ;
        else
            flProtect = PAGE_EXECUTE;
    } else {
        flProtect = PAGE_READONLY;
    }

    if (VirtualProtect(addr, len, flProtect, &flOldProtect))
        return 0;

    errno = EPERM;
    return -1;
}

void* mmap(void* addr, size_t len, int prot, int flags, int fd, uint64_t offset) {
    DWORD fProt  = prot;
    DWORD fFlags = flags;

    errno = 0;

    if (fProt & ~(PROT_READ | PROT_WRITE | PROT_EXEC)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (fd == -1) {
        if (!(fFlags & MAP_ANON) || offset) {
            errno = EINVAL;
            return MAP_FAILED;
        }
    } else if (fFlags & MAP_ANON) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    DWORD flProtect;
    if (fProt & PROT_WRITE) {
        if (fProt & PROT_EXEC)
            flProtect = PAGE_EXECUTE_READWRITE;
        else
            flProtect = PAGE_READWRITE;
    } else if (fProt & PROT_EXEC) {
        if (fProt & PROT_READ)
            flProtect = PAGE_EXECUTE_READ;
        else
            flProtect = PAGE_EXECUTE;
    } else {
        flProtect = PAGE_READONLY;
    }

    uint64_t end = len + offset;
    HANDLE fdh, fmh;
    if (fd == -1)
        fdh = INVALID_HANDLE_VALUE;
    else
        fdh = (HANDLE) _get_osfhandle(fd);
    fmh = CreateFileMapping(fdh,
                          nullptr,
                          flProtect,
                          DWORD_HI(end),
                          DWORD_LO(end),
                          nullptr);
    if (fmh == nullptr) {
        errno = EPERM;
        return MAP_FAILED;
    }

    DWORD dwDesiredAccess;
    if (fProt & PROT_WRITE)
        dwDesiredAccess = DWORD(FILE_MAP_WRITE);
    else
        dwDesiredAccess = DWORD(FILE_MAP_READ);

    if (fProt & PROT_EXEC)
        dwDesiredAccess |= DWORD(FILE_MAP_EXECUTE);

    if (fFlags & MAP_PRIVATE)
        dwDesiredAccess |= DWORD(FILE_MAP_COPY);

    void* ret = MAP_FAILED;
    if (fFlags & MAP_FIXED) {
        ret = MapViewOfFileEx(fmh,
                              dwDesiredAccess,
                              DWORD_HI(offset),
                              DWORD_LO(offset),
                              len,
                              addr);
    } else {
        ret = MapViewOfFile(fmh,
                            dwDesiredAccess,
                            DWORD_HI(offset),
                            DWORD_LO(offset),
                            len);
    }
    if (ret == nullptr) {
        CloseHandle(fmh);
        errno = EPERM;
    } else {
        auto tag = tag_alloc();
        if (tag) {
            tag->fdh  = fdh;
            tag->fmh  = fmh;
            tag->addr = ret;
            tag->next = s_head_tag;
            s_head_tag = tag;
        }
    }

    return ret;
}
