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

#include_next "sys/types.h"

typedef intptr_t       pid_t;
typedef int            gid_t;
typedef int            uid_t;
typedef int            sigval_t;
typedef int            sigset_t;
typedef unsigned short ushort;
typedef int            key_t;
typedef unsigned short mode_t;
typedef int            gid_t;
typedef int            uid_t;
typedef int            Atom;
#ifdef _WIN64
#   define ssize_t __int64
#else
#   define ssize_t long
#endif

enum {
    F_DUPFD,
    F_GETFD,
    F_SETFD,
    F_GETFL,
    F_SETFL,
    F_GETLK,
    F_SETLK,
    F_SETLKW,
    FD_CLOEXEC
};

#define R_OK 4
#define W_OK 2
#define F_OK 0

#ifndef S_ISREG
#define S_ISREG(x) (_S_IFREG & x)
#endif

#define S_ISLNK(x) 0

#ifndef S_ISDIR
#define S_ISDIR(x) (_S_IFDIR & x)
#endif

#define S_IXUSR _S_IEXEC
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define S_IXOTH S_IEXEC
#define S_IXGRP S_IEXEC
#define S_IRWXU S_IRUSR|S_IWUSR|S_IXUSR
#define S_IRWXG S_IRGRP|S_IWGRP|S_IXGRP
#define S_IRWXO S_IROTH|S_IWOTH|S_IXOTH
#define S_IROTH S_IREAD
#define S_IRGRP S_IREAD
#define S_IWGRP S_IWRITE
#define S_IWOTH S_IWRITE
#define O_CLOEXEC 0
#define O_DIRECTORY _O_OBTAIN_DIR

enum {
    S_IFSOCK = 1,
    S_IFLNK,
    S_IFBLK,
    S_IFIFO,
    S_ISUID,
    S_ISGID,
    S_ISVTX
};

#define EBADFD 200
#define ESHUTDOWN 201
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

#define MSG_NOSIGNAL 0

#define access _access

#define F_GETFL 0
#define F_SETFL 0
#define O_NONBLOCK 0
#define O_SYNC 0
#define O_NOCTTY 0
