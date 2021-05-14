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
#include <basecode/core/gap_buf.h>

namespace basecode::gap_buf {
    static b8 copy(gap_buf_t& buf, u32 dest, u32 src, u32 length) {
        if (dest == src || !length) return true;
        if (src > dest) {
            if (src + length >= buf.size) return false;
            auto d = buf.data + dest;
            auto s = buf.data + src;
            for (; length > 0; length--) *(d++) = *(s++);
        } else {
            src += length;
            dest += length;
            auto d = buf.data + dest;
            auto s = buf.data + src;
            for (; length > 0; length--) *(--d) = *(--s);
        }
        return true;
    }

    u0 free(gap_buf_t& buf) {
        memory::free(buf.alloc, buf.data);
        buf.data      = {};
        buf.size      = {};
        buf.gap.start = buf.gap.end = {};
        buf.gap_size  = buf.caret = buf.capacity = {};
    }

    u8 caret_curr(gap_buf_t& buf) {
        if (buf.caret == buf.gap.start) buf.caret = buf.gap.end;
        return buf.data[buf.caret];
    }

    u8 caret_next(gap_buf_t& buf) {
        if (buf.caret == buf.gap.start) {
            buf.caret = buf.gap.end;
            return buf.data[buf.caret];
        }
        return buf.data[++buf.caret];
    }

    u8 caret_prev(gap_buf_t& buf) {
        if (buf.caret == buf.gap.end)
            buf.caret = buf.gap.start;
        return buf.data[--buf.caret];
    }

    u0 print(const gap_buf_t& buf) {
        u32 i{};
        const auto size = buf.size + gap_size(buf);
        while (i < size) {
            if (i >= buf.gap.start && i < buf.gap.end) {
                format::print("_");
            } else {
                format::print("{}", (s8) buf.data[i]);
            }
            ++i;
        }
    }

    u0 gap_to_caret(gap_buf_t& buf) {
        if (buf.caret == buf.gap.start) return;
        if (buf.caret == buf.gap.end) {
            buf.caret = buf.gap.start;
            return;
        }

        if (buf.caret < buf.gap.start) {
            copy(buf,
                 buf.caret + (buf.gap.end - buf.gap.start),
                 buf.caret,
                 buf.gap.start - buf.caret);
            buf.gap.end -= (buf.gap.start - buf.caret);
            buf.gap.start = buf.caret;
        } else {
            copy(buf,
                 buf.gap.start,
                 buf.gap.end,
                 buf.caret - buf.gap.end);
            buf.gap.start += buf.caret - buf.gap.end;
            buf.gap.end = buf.caret;
            buf.caret   = buf.gap.start;
        }
    }

    u32 caret_offset(gap_buf_t& buf) {
        return buf.caret > buf.gap.end ? buf.caret - gap_size(buf) : buf.caret;
    }

    u32 gap_size(const gap_buf_t& buf) {
        return buf.gap.end - buf.gap.start;
    }

    u0 caret_put(gap_buf_t& buf, u8 value) {
        caret_insert(buf, value);
        buf.caret++;
    }

    u0 caret_delete(gap_buf_t& buf, u32 size) {
        if (buf.caret != buf.gap.start) gap_to_caret(buf);
        buf.gap.end += size;
        buf.size -= size;
    }

    u0 caret_move(gap_buf_t& buf, u32 offset) {
        buf.caret = offset;
        if (buf.caret > buf.gap.start)
            buf.caret += gap_size(buf);
    }

    u0 grow(gap_buf_t& buf, u32 new_capacity) {
        reserve(buf, buf.size + new_capacity + buf.gap_size);
    }

    u0 caret_replace(gap_buf_t& buf, u8 value) {
        if (buf.caret == buf.gap.start) buf.caret = buf.gap.end;
        if (buf.caret == buf.capacity) grow(buf, 1);
        buf.data[buf.caret] = value;
    }

    u0 gap_expand(gap_buf_t& buf, u32 new_size) {
        if (new_size <= gap_size(buf)) return;
        grow(buf, new_size);
        if (buf.size > 0) {
            copy(buf,
                 buf.gap.end + new_size,
                 buf.gap.end,
                 buf.size - buf.gap.end);
        }
        buf.gap.end += new_size;
    }

    u0 caret_insert(gap_buf_t& buf, u8 value) {
        if (buf.caret != buf.gap.start) gap_to_caret(buf);
        if (buf.gap.start == buf.gap.end) gap_expand(buf, 1);
        buf.data[buf.gap.start++] = value;
        ++buf.size;
    }

    u0 reserve(gap_buf_t& buf, u32 new_capacity) {
        if (new_capacity == 0) {
            memory::free(buf.alloc, buf.data);
            buf.data      = {};
            buf.size      = {};
            buf.gap.start = buf.gap.end = {};
            buf.gap_size  = buf.caret = buf.capacity = {};
            return;
        }

        if (new_capacity == buf.capacity) return;
        buf.data = (u8*) memory::realloc(buf.alloc, buf.data, new_capacity);
        buf.capacity = new_capacity;
    }

    status_t init(gap_buf_t& buf, u32 gap_size, alloc_t* alloc) {
        buf.data      = {};
        buf.size      = {};
        buf.gap.start = {};
        buf.alloc     = alloc;
        buf.gap_size  = gap_size;
        buf.gap.end   = buf.gap_size;
        buf.caret     = buf.capacity = {};
        return status_t::ok;
    }
}
