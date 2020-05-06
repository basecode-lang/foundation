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

#include <basecode/core/base64.h>

namespace basecode::base64 {
    static const s8* s_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const s32 s_index[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 62, 62, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 63,
        0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    b8 encode(str::slice_t src, str_t& buf) {
        const auto length = (src.length + 2) / 3 * 4;

        str::resize(buf, length);
        std::memset(buf.data, '=', length);

        auto p = (s8*) src.data;
        auto buffer = buf.data;
        u32 j = 0, pad = src.length % 3;
        const u32 last = src.length - pad;

        for (u32 i = 0; i < last; i += 3) {
            auto n = s32(p[i]) << 16 | s32(p[i + 1]) << 8 | p[i + 2];
            buffer[j++] = s_chars[n >> 18];
            buffer[j++] = s_chars[n >> 12 & 0x3F];
            buffer[j++] = s_chars[n >> 6 & 0x3F];
            buffer[j++] = s_chars[n & 0x3F];
        }

        if (pad) {
            auto n = --pad ? s32(p[last]) << 8 | p[last + 1] : p[last];
            buffer[j++] = s_chars[pad ? n >> 10 & 0x3F : n >> 2];
            buffer[j++] = s_chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
            buffer[j++] = pad ? s_chars[n << 2 & 0x3F] : '=';
        }

        return true;
    }

    b8 decode(str::slice_t src, str_t& buf) {
        if (slice::empty(src))
            return false;

        auto p = (u8*) src.data;
        u32 j = 0,
             pad1 = src.length % 4 || p[src.length - 1] == '=',
             pad2 = pad1 && (src.length % 4 > 2 || p[src.length - 2] != '=');
        const u32 last = (src.length - pad1) / 4 << 2;
        str::resize(buf, last / 4 * 3 + pad1 + pad2);
        std::memset(buf.data, 0, buf.length);

        auto buffer = buf.data;

        for (u32 i = 0; i < last; i += 4) {
            auto n = s_index[p[i]] << 18 | s_index[p[i + 1]] << 12 | s_index[p[i + 2]] << 6 | s_index[p[i + 3]];
            buffer[j++] = n >> 16;
            buffer[j++] = n >> 8 & 0xFF;
            buffer[j++] = n & 0xFF;
        }

        if (pad1) {
            auto n = s_index[p[last]] << 18 | s_index[p[last + 1]] << 12;
            buffer[j++] = n >> 16;
            if (pad2) {
                n |= s_index[p[last + 2]] << 6;
                buffer[j++] = n >> 8 & 0xFF;
            }
        }

        return true;
    }

    b8 encode_url(str::slice_t src, str_t& buf) {
        encode(src, buf);
        for (u32 i = 0; i < buf.length; i++) {
            if (buf[i] == '+') buf[i] = '-';
            if (buf[i] == '/') buf[i] = '_';
            if (buf[i] == '=') {
                str::trunc(buf, i);
                break;
            }
        }
        return true;
    }

    b8 decode_url(str::slice_t src, str_t& buf) {
        str::resize(buf, src.length + 2);
        std::memcpy(buf.data, src.data, src.length);

        for (u32 i = 0; i < buf.length; i++) {
            if (buf[i] == '-') buf[i] = '+';
            if (buf[i] == '_') buf[i] = '/';
        }

        switch (buf.length % 4) {
            case 0:
                break;
            case 2:
                str::append(buf, "==");
                break;
            case 3:
                str::append(buf, "=");
                break;
            default:
                break;
        }

        return decode(slice::make(buf), buf);
    }
}
