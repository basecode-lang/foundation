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

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <Mstcpip.h>
#include <Ws2tcpip.h>
#include <cstdint>

typedef uint32_t sa_family_t;

struct iovec {
    int             junk;
};

struct msghdr {
    void*           msg_name;
    socklen_t       msg_namelen;
    struct iovec*   msg_iov;
    int             msg_iovlen;
    void*           msg_control;
    socklen_t       msg_controllen;
    int             msg_flags;
};

typedef int caddr_t;
