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

#include <dirent.h>
#include <basecode/core/utf.h>
#include <basecode/core/defer.h>

thread_local DIR t_dirs[256];

static DIR* find_free_dir() {
    for (int i = 0; i < 256; ++i) {
        if (t_dirs[i].free) {
            t_dirs[i].free = false;
            return &t_dirs[i];
        }
    }
    return nullptr;
}

int closedir(struct DIR* dir) {
    if (!dir) {
        errno = EINVAL;
        return -1;
    }
    FindClose(dir->handle);
    dir->handle = INVALID_HANDLE_VALUE;
    dir->free   = true;
    return 0;
}

long telldir(struct DIR* dir) {
    return 0;
}

void rewinddir(struct DIR* dir) {
}

int scandir(const char* buf,
            struct dirent*** namelist,
            scandir_f sf,
            scandir_alphasort af) {
    return 0;
}

struct DIR* opendir(const char* path) {
    basecode::utf16_str_t utf16{};
    basecode::utf::init(utf16);
    defer(basecode::utf::free(utf16));
    basecode::utf::append(utf16, basecode::slice::make(path));
    if (utf16[utf16.size - 1] != '*')
        basecode::utf::append(utf16, '*');

    auto dir = find_free_dir();
    if (!dir) {
        errno = ENOMEM;
        return nullptr;
    }

    dir->handle = FindFirstFileW((LPCWSTR) basecode::utf::c_str(utf16),
                                 (LPWIN32_FIND_DATAW) &dir->find_data);
    if (dir->handle == INVALID_HANDLE_VALUE) {
        errno = EINVAL;
        return nullptr;
    }

    dir->has_next = true;
    dir->at_end   = false;

    return dir;
}

struct dirent* readdir(struct DIR* dir) {
    if (!dir) {
        errno = EINVAL;
        return nullptr;
    }

    if (dir->at_end) {
        errno = ENOENT;
        return nullptr;
    }

    dir->e.d_namlen = 0;
    unsigned short* p = (unsigned short*) dir->find_data.cFileName;
    while (*p)
        dir->e.d_name[dir->e.d_namlen++] = *p++;
    dir->e.d_name[dir->e.d_namlen] = '\0';
    if (dir->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        dir->e.d_type = DT_DIR;
    else if (dir->find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        dir->e.d_type = DT_LNK;
    else
        dir->e.d_type = DT_REG;
    if (!dir->has_next) {
        dir->at_end = true;
    } else {
        dir->has_next = FindNextFileW(dir->handle,
                                      (LPWIN32_FIND_DATAW) &dir->find_data);
    }
    return &dir->e;
}

void seekdir(struct DIR* dir, long tell) {
}

int alphasort(const struct dirent** a, const struct dirent** b) {
    return 0;
}

int versionsort(const struct dirent** a, const struct dirent** b) {
    return 0;
}

int readdir_r(struct DIR* dir, struct dirent* entry, struct dirent** result) {
    return 0;
}


