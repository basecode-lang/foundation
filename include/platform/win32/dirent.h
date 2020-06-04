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

#include <io.h>
//#include "unistd.h"

enum {
    DT_BLK,        // This is a block device.
    DT_CHR,        // This is a character device.
    DT_DIR,        // This is a directory.
    DT_FIFO,    // This is a named pipe (FIFO).
    DT_LNK,        // This is a symbolic link.
    DT_REG,        // This is a regular file.
    DT_SOCK,    // This is a UNIX domain socket.
    DT_UNKNOWN    // The file type could not be determined.
};

struct dirent {
    long           d_ino;
    unsigned short d_reclen;
    unsigned short d_namlen;
    unsigned char  d_type;
    char* d_name;
};

typedef ptrdiff_t handle_type; /* C99's intptr_t not sufficiently portable */

struct DIR {
#ifdef GLIB_STYLE
    handle_type         handle; /* -1 for failed rewind */
    struct _finddata_t  info;
    struct dirent       result; /* d_name null iff first time */
    char                *name;  /* null-terminated char string */
#else
    int dummy;
#endif
};

typedef int scandir_f(const struct dirent* d);

typedef int scandir_alphasort(const struct dirent** a, const struct dirent** b);

int alphasort(const struct dirent** a, const struct dirent** b);

int versionsort(const struct dirent** a, const struct dirent** b);

typedef struct DIR DIR;

struct DIR* opendir(const char* path);

struct dirent* readdir(struct DIR* dir);

int readdir_r(struct DIR* dir, struct dirent* entry, struct dirent** result);

int closedir(struct DIR* dir);

void rewinddir(struct DIR* dir);

long telldir(struct DIR* dir);

void seekdir(struct DIR* dir, long tell);

int scandir(const char* buf, struct dirent*** namelist, scandir_f sf, scandir_alphasort af);
