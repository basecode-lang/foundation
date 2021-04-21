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

#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <basecode/core/types.h>
#include <basecode/core/memory.h>

#ifndef INVALID_SOCKET
#   define INVALID_SOCKET   (-1)
#   define SOCKET           s32
#endif
#ifndef MSG_NOSIGNAL
#  define MSG_NOSIGNAL 0
#endif

namespace basecode {
    struct ip_address_t final {
        u32                     number;
        s8                      text[17];
    };

    struct socket_t;

    struct socket_t final {
        alloc_t*                alloc;
        u8*                     buf;
        u8*                     buf_cur;
        u0*                     user;
        socket_close_callback_t close_cb;
        SOCKET                  socket;
        u32                     buf_free;
        u32                     buf_size;
    };

    namespace network {
        namespace udp {
            status_t listen(socket_t& sock, u16 port);

            s32 send(socket_t& sock, u16 port, const u0* data, u32 len);

            u8* read(socket_t& sock, ssize& len, ip_address_t& addr, s32 timeout = 10);
        }

        namespace tcp {
            b8 read(socket_t& sock,
                    u0* buf,
                    s32 len,
                    s32 timeout,
                    socket_read_callback_t read_cb,
                    u0* user = {});

            s32 send(socket_t& sock, const u0* buf, s32 len);

            status_t listen(socket_t& sock, u16 port, s32 backlog);

            b8 read_raw(socket_t& sock, u0* buf, s32 len, s32 timeout);

            status_t connect(socket_t& sock, str::slice_t addr, u16 port);

            b8 accept(socket_t& listen_sock, socket_t& client_sock, s32 timeout = 10);
        }

        namespace system {
            u0 fini();

            status_t init();
        }

        namespace socket {
            u0 init(socket_t& sock,
                    u32 buf_size = 2 * 1024,
                    socket_close_callback_t close_cb = {},
                    alloc_t* alloc = context::top()->alloc.main);

            u0 free(socket_t& sock);

            b8 has_data(socket_t& sock);

            status_t close(socket_t& sock);

            s32 send_buf_size(socket_t& sock);
        }

        namespace ip_address {
            u0 set(ip_address_t& ip, const struct sockaddr* addr);

            u0 init(ip_address_t& ip, const struct sockaddr* addr = {});
        }
    }
}
