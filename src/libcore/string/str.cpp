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
#include "str.h"

namespace basecode::string {
    static auto s_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"_ss;

    thread_local std::mt19937                       t_rg{std::random_device{}()};
    thread_local std::uniform_int_distribution<u32> t_pick(0, s_chars.length - 1);

    u0 free(string_t& str) {
        clear(str);
    }

    u0 trim(string_t& str) {
        ltrim(str);
        rtrim(str);
    }

    u0 ltrim(string_t& str) {
        erase(
            str,
            str.begin(),
            std::find_if(
                str.begin(),
                str.end(),
                [](u8 c) { return !std::isspace(c); }));
    }

    u0 rtrim(string_t& str) {
        erase(
            str,
            std::find_if(
                str.rbegin(),
                str.rend(),
                [](u8 c) { return !std::isspace(c); }),
            str.end());
    }

    u0 clear(string_t& str) {
        memory::free(str.allocator, str.data);
        str.data = {};
        str.length = str.capacity = {};
    }

    u0 reset(string_t& str) {
        str.length = 0;
    }

    u0 lower(string_t& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    u0 upper(string_t& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    u8& back(string_t& str) {
        return str.data[str.length - 1];
    }

    b8 empty(string_t& str) {
        return str.length == 0 || str.data == nullptr;
    }

    u0 shrink(string_t& str) {
        set_capacity(str, str.length);
    }

    string_t make(alloc_t* allocator) {
        string_t str{};
        init(str, allocator);
        return str;
    }

    u0 append(string_t& str, u8 value) {
        if (str.length + 1 > str.capacity)
            grow(str);
        str.data[str.length++] = value;
    }

    u0 append(string_t& str, slice_t value) {
        append(str, value.data, value.length);
    }

    u0 trunc(string_t& str, u32 new_length) {
        str.length = new_length;
    }

    u0 grow(string_t& str, u32 min_capacity) {
        auto new_capacity = std::max(str.capacity, std::max(min_capacity, (u32) 8));
        set_capacity(str, new_capacity * 2 + 8);
    }

    u0 resize(string_t& str, u32 new_length) {
        if (new_length > str.capacity)
            grow(str, new_length);
        str.length = new_length;
    }

    u0 reserve(string_t& str, u32 new_capacity) {
        if (new_capacity > str.capacity)
            set_capacity(str, new_capacity);
    }

    u0 insert(string_t& str, u32 pos, u8 value) {
        insert(str, pos, &value, 1);
    }

    u0 append(string_t& str, const string_t& value) {
        append(str, value.data, value.length);
    }

    u0 insert(string_t& str, u32 pos, slice_t value) {
        insert(str, pos, value.data, value.length);
    }

    u0 set_capacity(string_t& str, u32 new_capacity) {
        if (new_capacity == str.capacity)
            return;

        u8* new_data{};
        if (new_capacity > 0) {
            new_data = (u8*) memory::alloc(str.allocator, new_capacity * sizeof(u8));
            if (str.data)
                std::memcpy(new_data, str.data, str.length * sizeof(u8));
        }

        memory::free(str.allocator, str.data);
        str.data = new_data;
        str.capacity = new_capacity;
        if (new_capacity < str.length)
            str.length = new_capacity;
    }

    u0 append(string_t& str, const u8* value, s32 len) {
        const auto n = len != -1 ? len : strlen((const char*) value);
        if (str.length + n > str.capacity)
            grow(str, n);
        u32 i{};
        while (i < n)
            str.data[str.length++] = value[i++];
    }

    u0 append(string_t& str, const s8* value, s32 len) {
        append(str, (const u8*) value, len != -1 ? len : strlen(value));
    }

    u0 init(string_t& str, alloc_t* allocator) {
        str.data = {};
        str.allocator = allocator;
        str.length = str.capacity = {};
    }

    string_t random(u32 length, alloc_t* allocator) {
        string_t str;
        init(str, allocator);
        reserve(str, length);
        while(length--)
            append(str, s_chars[t_pick(t_rg)]);
        return str;
    }

    u0 insert(string_t& str, u32 pos, const string_t& value) {
        insert(str, pos, value.data, value.length);
    }

    u8* erase(string_t& str, const u8* begin, const u8* end) {
        const auto count = end - begin;
        if (count <= 0)
            return (u8*) begin;

        const auto offset = begin - str.data;
        std::memmove(
            str.data + offset,
            str.data + offset + count,
            (str.length - offset - count) * sizeof(u8));
        str.length -= count;

        return str.data + offset;
    }

    u0 insert(string_t& str, u32 pos, const u8* value, s32 len) {
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
}
