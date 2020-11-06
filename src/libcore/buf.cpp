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

#include <sys/mman.h>
#ifndef _WIN32
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif
#include <basecode/core/buf.h>
#include <basecode/core/filesys.h>

namespace basecode::buf {
    static u0 grow(buf_t& buf, u32 new_capacity = 0) {
        new_capacity = std::max(new_capacity, buf.capacity);
        reserve(buf, new_capacity * 2 + 8);
    }

    namespace cursor {
        u0 pop(buf_crsr_t& crsr) {
            crsr.pos = stack::pop(crsr.stack);
        }

        u0 push(buf_crsr_t& crsr) {
            stack::push(crsr.stack, crsr.pos);
        }

        u0 free(buf_crsr_t& crsr) {
            stack::free(crsr.stack);
        }

        u0 seek(buf_crsr_t& crsr, u32 offset) {
            crsr.pos = offset;
        }

        u0 init(buf_crsr_t& crsr, buf_t& buf) {
            crsr.buf = &buf;
            crsr.pos = crsr.line = crsr.column = {};
            stack::init(crsr.stack, buf.alloc);
        }

        u0 write_u8(buf_crsr_t& crsr, u8 value) {
            write(*crsr.buf, crsr.pos, (const u8*) &value, sizeof(u8));
            crsr.pos += sizeof(u8);
        }

        u0 write_u16(buf_crsr_t& crsr, u16 value) {
            write(*crsr.buf, crsr.pos, (const u8*) &value, sizeof(u16));
            crsr.pos += sizeof(u16);
        }

        u0 write_s16(buf_crsr_t& crsr, s16 value) {
            write(*crsr.buf, crsr.pos, (const u8*) &value, sizeof(s16));
            crsr.pos += sizeof(s16);
        }

        u0 write_u32(buf_crsr_t& crsr, u32 value) {
            write(*crsr.buf, crsr.pos, (const u8*) &value, sizeof(u32));
            crsr.pos += sizeof(u32);
        }

        u0 seek_fwd(buf_crsr_t& crsr, u32 offset) {
            crsr.pos += offset;
        }

        u0 seek_rev(buf_crsr_t& crsr, u32 offset) {
            crsr.pos -= offset;
        }

        u0 write_u64(buf_crsr_t& crsr, u64 value) {
            write(*crsr.buf, crsr.pos, (const u8*) &value, sizeof(u64));
            crsr.pos += sizeof(u64);
        }

        status_t read_u8(buf_crsr_t& crsr, u8& value) {
            read(*crsr.buf, crsr.pos, &value, sizeof(u8));
            crsr.pos += sizeof(u8);
            return status_t::ok;
        }

        status_t read_u16(buf_crsr_t& crsr, u16& value) {
            read(*crsr.buf, crsr.pos, &value, sizeof(u16));
            crsr.pos += sizeof(u16);
            return status_t::ok;
        }

        status_t read_u32(buf_crsr_t& crsr, u32& value) {
            read(*crsr.buf, crsr.pos, &value, sizeof(u32));
            crsr.pos += sizeof(u32);
            return status_t::ok;
        }

        status_t read_u64(buf_crsr_t& crsr, u64& value) {
            read(*crsr.buf, crsr.pos, &value, sizeof(u64));
            crsr.pos += sizeof(u64);
            return status_t::ok;
        }

        status_t read_obj(buf_crsr_t& crsr, u0* data, u32 length) {
            if (crsr.pos + length > crsr.buf->length)
                return status_t::end_of_buffer;
            read(*crsr.buf, crsr.pos, data, length);
            crsr.pos += length;
            return status_t::ok;
        }
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

    status_t free(buf_t& buf) {
        if (buf.mode == buf_mode_t::mapped) {
            auto status = buf::unmap(buf);
            if (!OK(status))
                return status;
        } else {
            memory::free(buf.alloc, buf.data);
            buf.data = {};
            buf.length = buf.capacity = {};
        }
        path::free(buf.path);
        array::free(buf.lines);
        array::free(buf.tokens);
        return status_t::ok;
    }

    buf_t make(alloc_t* alloc) {
        buf_t buf{};
        init(buf, alloc);
        return buf;
    }

