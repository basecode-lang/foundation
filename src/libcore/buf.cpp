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

#include <sys/mman.h>
#ifndef _WIN32
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#endif
#include <basecode/core/buf.h>
#include <basecode/core/stack.h>
#include <basecode/core/filesys.h>

namespace basecode::buf {
    namespace cursor {
        u0 pop(buf_crsr_t& crsr) {
            if (!stack::empty(crsr.stack))
                crsr.pos = stack::pop(crsr.stack);
        }

        u0 push(buf_crsr_t& crsr) {
            stack::push(crsr.stack, crsr.pos);
        }

        u0 free(buf_crsr_t& crsr) {
            stack::free(crsr.stack);
        }

        u0 init(buf_crsr_t& crsr, buf_t& buf) {
            crsr.buf = &buf;
            crsr.pos = crsr.line = crsr.column = {};
            stack::init(crsr.stack, buf.alloc);
        }

        status_t seek(buf_crsr_t& crsr, u32 offset) {
            crsr.pos = offset;
            return status_t::ok;
        }

        status_t seek_fwd(buf_crsr_t& crsr, u32 offset) {
            crsr.pos += offset;
            return status_t::ok;
        }

        status_t seek_rwd(buf_crsr_t& crsr, u32 offset) {
            crsr.pos -= offset;
            return status_t::ok;
        }

        status_t zero_fill(buf_crsr_t& crsr, u32 length) {
            auto status = buf::zero_fill(*crsr.buf, crsr.pos, length);
            if (!OK(status))
                return status;
            crsr.pos += length;
            return status_t::ok;
        }

        status_t read_str(buf_crsr_t& crsr, u0* dest, u32 length) {
            auto status = buf::read(*crsr.buf, crsr.pos, dest, length);
            if (!OK(status))
                return status;
            crsr.pos += length;
            return status_t::ok;
        }

        status_t write_cstr(buf_crsr_t& crsr, str::slice_t slice) {
            auto status = buf::write(*crsr.buf,
                                     crsr.pos,
                                     slice.data,
                                     slice.length);
            if (!OK(status))
                return status;
            crsr.pos += slice.length;
            const u8 zero{};
            status = buf::write(*crsr.buf,
                                crsr.pos,
                                (const u8*) &zero,
                                sizeof(u8));
            if (!OK(status))
                return status;
            crsr.pos += sizeof(u8);
            return status_t::ok;
        }
    }

    u0 index(buf_t& buf) {
#if defined(__AVX2__)
        const __m256i lf        = _mm256_set1_epi8('\n');
        const u32     lane_size = 32;
#elif defined(__SSE4_2__)
        const __m128i lf        = _mm_set1_epi8('\n');
        const u32     lane_size = 16;
#endif

        array::reset(buf.lines);
        array::reserve(buf.lines, (buf.length / 80) * 3);

        u32 start{};

        for (u32 idx = 0; idx < buf.length; idx += lane_size) {
            u8* p           = buf.data + idx;
#if defined(__AVX2__)
            __m256i value   = _mm256_load_si256((const __m256i*) p);
            __m256i matched = _mm256_cmpeq_epi8(value, lf);
            u32     mask    = _mm256_movemask_epi8(matched);
#elif defined(__SSE4_2__)
            __m128i value   = _mm_load_si128((const __m128i*) p);
            __m128i matched = _mm_cmpeq_epi8(value, lf);
            u32     mask    = _mm_movemask_epi8(matched);
#endif

            while (mask) {
                u32 bit = __builtin_ffs(mask) - 1;
                mask &= ~(1U << bit);
                u32 end = idx + bit;
                s32 len = end - start;
                if (len == 0 && buf[start] == '\n')
                    len = 1;
                if (buf[start + len - 1] == '\r')
                    --len;
                if (len > 0) {
                    auto& line = array::append(buf.lines);
                    line.pos = start;
                    line.len = len;
                }
                start = end + 1;
            }
        }
    }

