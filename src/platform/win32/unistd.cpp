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


#include <basecode/core/types.h>
#include <io.h>
#include <cstdio>
#include <fcntl.h>
#include <windows.h>
#include <sys/stat.h>

FILE* fmemopen(void* buf, size_t len, const char* type) {
    UNUSED(type);

    int fd;
    FILE *fp;
    char tp[MAX_PATH - 13];
    char fn[MAX_PATH + 1];

    if (!GetTempPathA(sizeof(tp), tp))
        return nullptr;

    if (!GetTempFileNameA(tp, "basecode", 0, fn))
        return nullptr;

    fd = _open(fn, _O_CREAT
                   | _O_RDWR
                   | _O_SHORT_LIVED
                   | _O_TEMPORARY
                   | _O_BINARY,
               _S_IREAD | _S_IWRITE);
    if (fd == -1)
        return nullptr;

    fp = _fdopen(fd, "w+");
    if (!fp) {
        _close(fd);
        return nullptr;
    }

    fwrite(buf, len, 1, fp);
    rewind(fp);

    return fp;
}

long sysconf(int name) {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    switch (name) {
        case _SC_PAGE_SIZE:         return system_info.dwPageSize;
        case _SC_NPROCESSORS_ONLN:  return system_info.dwNumberOfProcessors;
        default:                    break;
    }
    return 0;
}