    status_t reset(buf_t& buf) {
        if (buf.mode == buf_mode_t::mapped)
            return status_t::cannot_reset_mapped_buf;
        buf.length = {};
        path::reset(buf.path);
        return status_t::ok;
    }

    status_t unmap(buf_t& buf) {
        if (buf.mode != buf_mode_t::mapped)
            return status_t::cannot_unmap_buf;

        s32 rc{};
        if (buf.file && buf.data) {
            rc = munmap(buf.data, buf.length);
            close(buf.file);
            buf.data   = {};
            buf.file   = 0;
            buf.length = {};
            buf.mode = buf_mode_t::none;
            path::reset(buf.path);
        }

        return rc == 0 ? status_t::ok : status_t::munmap_error;
    }

    u0 init(buf_t& buf, alloc_t* alloc) {
        buf.data   = {};
        buf.file   = {};
        buf.mode   = {};
        buf.alloc  = alloc;
        buf.length = buf.capacity = {};
        path::init(buf.path, buf.alloc);
        array::init(buf.lines, buf.alloc);
        array::init(buf.tokens, buf.alloc);
    }

    str::slice_t line(buf_t& buf, u32 idx) {
        if (buf.lines.size == 0 || idx > buf.lines.size - 1)
            return {};
        const auto& line = buf.lines[idx];
        return slice::make(buf.data + line.pos, line.len);
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

    status_t map(buf_t& buf, const path_t& path) {
        if (buf.mode == buf_mode_t::mapped)
            return status_t::buf_already_mapped;

        usize file_size{};
        if (!OK(filesys::file_size(path, file_size)))
            return status_t::unable_to_open_file;

        mode_t mode = S_IRUSR | S_IWUSR;
#ifdef _WIN32
        s32 flags = O_BINARY | O_RDWR;
#else
        s32 flags = O_RDWR;
#endif
        buf.file = open(path::c_str(path), flags, mode);
        if (buf.file == -1)
            return status_t::unable_to_open_file;

        auto map = mmap(nullptr,
                        file_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        buf.file,
                        0);
        if (map == MAP_FAILED) {
            close(buf.file);
            buf.file = 0;
            return status_t::mmap_error;
        }

        buf.mode   = buf_mode_t::mapped;
        buf.data   = (u8*) map;
        buf.length = file_size;
        path::set(buf.path, path);

        return status_t::ok;
    }

    status_t load(buf_t& buf, const path_t& path) {
        if (buf.mode == buf_mode_t::mapped)
            return status_t::buf_already_mapped;

        usize file_size{};
        if (!OK(filesys::file_size(path, file_size)))
            return status_t::unable_to_open_file;

        auto file = fopen(path::c_str(path), "rb");
        if (!file)
            return status_t::unable_to_open_file;

        defer(fclose(file));

        buf.mode = buf_mode_t::alloc;
        path::set(buf.path, path);

        write(buf, 0, file, file_size);

        return status_t::ok;
    }

    u0 zero_fill(buf_t& buf, u32 offset, u32 length) {
        std::memset(buf.data + offset, 0, length);
    }

    u0 read(buf_t& buf, u32 offset, u0* data, u32 length) {
        std::memcpy(data, buf.data + offset, length);
    }

    u0 write(buf_t& buf, u32 offset, FILE* file, u32 length) {
        if (buf.mode == buf_mode_t::alloc
        && (offset + length + 1 > buf.capacity)) {
            grow(buf, offset + length + 1);
        }
        fread(buf.data + offset, 1, length, file);
        buf.length += offset + length > buf.length ? offset + length : length;
    }

    u0 write(buf_t& buf, u32 offset, const u8* data, u32 length) {
        if (buf.mode == buf_mode_t::alloc
        && (offset + length + 1 > buf.capacity)) {
            grow(buf, offset + length + 1);
        }
        std::memcpy(buf.data + offset, data, length);
        buf.length += offset + length > buf.length ? offset + length : length;
    }

    status_t save(buf_t& buf, const path_t& path, u32 offset, u32 length) {
        if (buf.mode == buf_mode_t::mapped
        &&  buf.path == path) {
            return status_t::cannot_save_over_mapped_path;
        }
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
