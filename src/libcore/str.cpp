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

#include <random>
#include <basecode/core/str.h>

namespace basecode::str {
    static auto s_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"_ss;

    thread_local std::mt19937                       t_rg{std::random_device{}()};
    thread_local std::uniform_int_distribution<u32> t_pick(0, s_chars.length - 1);

    u0 free(str_t& str) {
        clear(str);
    }

    u0 trim(str_t& str) {
        ltrim(str);
        rtrim(str);
    }

    u0 ltrim(str_t& str) {
        erase(str, *str.begin(), *std::find_if(str.begin(), str.end(), [](u8 c) { return !std::isspace(c); }));
    }

    u0 rtrim(str_t& str) {
        erase(str, *std::find_if(str.rbegin(), str.rend(), [](u8 c) { return !std::isspace(c); }), *str.end());
    }

    u0 clear(str_t& str) {
        memory::free(str.allocator, str.data);
        str.data = {};
        str.length = str.capacity = {};
    }

    u0 reset(str_t& str) {
        str.length = 0;
    }

    u0 lower(str_t& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    u0 upper(str_t& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    u8& back(str_t& str) {
        return str.data[str.length - 1];
    }

    u0 shrink(str_t& str) {
        set_capacity(str, str.length);
    }

    b8 empty(const str_t& str) {
        return str.length == 0 || str.data == nullptr;
    }

    const s8* c_str(str_t& str) {
        if (str.length + 1 > str.capacity)
            grow(str);
        str.data[str.length] = '\0';
        return (const s8*) str.data;
    }

    str_t make(alloc_t* allocator) {
        str_t str{};
        init(str, allocator);
        return str;
    }

    u0 append(str_t& str, u8 value) {
        if (str.length + 1 > str.capacity)
            grow(str);
        str.data[str.length++] = value;
    }

    u0 append(str_t& str, slice_t value) {
        append(str, value.data, value.length);
    }

    u0 trunc(str_t& str, u32 new_length) {
        str.length = new_length;
    }

    u0 grow(str_t& str, u32 min_capacity) {
        auto new_capacity = std::max(str.capacity, std::max(min_capacity, (u32) 8));
        set_capacity(str, new_capacity * 2 + 8);
    }

    u0 resize(str_t& str, u32 new_length) {
        if (new_length > str.capacity)
            grow(str, new_length);
        str.length = new_length;
    }

    b8 erase(str_t& str, u32 pos, u32 len) {
        if (pos + len < str.length) {
            std::memmove(str.data + pos, str.data + pos + 1, len * sizeof(u8));
        }
        str.length -= len;
        return true;
    }

    u0 init(str_t& str, alloc_t* allocator) {
        str.data      = {};
        str.allocator = allocator;
        str.length    = str.capacity = {};
    }

    u0 reserve(str_t& str, u32 new_capacity) {
        if (new_capacity > str.capacity)
            set_capacity(str, new_capacity);
    }

    u0 insert(str_t& str, u32 pos, u8 value) {
        insert(str, pos, &value, 1);
    }

    u0 append(str_t& str, const str_t& value) {
        append(str, value.data, value.length);
    }

    str_t random(u32 length, alloc_t* allocator) {
        str_t str;
        init(str, allocator);
        reserve(str, length);
        while(length--)
            append(str, s_chars[t_pick(t_rg)]);
        return str;
    }

    u0 insert(str_t& str, u32 pos, slice_t value) {
        insert(str, pos, value.data, value.length);
    }

    u0 set_capacity(str_t& str, u32 new_capacity) {
        if (new_capacity == str.capacity)
            return;

        if (new_capacity == 0) {
            memory::free(str.allocator, str.data);
            str.data = {};
            str.length = str.capacity = {};
            return;
        }

        str.data = (u8*) memory::realloc(str.allocator, str.data, new_capacity * sizeof(u8));
        str.capacity = new_capacity;
        if (new_capacity < str.length)
            str.length = new_capacity;
    }

    u0 append(str_t& str, const u8* value, s32 len) {
        if (len == 0) return;
        const auto n = len != -1 ? len : strlen((const char*) value);
        if (str.length + n > str.capacity)
            grow(str, n);
        u32 i{};
        while (i < n)
            str.data[str.length++] = value[i++];
    }

    u0 append(str_t& str, const s8* value, s32 len) {
        append(str, (const u8*) value, len != -1 ? len : strlen(value));
    }

    u0 insert(str_t& str, u32 pos, const str_t& value) {
        insert(str, pos, value.data, value.length);
    }

    u0 insert(str_t& str, u32 pos, const u8* value, s32 len) {
        if (len == 0) return;
        auto const n = len == -1 ? strlen((const char*) value) : len;
        const auto offset = (ptrdiff_t) str.data + pos;
        if (str.length + n > str.capacity)
            grow(str, n);
        if (offset < str.length) {
            std::memmove(
                str.data + offset + n,
                str.data + offset,
                (str.length + n - offset) * sizeof(u8));
        }
        std::memcpy(str.data + offset, value, n * sizeof(u8));
        str.length += n;
    }

    b8 each_line(const str_t& str, const line_callback_t& cb, str::slice_t sep) {
        s32 i{}, start_pos{};
        while (i < str.length) {
            if (std::memcmp(str.data + i, sep.data, sep.length) == 0) {
                str::slice_t line;
                line.data = str.data + start_pos;
                line.length = (i - start_pos) - 1;
                if (!cb(line))
                    return false;
                start_pos = i + sep.length;
            }
            ++i;
        }
        return 0;
    }
}
