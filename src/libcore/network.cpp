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

#include <basecode/core/format.h>
#include <basecode/core/network.h>

namespace basecode::network {
    static s32 recv(socket_t& sock, u0* buf, s32 len, s32 timeout) {
        struct pollfd fd{};
        fd.fd     = sock.socket;
        fd.events = POLLIN;
        return poll(&fd, 1, timeout) > 0 ?
               ::recv(sock.socket, (s8*) buf, len, 0) : -1;
    }

    static s32 recv_buffered(socket_t& sock, u0* buf, s32 len, s32 timeout) {
        if (len <= sock.buf_free) {
            std::memcpy(buf, sock.buf_cur, len);
            sock.buf_cur += len;
            sock.buf_free -= len;
            return len;
        }

        if (sock.buf_free > 0) {
            std::memcpy(buf, sock.buf_cur, sock.buf_free);
            const auto read_len = sock.buf_free;
            sock.buf_free = 0;
            return read_len;
        }

        if (len >= sock.buf_size)
            return recv(sock, buf, len, timeout);

        sock.buf_free = recv(sock, sock.buf, sock.buf_size, timeout);
        if (sock.buf_free <= 0)
            return sock.buf_free;

        const auto size = std::min<u32>(len, sock.buf_free);
        std::memcpy(buf, sock.buf, size);
        sock.buf_cur = sock.buf + size;
        sock.buf_free -= size;
        return size;
    }

    namespace system {
        u0 fini() {
#ifdef _WIN32
            WSACleanup();
#endif
        }

        status_t init() {
#ifdef _WIN32
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
                return status_t::winsock_init_failure;
#endif
            return status_t::ok;
        }
    }

    namespace socket {
        u0 free(socket_t& sock) {
            socket::close(sock);
            memory::free(sock.alloc, sock.buf);
            sock.buf_cur  = sock.buf      = {};
            sock.buf_size = sock.buf_free = {};
        }

        u0 init(socket_t& sock,
                u32 buf_size,
                socket_close_callback_t close_cb,
                alloc_t* alloc) {
            sock.socket   = INVALID_SOCKET;
            sock.user     = {};
            sock.alloc    = alloc;
            sock.close_cb = close_cb;
            sock.buf_cur  = sock.buf      = (u8*) memory::alloc(sock.alloc,
                                                                buf_size);
            sock.buf_size = sock.buf_free = buf_size;
        }

        b8 has_data(socket_t& sock) {
            if (sock.buf_free > 0)
                return true;
            struct pollfd fd{};
            fd.fd     = sock.socket;
            fd.events = POLLIN;
            return poll(&fd, 1, 0) > 0;
        }

        status_t close(socket_t& sock) {
            if (sock.socket == INVALID_SOCKET)
                return status_t::socket_already_closed;
            ::close(sock.socket);
            sock.socket = INVALID_SOCKET;
            if (sock.close_cb)
                sock.close_cb(sock);
            return status_t::ok;
        }

        s32 send_buf_size(socket_t& sock) {
            s32 buf_size;
            socklen_t sz = sizeof(buf_size);
            getsockopt(sock.socket,
                       SOL_SOCKET,
                       SO_SNDBUF,
                       (s8*) &buf_size,
                       &sz);
            return buf_size;
        }
    }

    namespace udp {
        status_t listen(socket_t& sock, u16 port) {
            if (sock.socket != INVALID_SOCKET)
                return status_t::socket_already_open;

            auto socket = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (socket == INVALID_SOCKET)
                return status_t::socket_dgram_error;

#if defined __APPLE__
            s32 val = 1;
            setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
#endif
            s32 result;
            s32 reuse = 1;
            s32 broadcast = 1;
            setsockopt(socket,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       (const s8*) &reuse,
                       sizeof(reuse));
            result = setsockopt(socket,
                                SOL_SOCKET,
                                SO_BROADCAST,
                                (const s8*) &broadcast,
                                sizeof(broadcast));
            if (result == -1) {
                ::close(socket);
                return status_t::socket_option_broadcast_error;
            }

            struct sockaddr_in addr{};
            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(socket, (sockaddr*) &addr, sizeof(addr)) == -1) {
                ::close(socket);
                return status_t::bind_failure;
            }

            sock.socket = socket;
            return status_t::ok;
        }

        s32 send(socket_t& sock, u16 port, const u0* data, u32 len) {
            struct sockaddr_in addr{};
            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(port);
            addr.sin_addr.s_addr = INADDR_BROADCAST;
            return (s32) sendto(sock.socket,
                                (const s8*) data,
                                len,
                                MSG_NOSIGNAL,
                                (sockaddr*) &addr,
                                sizeof(addr));
        }

