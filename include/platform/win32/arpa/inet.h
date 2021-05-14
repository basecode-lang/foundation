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

#include <winsock2.h>
#include <ws2tcpip.h>

typedef unsigned in_addr_t;

inline in_addr_t uni_inet_addr(const char* ip) {
    in_addr_t out;
    if (inet_pton(AF_INET, ip, &out) <= 0)
        return INADDR_NONE;
    return out;
}

#define inet_addr uni_inet_addr