    status_t free(buf_t& buf) {
        status_t status{};
        if (buf.mode == buf_mode_t::mapped) {
            status = buf::unmap(buf);
        } else if (buf.mode == buf_mode_t::alloc) {
            memory::free(buf.alloc, buf.data);
            buf.data = {};
            buf.length = buf.capacity = {};
        }
        path::free(buf.path);
        array::free(buf.lines);
        array::free(buf.tokens);
        return status;
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

    status_t unmap(buf_t& buf, b8 sync) {
        if (buf.mode != buf_mode_t::mapped)
            return status_t::cannot_unmap_buf;

        s32 rc{};
        if (buf.file && buf.data) {
            if (sync) {
                rc = msync(buf.data, buf.length, MS_SYNC);
            }
            if (rc == 0)
                rc = munmap(buf.data, buf.length);
            close(buf.file);
            buf.data   = {};
            buf.file   = 0;
            buf.length = {};
            buf.mode   = buf_mode_t::none;
            path::reset(buf.path);
        }

        return rc == 0 ? status_t::ok : status_t::munmap_error;
    }

    u0 grow(buf_t& buf, u32 new_capacity) {
        new_capacity = std::max(new_capacity, buf.capacity);
        reserve(buf, new_capacity * 2 + 8);
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
#if defined(__AVX2__)
        buf.data = (u8*) memory::realloc(buf.alloc,
                                         buf.data,
                                         new_capacity,
                                         32);
#elif defined(__SSE4_2__)
        buf.data = (u8*) memory::realloc(buf.alloc, buf.data, new_capacity, 16);
#else
        buf.data = (u8*) memory::realloc(buf.alloc, buf.data, new_capacity, 8);
#endif
        buf.capacity = new_capacity;
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

    status_t load(buf_t& buf, const u8* data, u32 size) {
        if (buf.mode == buf_mode_t::mapped)
            return status_t::buf_already_mapped;
        auto file = ::fmemopen((u0*) data, size, "r");
        if (!file)
            return status_t::unable_to_open_file;
        defer(::fclose(file));
        buf.mode = buf_mode_t::alloc;
        write(buf, 0, file, size);
        path::reset(buf.path);
        return status_t::ok;
    }

    status_t map_existing(buf_t& buf, const path_t& path) {
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

    status_t zero_fill(buf_t& buf, u32 offset, u32 length) {
        std::memset(buf.data + offset, 0, length);
        return status_t::ok;
    }

    b8 each_line(const buf_t& buf, const line_callback_t& cb) {
        for (const auto& line : buf.lines) {
            auto slice = slice::make(buf.data + line.pos, line.len);
            if (!cb(slice))
                return false;
        }
        return true;
    }

    status_t read(buf_t& buf, u32 offset, u0* data, u32 length) {
        std::memcpy(data, buf.data + offset, length);
        return status_t::ok;
    }

    status_t map_new(buf_t& buf, const path_t& path, usize size) {
        if (buf.mode == buf_mode_t::mapped)
            return status_t::buf_already_mapped;

        mode_t mode = S_IRUSR | S_IWUSR;
#ifdef _WIN32
        s32 flags = O_TRUNC | O_BINARY | O_RDWR | O_CREAT;
#else
        s32 flags = O_RDWR | O_CREAT;
#endif
        buf.file = open(path::c_str(path), flags, mode);
        if (buf.file == -1)
            return status_t::unable_to_open_file;

#ifdef _WIN32
        if (_chsize(buf.file, size) != 0)
            return status_t::unable_to_truncate_file;
#else
        if (ftruncate(buf.file, size) != 0)
            return status_t::unable_to_truncate_file;
#endif

        auto map = mmap(nullptr,
                        size,
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
        buf.length = size;
        path::set(buf.path, path);

        return status_t::ok;
    }

    status_t write(buf_t& buf, u32 offset, FILE* file, u32 length) {
        if (buf.mode == buf_mode_t::alloc
        && (offset + length + 1 > buf.capacity)) {
            grow(buf, offset + length + 1);
        }
        fread(buf.data + offset, 1, length, file);
        buf.length += offset + length > buf.length ? offset + length : length;
        return status_t::ok;
    }

    status_t write(buf_t& buf, u32 offset, const u8* data, u32 length) {
        if (buf.mode == buf_mode_t::alloc
        && (offset + length + 1 > buf.capacity)) {
            grow(buf, offset + length + 1);
        }
        std::memcpy(buf.data + offset, data, length);
        buf.length += offset + length > buf.length ? offset + length : length;
        return status_t::ok;
    }

    status_t save(buf_t& buf, const path_t& path, u32 offset, u32 length) {
        if (buf.length == 0)
            return status_t::cannot_save_zero_length_buf;
        if (buf.mode == buf_mode_t::mapped
        &&  buf.path == path) {
            return status_t::cannot_save_over_mapped_path;
        }
        auto file = fopen(path::c_str(path), "wb");
        if (!file)
            return status_t::unable_to_open_file;
        defer(fclose(file));
        length = !length ? buf.length : length;
        fwrite(buf.data + offset, 1, length, file);
        fflush(file);
        return status_t::ok;
    }
}
