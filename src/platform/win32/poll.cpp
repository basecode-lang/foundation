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

#include <poll.h>
#include <WinSock2.h>
#include <basecode/core/defer.h>

int poll(struct pollfd* fds, nfds_t nfds, int mille_timeout) {
    struct timeval timeout{};
    timeout.tv_sec  = mille_timeout / 1000;
    timeout.tv_usec = 1000000 * mille_timeout % 1000;
    auto* fd = (fd_set*) malloc(2 * nfds * sizeof(fd_set));
    if (!fd)
        return -1;
    defer(free(fd));
    u_int* const readerCount = &fd[0].fd_count;
    *readerCount = 0;
    SOCKET* fdReader = fd[0].fd_array;
    u_int* const writerCount = &fd[nfds].fd_count;
    *writerCount = 0;
    SOCKET* fdWriter = fd[nfds].fd_array;
    for (int i = 0; i < nfds; i++) {
        if (fds[i].events & POLLIN) {
            fdReader[*readerCount] = fds[i].fd;
            (*readerCount)++;
        }
        if (fds[i].events & POLLOUT) {
            fdWriter[*writerCount] = fds[i].fd;
            (*writerCount)++;
        }
    }
    fd_set   fdExcept;
    fdExcept.fd_count = 0;
    const int ok = select(nfds, &fd[0], &fd[nfds], &fdExcept, &timeout);
    return ok;
}

int ppoll(struct pollfd* fds, nfds_t nfds, const struct timespec* tmo_p, const sigset_t* sigmask) {
    return 0;
}
