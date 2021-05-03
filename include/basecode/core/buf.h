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

#include <basecode/core/defer.h>
#include <basecode/core/context.h>

#define CRSR_POS(c)             ((c).pos)
#define CRSR_END(c)             ((c).buf->length - 1)
#define CRSR_PTR(c)             (*(c))
#define CRSR_MORE(c)            ((c).pos < (c).buf->length)
#define CRSR_PREV(c)            SAFE_SCOPE(                 \
        if ((c).pos >= 0) {                                 \
            --(c).pos;                                      \
            --(c).column;                                   \
        })
#define CRSR_NEXT(c)            SAFE_SCOPE(                 \
        if (CRSR_MORE(c)) {                                 \
            ++(c).pos;                                      \
            ++(c).column;                                   \
        })
#define CRSR_READ(c)            ((c).pos == (c).buf->length ? 0 : (c)[(c).pos])
#define CRSR_PEEK(c)            ((c).pos < (c).buf->length ? \
                                 (c)[(c).pos + 1] : 0)
#define CRSR_NEWLINE(c)         SAFE_SCOPE((c).column = 0; ++(c).line;)

#define FILE_ALIAS(f)           auto& file = f
#define FILE_EOF()              (!CRSR_MORE(file.crsr))
#define FILE_POS()              CRSR_POS(file.crsr)
#define FILE_PTR()              CRSR_PTR(file.crsr)
#define FILE_NEXT()             CRSR_NEXT(file.crsr)
#define FILE_PEEK(o)            (file.crsr[file.crsr.pos + (o)])
#define FILE_PTR_OFFS(o)        (file.crsr.buf->data + (o))
#define FILE_POP_POS()          SAFE_SCOPE(buf::cursor::pop(file.crsr);)
#define FILE_PUSH_POS()         SAFE_SCOPE(buf::cursor::push(file.crsr);)
#define FILE_SEEK(p)            SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek(file.crsr, (p))))             \
        return status_t::read_error;)
#define FILE_SEEK_FWD(p)        SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek_fwd(file.crsr, (p))))         \
        return status_t::read_error;)
#define FILE_SEEK_RWD(p)        SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek_rwd(file.crsr, (p))))         \
        return status_t::read_error;)
#define FILE_WRITE(t, v)        SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);          \
    if (!OK(buf::cursor::write(file.crsr, (v))))            \
        return status_t::write_error;)
#define FILE_READ(t, v)         SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);          \
    if (!OK(buf::cursor::read(file.crsr, (v))))             \
        return status_t::read_error;)
#define FILE_READ_STR(d, l)     SAFE_SCOPE(                 \
    if (!OK(buf::cursor::read_str(file.crsr, (d), (l))))    \
        return status_t::read_error;)
#define FILE_WRITE_STR(v)       SAFE_SCOPE(                 \
    if (!OK(buf::cursor::write_str(file.crsr, (v))))        \
        return status_t::write_error;)
#define FILE_WRITE_CSTR(v)      SAFE_SCOPE(                 \
    if (!OK(buf::cursor::write_cstr(file.crsr, (v))))       \
        return status_t::write_error;)
#define FILE_WRITE_PAD(l)       SAFE_SCOPE(                 \
    if (!OK(buf::cursor::zero_fill(file.crsr, (l))))        \
        return status_t::write_error;)
#define FILE_WRITE0(T)          SAFE_SCOPE(                 \
    T zero{};                                               \
    if (!OK(buf::cursor::write(file.crsr, zero)))           \
        return status_t::write_error;)

namespace basecode::buf {
    status_t zero_fill(buf_t& buf, u32 offset, u32 length);

    status_t read(buf_t& buf, u32 offset, u0* data, u32 length);

    status_t write(buf_t& buf, u32 offset, FILE* file, u32 length);

    status_t write(buf_t& buf, u32 offset, const u8* data, u32 length);

    namespace cursor {
        u0 pop(buf_crsr_t& crsr);

        u0 push(buf_crsr_t& crsr);

        u0 free(buf_crsr_t& crsr);

        u0 init(buf_crsr_t& crsr, buf_t& buf);

        template <typename T> requires std::is_trivial_v<T>
        status_t read(buf_crsr_t& crsr, T& value) {
            auto status = buf::read(*crsr.buf,
                                    crsr.pos,
                                    (u0*) &value,
                                    sizeof(value));
            if (!OK(status))
                return status;
            crsr.pos += sizeof(value);
            return status_t::ok;
        }

        template <typename T> requires std::is_trivial_v<T>
        status_t write(buf_crsr_t& crsr, T value) {
            auto status = buf::write(*crsr.buf,
                                     crsr.pos,
                                     (u8*) &value,
                                     sizeof(value));
            if (!OK(status))
                return status;
            crsr.pos += sizeof(value);
            return status_t::ok;
        }

        status_t seek(buf_crsr_t& crsr, u32 offset);

        status_t seek_fwd(buf_crsr_t& crsr, u32 offset);

        status_t seek_rwd(buf_crsr_t& crsr, u32 offset);

        status_t zero_fill(buf_crsr_t& crsr, u32 length);

        template <String_Concept T>
        status_t write_str(buf_crsr_t& crsr, const T& str) {
            auto status = buf::write(*crsr.buf,
                                     crsr.pos,
                                     str.data,
                                     str.length);
            if (!OK(status))
                return status;
            crsr.pos += str.length;
            return status_t::ok;
        }

        status_t write_cstr(buf_crsr_t& crsr, str::slice_t slice);

        status_t read_str(buf_crsr_t& crsr, u0* dest, u32 length);
    }

    u0 index(buf_t& buf);

    status_t free(buf_t& buf);

    status_t reset(buf_t& buf);

    status_t save(buf_t& buf,
                  const path_t& path,
                  u32 offset = {},
                  u32 length = {});

    str::slice_t line(buf_t& buf, u32 idx);

    u0 reserve(buf_t& buf, u32 new_capacity);

    u0 grow(buf_t& buf, u32 new_capacity = 0);

    template <String_Concept T>
    status_t load(buf_t& buf, const T& value) {
        return load(buf, value.data, value.length);
    }

    status_t unmap(buf_t& buf, b8 sync = false);

    status_t load(buf_t& buf, const path_t& path);

    status_t load(buf_t& buf, const u8* data, u32 size);

    status_t map_existing(buf_t& buf, const path_t& path);

    buf_t make(alloc_t* alloc = context::top()->alloc.main);

    b8 each_line(const buf_t& buf, const line_callback_t& cb);

    status_t map_new(buf_t& buf, const path_t& path, usize size);

    u0 init(buf_t& buf, alloc_t* alloc = context::top()->alloc.main);
}

