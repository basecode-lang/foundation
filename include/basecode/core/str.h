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

#pragma once

#include <cstring>
#include <cassert>
#include <basecode/core/slice.h>
#include <basecode/core/types.h>
#include <basecode/core/memory.h>
#include <basecode/core/context.h>
#include <basecode/core/hashable.h>
#include <basecode/core/hash/murmur.h>

namespace basecode {
    struct str_t;

    namespace str {
        u0 free(str_t& str);

        u0 trim(str_t& str);

        u0 clear(str_t& str);

        u0 ltrim(str_t& str);

        u0 rtrim(str_t& str);

        u0 reset(str_t& str);

        u0 upper(str_t& str);

        u0 lower(str_t& str);

        u8& back(str_t& str);

        u0 shrink(str_t& str);

        b8 empty(const str_t& str);

        const s8* c_str(str_t& str);

        u0 append(str_t& str, u8 value);

        u0 resize(str_t& str, u32 new_size);

        u0 trunc(str_t& str, u32 new_length);

        u0 append(str_t& str, slice_t value);

        u0 grow(str_t&, u32 min_capacity = 8);

        b8 erase(str_t& str, u32 pos, u32 len);

        u0 insert(str_t& str, u32 pos, u8 value);

        u0 reserve(str_t& str, u32 new_capacity);

        u0 append(str_t& str, const str_t& value);

        u0 set_capacity(str_t& str, u32 new_capacity);

        u0 insert(str_t& str, u32 pos, slice_t value);

        u0 insert(str_t& str, u32 pos, const str_t& value);

        u0 insert(str_t& str, u32, const u8*, s32 len = -1);

        u0 append(str_t& str, const u8* value, s32 len = -1);

        u0 append(str_t& str, const s8* value, s32 len = -1);

        str_t make(alloc_t* allocator = context::top()->alloc);

        u0 init(str_t& str, alloc_t* allocator = context::top()->alloc);

        str_t random(u32 length, alloc_t* allocator = context::top()->alloc);
    }

    struct str_t final {
        u8*                     data{};
        alloc_t*                allocator{};
        u32                     length{};
        u32                     capacity{};

        str_t() = default;
        ~str_t()                                                  { str::clear(*this); }
        str_t(str_t&& other) noexcept                             { operator=(other); }
        str_t(const str_t& other) : allocator(other.allocator)    { operator=(other); }

        explicit str_t(
                const char* value,
                alloc_t* allocator = context::top()->alloc) : allocator(allocator) {
            const auto n = strlen(value) + 1;
            str::grow(*this, n);
            std::memcpy(data, value, n * sizeof(u8));
            data[n] = '\0';
            length = n;
        }

        explicit str_t(
            str::slice_t value,
            alloc_t* allocator = context::top()->alloc) : allocator(allocator) {
            str::grow(*this, value.length);
            std::memcpy(data, value.data, value.length * sizeof(u8));
            length = value.length;
        }

        u8* end()                               { return data + length; }
        u8* rend()                              { return data; }
        u8* begin()                             { return data; }
        u8* rbegin()                            { return data + length; }
        const u8* end() const                   { return data + length; }
        const u8* rend() const                  { return data; }
        const u8* begin() const                 { return data; }
        const u8* rbegin() const                { return data + length; }
        u8& operator[](u32 index)               { return data[index]; }
        const u8& operator[](u32 index) const   { return data[index]; }
        operator std::string_view () const      { return std::string_view((const s8*) data, length); }

        str_t& operator+(const str_t& other) {
            str::append(*this, other);
            return *this;
        }

        str_t& operator=(const str_t& other) {
            if (this != &other) {
                if (!allocator)
                    allocator = other.allocator;
                const auto n = other.length;
                str::grow(*this, n);
                std::memcpy(data, other.data, n * sizeof(u8));
                length = n;
            }
            return *this;
        }

        b8 operator<(const str_t& other) const {
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }

        b8 operator>(const str_t& other) const {
            return !std::lexicographical_compare(begin(), end(), other.begin(),
                other.end());
        }

        b8 operator==(const char* other) const {
            const auto n = strlen(other);
            return length == n && std::memcmp(data, other, length) == 0;
        }

        b8 operator==(const str_t& other) const {
            return length == other.length && std::memcmp(data, other.data, length) == 0;
        }

        str_t& operator=(str_t&& other) noexcept {
            if (this != &other) {
                if (allocator)
                    memory::free(allocator, data);
                data = other.data;
                length = other.length;
                capacity = other.capacity;
                allocator = other.allocator;
                other.data = {};
                other.length = other.capacity = {};
            }
            return *this;
        }

        b8 operator==(const str::slice_t& other) const {
            return length == other.length && std::memcmp(data, other.data, length) == 0;
        }
    };
    static_assert(sizeof(str_t) <= 24, "str_t is now larger than 24 bytes!");

    namespace slice {
        inline str::slice_t make(const str_t& str) {
            return str::slice_t{.data = str.data, .length = str.length};
        }
    }

    inline str::slice_t operator "" _ss(const s8* value) {
        return slice::make(value);
    }

    inline str::slice_t operator "" _ss(const s8* value, std::size_t length) {
        return slice::make(value, length);
    }

    namespace str {
        b8 each_line(const str_t& str, const line_callback_t& cb, str::slice_t sep = "\n"_ss);
    }
}

namespace basecode::hash {
    inline u64 hash64(const str_t& key) {
        return murmur::hash64(key.data, key.length);
    }
}

FORMAT_TYPE_AS(basecode::str_t, std::string_view);
