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
#include <basecode/core/context.h>
#include <basecode/core/memory/memory.h>
#include <basecode/core/hashing/murmur.h>
#include <basecode/core/hashing/hashable.h>

namespace basecode {
    struct string_t;

    namespace string {
        u0 free(string_t& str);

        u0 trim(string_t& str);

        u0 clear(string_t& str);

        b8 empty(string_t& str);

        u0 ltrim(string_t& str);

        u0 rtrim(string_t& str);

        u0 reset(string_t& str);

        u0 upper(string_t& str);

        u0 lower(string_t& str);

        u8& back(string_t& str);

        u0 shrink(string_t& str);

        u0 append(string_t& str, u8 value);

        u0 resize(string_t& str, u32 new_size);

        u0 trunc(string_t& str, u32 new_length);

        u0 append(string_t& str, slice_t value);

        u0 grow(string_t&, u32 min_capacity = 8);

        u0 insert(string_t& str, u32 pos, u8 value);

        u0 reserve(string_t& str, u32 new_capacity);

        u0 append(string_t& str, const string_t& value);

        u0 set_capacity(string_t& str, u32 new_capacity);

        u0 insert(string_t& str, u32 pos, slice_t value);

        u0 insert(string_t& str, u32, const u8*, s32 len = -1);

        u0 append(string_t& str, const u8* value, s32 len = -1);

        u0 append(string_t& str, const s8* value, s32 len = -1);

        u8* erase(string_t& str, const u8* begin, const u8* end);

        u0 insert(string_t& str, u32 pos, const string_t& value);

        string_t make(alloc_t* allocator = context::top()->alloc);

        u0 init(string_t& str, alloc_t* allocator = context::top()->alloc);

        string_t random(u32 length, alloc_t* allocator = context::top()->alloc);
    }

    struct string_t final {
        u8*                     data{};
        alloc_t*                allocator{};
        u32                     length{};
        u32                     capacity{};

        string_t() = default;
        ~string_t()                                                     { string::clear(*this); }
        string_t(string_t&& other) noexcept                             { operator=(other); }
        string_t(const string_t& other) : allocator(other.allocator)    { operator=(other); }

        explicit string_t(
                const char* value,
                alloc_t* allocator = context::top()->alloc) : allocator(allocator) {
            const auto n = strlen(value);
            string::grow(*this, n);
            std::memcpy(data, value, n * sizeof(u8));
            length = n;
        }

        explicit string_t(
                string::slice_t value,
                alloc_t* allocator = context::top()->alloc) : allocator(allocator) {
            string::grow(*this, value.length);
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

        b8 operator<(const string_t& other) const {
            return std::lexicographical_compare(
                begin(),
                end(),
                other.begin(),
                other.end());
        }

        b8 operator>(const string_t& other) const {
            return !std::lexicographical_compare(
                begin(),
                end(),
                other.begin(),
                other.end());
        }

        b8 operator==(const char* other) const {
            const auto n = strlen(other);
            return length == n && std::memcmp(data, other, length) == 0;
        }

        string_t& operator=(const string_t& other) {
            if (this != &other) {
                if (!allocator)
                    allocator = other.allocator;
                const auto n = other.length;
                string::grow(*this, n);
                std::memcpy(data, other.data, n * sizeof(u8));
                length = n;
            }
            return *this;
        }

        string_t& operator+(const string_t& other) {
            string::append(*this, other);
            return *this;
        }

        b8 operator==(const string_t& other) const {
            return length == other.length && std::memcmp(data, other.data, length) == 0;
        }

        string_t& operator=(string_t&& other) noexcept {
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

        b8 operator==(const string::slice_t& other) const {
            return length == other.length && std::memcmp(data, other.data, length) == 0;
        }
    };

    namespace slice {
        inline string::slice_t make(const string_t& str) {
            return string::slice_t{.data = str.data, .length = str.length};
        }
    }

    inline string::slice_t operator "" _ss(const s8* value) {
        return string::slice_t{.data = (const u8*) value, .length = (u32) strlen(value)};
    }

    inline string::slice_t operator "" _ss(const s8* value, std::size_t length) {
        return string::slice_t{.data = (const u8*) value, .length = (u32) length};
    }
}

namespace basecode::hashing {
    inline u64 hash64(const string_t& key) {
        return murmur::hash64(key.data, key.length);
    }
}

template<>
struct fmt::formatter<basecode::string_t> : fmt::formatter<std::string_view> {
    template<typename FormatContext>
    auto format(const basecode::string_t& str, FormatContext& ctx) {
        std::string_view temp((const basecode::s8*) str.data, str.length);
        return formatter<std::string_view>::format(temp, ctx);
    }
};
