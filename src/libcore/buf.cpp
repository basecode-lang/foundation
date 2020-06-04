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

#include <basecode/core/buf.h>

namespace basecode::buf {
    namespace cursor {
        buf_crsr_t make(buf_t& buf) {
            buf_crsr_t crsr{};
            init(crsr, buf);
            return crsr;
        }

        u0 init(buf_crsr_t& crsr, buf_t& buf) {
            crsr.buf = &buf;
            crsr.pos = crsr.line = crsr.column = {};
        }
    }

    static u0 grow(buf_t& buf, u32 new_capacity = 0) {
        new_capacity = std::max(new_capacity, buf.capacity);
        reserve(buf, new_capacity * 2 + 8);
    }

    u0 free(buf_t& buf) {
        array::free(buf.lines);
        array::free(buf.tokens);
        memory::free(buf.alloc, buf.data);
        buf.data = {};
        buf.length = buf.capacity = {};
    }

    u0 reset(buf_t& buf) {
        buf.length = {};
    }


    u0 index(buf_t& buf) {
        const __m128i cr = _mm_set1_epi8('\r');
        const __m128i lf = _mm_set1_epi8('\n');

        m128i_bytes_t   dqw         {};
        buf_line_t*     line;
        u32             start_pos   {};
        u32             end_pos;
        u32             mask;
        u32             i           {};

        array::reset(buf.lines);
        array::reserve(buf.lines, (buf.length / 80) * 3);

        while (i < buf.length) {
            dqw.value = _mm_loadu_si128((const __m128i*) (buf.data + i));
            mask = _mm_movemask_epi8(_mm_or_si128(_mm_cmpeq_epi8(dqw.value, cr), _mm_cmpeq_epi8(dqw.value, lf)));
            if (!mask) {
                i += 16;
            } else {
                while (mask) {
                    u32 bit = __builtin_ffs(mask) - 1;
                    end_pos = i + bit;
                    mask &= ~(u32(1) << bit);
                    if (dqw.bytes[bit] == '\r') {
                        dqw.bytes[bit] = ' ';
                    } else {
                        line = &array::append(buf.lines);
                        line->pos = start_pos;
                        line->len = end_pos - start_pos;
                    }
                    start_pos = end_pos + 1;
                }
                _mm_storeu_si128((__m128i*) (buf.data + i), dqw.value);
                i = start_pos;
            }
        }
    }

    buf_t make(alloc_t* alloc) {
        buf_t buf{};
        init(buf, alloc);
        return buf;
    }

    u0 init(buf_t& buf, alloc_t* alloc) {
        buf.data   = {};
        buf.alloc  = alloc;
        buf.length = buf.capacity = {};
        array::init(buf.lines, buf.alloc);
        array::init(buf.tokens, buf.alloc);
    }

    u0 reserve(buf_t& buf, u32 new_capacity) {
        if (new_capacity == 0) {
            memory::free(buf.alloc, buf.data);
            buf.data     = {};
            buf.capacity = buf.length = {};
            return;
        }

        if (new_capacity == buf.capacity)
            return;

        new_capacity = std::max(buf.length, new_capacity);
        new_capacity = new_capacity + (-new_capacity & 15U);
        buf.data = (u8*) memory::realloc(buf.alloc, buf.data, new_capacity, 16);
        buf.capacity = new_capacity;
    }

    status_t load(buf_t& buf, const path_t& path) {
        auto file = fopen(str::c_str(const_cast<str_t&>(path.str)), "rb");
        if (!file)
            return status_t::unable_to_open_file;
        defer(fclose(file));
        fseek(file, 0, SEEK_END);
        const auto length = ftell(file) + 1;
        fseek(file, 0, SEEK_SET);
        write(buf, 0, file, length);
        return status_t::ok;
    }

    u0 write(buf_t& buf, u32 offset, FILE* file, u32 length) {
        if (offset + length + 1 > buf.capacity)
            grow(buf, offset + length + 1);
        fread(buf.data + offset, 1, length, file);
        buf.length += offset + length > buf.length ? offset + length : length;
        buf.data[buf.length] = 255;
    }

    u0 write(buf_t& buf, u32 offset, const u8* data, u32 length) {
        if (offset + length + 1 > buf.capacity)
            grow(buf, offset + length + 1);
        std::memcpy(buf.data + offset, data, length);
        buf.length += offset + length > buf.length ? offset + length : length;
        buf.data[buf.length] = 255;
    }

    status_t save(buf_t& buf, const path_t& path, u32 offset, u32 length) {
        auto file = fopen(str::c_str(const_cast<str_t&>(path.str)), "wb");
        if (!file)
            return status_t::unable_to_open_file;
        defer(fclose(file));
        length = !length ? buf.length : length;
        fwrite(buf.data + offset, 1, length, file);
        fflush(file);
        return status_t::ok;
    }

    b8 each_line(const buf_t& buf, const line_callback_t& cb, str::slice_t sep) {
        u32 i{}, start_pos{};
        while (i < buf.length) {
            if (std::memcmp(buf.data + i, sep.data, sep.length) == 0) {
                str::slice_t line;
                line.data = buf.data + start_pos;
                line.length = (i - start_pos) - 1;
                if (!cb(line))
                    return false;
                start_pos = i + sep.length;
            }
            ++i;
        }
        return true;
    }
}