        u8* read(socket_t& sock, ssize& len, ip_address_t& addr, s32 timeout) {
            struct pollfd fd{};
            fd.fd     = sock.socket;
            fd.events = POLLIN;
            if (poll(&fd, 1, timeout) <= 0)
                return nullptr;
            struct sockaddr sa{};
            socklen_t sa_len = sizeof(struct sockaddr);
            len = recvfrom(sock.socket,
                           (s8*) sock.buf,
                           sock.buf_size,
                           0,
                           &sa,
                           &sa_len);
            ip_address::set(addr, &sa);
            return sock.buf;
        }

    }

    namespace tcp {
        b8 read(socket_t& sock,
                u0* buf,
                s32 len,
                s32 timeout,
                socket_read_callback_t read_cb,
                u0* user) {
            auto buf_ptr = (s8*) buf;
            while (len > 0) {
                if (read_cb(sock, user ? user : sock.user)) return false;
                const auto sz = recv_buffered(sock, buf_ptr, len, timeout);
                switch (sz) {
                    case 0:  return false;
                    case -1: {
#ifdef _WIN32
                        auto err = WSAGetLastError();
                        if (err == WSAECONNABORTED || err == WSAECONNRESET)
                            return false;
#endif
                        break;
                    }
                    default:
                        len -= sz;
                        buf_ptr += sz;
                        break;
                }
            }
            return true;
        }

        s32 send(socket_t& sock, const u0* buf, s32 len) {
            auto buf_ptr   = sock.buf;
            auto buf_start = buf_ptr;
            while (len > 0) {
                auto write_len = ::send(sock.socket,
                                        (const s8*) buf,
                                        len,
                                        MSG_NOSIGNAL);
                if (write_len == -1) return -1;
                len -= write_len;
                buf_ptr += write_len;
            }
            return s32(buf_ptr - buf_start);
        }

        status_t listen(socket_t& sock, u16 port, s32 backlog) {
            if (sock.socket != INVALID_SOCKET)
                return status_t::socket_already_open;

            struct addrinfo* res;
            struct addrinfo hints{};

            memset(&hints, 0, sizeof(hints));
            hints.ai_family   = AF_INET6;
            hints.ai_flags    = AI_PASSIVE;
            hints.ai_socktype = SOCK_STREAM;

            auto port_str = format::format("{}", port);
            if (getaddrinfo(nullptr, str::c_str(port_str), &hints, &res) != 0)
                return status_t::invalid_address_and_port;

            sock.socket = ::socket(res->ai_family,
                                   res->ai_socktype,
                                   res->ai_protocol);
            s32 val = 1;
            setsockopt(sock.socket,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       (const s8*) &val,
                       sizeof(val));
            if (bind(sock.socket, res->ai_addr, res->ai_addrlen) == -1)
                return status_t::bind_failure;

            if (::listen(sock.socket, backlog) == -1)
                return status_t::listen_failure;

            return status_t::ok;
        }

        b8 read_raw(socket_t& sock, u0* buf, s32 len, s32 timeout) {
            auto buf_ptr = (s8*) buf;
            while (len > 0) {
                const auto sz = recv(sock, buf_ptr, len, timeout);
                if (sz <= 0) return false;
                len -= sz;
                buf_ptr += sz;
            }
            return true;
        }

        status_t connect(socket_t& sock, str::slice_t addr, u16 port) {
            if (sock.socket != INVALID_SOCKET)
                return status_t::socket_already_open;

            struct addrinfo hints{};
            struct addrinfo* res;
            struct addrinfo* ptr;

            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family   = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;

            str_t addr_str(addr);
            auto port_str = format::format("{}", port);
            if (getaddrinfo(str::c_str(addr_str),
                            str::c_str(port_str),
                            &hints, &res) != 0) {
                return status_t::invalid_address_and_port;
            }

            s32 socket;
            for (ptr = res; ptr; ptr = ptr->ai_next) {
                socket = ::socket(ptr->ai_family,
                                  ptr->ai_socktype,
                                  ptr->ai_protocol);
                if (socket == INVALID_SOCKET) continue;
#ifdef __APPLE__
                s32 val = 1;
                setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
#endif
                if (::connect(socket, ptr->ai_addr, ptr->ai_addrlen) == -1) {
                    ::close(socket);
                    continue;
                }
                break;
            }

            freeaddrinfo(res);
            if (!ptr)
                return status_t::connect_failure;

            sock.socket = socket;
            return status_t::ok;
        }

        b8 accept(socket_t& listen_sock, socket_t& client_sock, s32 timeout) {
            struct sockaddr_storage remote{};
            socklen_t sz = sizeof(remote);

            struct pollfd fd{};
            fd.fd     = listen_sock.socket;
            fd.events = POLLIN;

            if (poll(&fd, 1, timeout) > 0) {
                auto socket = ::accept(listen_sock.socket,
                                       (sockaddr*) &remote,
                                       &sz);
                if (socket == INVALID_SOCKET)
                    return false;
#if defined __APPLE__
                s32 val = 1;
                setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
#endif
                client_sock.socket = socket;
                return true;
            }
            return false;
        }
    }

    namespace ip_address {
        u0 set(ip_address_t& ip, const struct sockaddr* addr) {
            auto ai = (const struct sockaddr_in*) addr;
            inet_ntop(AF_INET, &ai->sin_addr, ip.text, 17);
            ip.number = ai->sin_addr.s_addr;
        }

        u0 init(ip_address_t& ip, const struct sockaddr* addr) {
            if (addr) {
                set(ip, addr);
            } else {
                ip.text[0] = '\0';
            }
        }
    }
}
