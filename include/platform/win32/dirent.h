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

#include <io.h>
#include <unistd.h>

enum {
    DT_BLK,                     // This is a block device.
    DT_CHR,                     // This is a character device.
    DT_DIR,                     // This is a directory.
    DT_FIFO,                    // This is a named pipe (FIFO).
    DT_LNK,                     // This is a symbolic link.
    DT_REG,                     // This is a regular file.
    DT_SOCK,                    // This is a UNIX domain socket.
    DT_UNKNOWN                  // The file type could not be determined.
};

struct dirent {
    long                        d_ino;
    unsigned short              d_reclen;
    unsigned short              d_namlen;
    unsigned char               d_type;
    char                        d_name[256];
};

struct DIR {
    HANDLE                      handle      {INVALID_HANDLE_VALUE};
    WIN32_FIND_DATA             find_data;
    dirent                      e;
    int                         free        {true};
    int                         has_next    {false};
    int                         at_end      {false};
};

typedef struct DIR DIR;

typedef int scandir_f(const struct dirent* d);

typedef int scandir_alphasort(const struct dirent** a, const struct dirent** b);

int closedir(struct DIR* dir);

long telldir(struct DIR* dir);

void rewinddir(struct DIR* dir);

int scandir(const char* buf,
            struct dirent*** namelist,
            scandir_f sf,
            scandir_alphasort af);

struct DIR* opendir(const char* path);

struct dirent* readdir(struct DIR* dir);

void seekdir(struct DIR* dir, long tell);

int alphasort(const struct dirent** a, const struct dirent** b);

int versionsort(const struct dirent** a, const struct dirent** b);

int readdir_r(struct DIR* dir, struct dirent* entry, struct dirent** result);
