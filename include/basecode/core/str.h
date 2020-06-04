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
        u0 free(String_Concept auto& str);

        str_t make(alloc_t* alloc = context::top()->alloc);

        u0 grow(Dynamic_String_Concept auto&, u32 min_capacity = 8);

        u0 append(String_Concept auto& str, String_Concept auto& value);

        u0 init(String_Concept auto& str, alloc_t* alloc = context::top()->alloc);
    }

    struct str_t final {
        using is_static         = std::integral_constant<b8, false>;

        alloc_t*                alloc{};
        u8*                     data{};
        u32                     length{};
        u32                     capacity{};

        str_t() = default;
        ~str_t()                                                  { str::free(*this); }
        str_t(str_t&& other) noexcept                             { operator=(other); }
        str_t(const str_t& other) : alloc(other.alloc)            { operator=(other); }

        explicit str_t(const s8* value, alloc_t* alloc = context::top()->alloc) : alloc(alloc) {
            const auto n = strlen(value) + 1;
            str::grow(*this, n);
            std::memcpy(data, value, n * sizeof(u8));
            data[n] = '\0';
            length = n;
        }

        explicit str_t(str::slice_t value, alloc_t* alloc = context::top()->alloc) : alloc(alloc) {
            str::grow(*this, value.length);
            std::memcpy(data, value.data, value.length * sizeof(u8));
            length = value.length;
        }

        u8* end()                               { return data + length; }
        u8* rend()                              { return data; }
        u8* begin()                             { return data; }
        u8* rbegin()                            { return data + length; }
        [[nodiscard]] const u8* end() const     { return data + length; }
        [[nodiscard]] const u8* rend() const    { return data; }
        [[nodiscard]] const u8* begin() const   { return data; }
        [[nodiscard]] const u8* rbegin() const  { return data + length; }
        u8& operator[](u32 index)               { return data[index]; }
        const u8& operator[](u32 index) const   { return data[index]; }
        operator std::string_view () const      { return std::string_view((const s8*) data, length); }
        operator str::slice_t() const           { return str::slice_t{.data = data, .length = length};  }

        str_t& operator+(const str_t& other) {
            str::append(*this, other);
            return *this;
        }

        str_t& operator=(const str_t& other) {
            if (this != &other) {
                if (!alloc)
                    alloc = other.alloc;
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
                if (alloc)
                    memory::free(alloc, data);
                data = other.data;
                length = other.length;
                capacity = other.capacity;
                alloc    = other.alloc;
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
        u8 random_char();

        u0 trim(String_Concept auto& str) {
            ltrim(str);
            rtrim(str);
        }

        u0 free(String_Concept auto& str) {
            using T = std::remove_reference_t<decltype(str)>;
            if constexpr (!T::is_static::value) {
                memory::free(str.alloc, str.data);
                str.data     = {};
                str.capacity = {};
            }
            str.length = {};
        }

        u0 clear(String_Concept auto& str) {
            free(str);
        }

        u0 ltrim(String_Concept auto& str) {
            erase(str, *str.begin(), *std::find_if(str.begin(), str.end(), [](u8 c) { return !std::isspace(c); }));
        }

        u0 rtrim(String_Concept auto& str) {
            erase(str, *std::find_if(str.rbegin(), str.rend(), [](u8 c) { return !std::isspace(c); }), *str.end());
        }

        u0 reset(String_Concept auto& str) {
            str.length = 0;
        }

        u0 upper(String_Concept auto& str) {
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        }

        u0 lower(String_Concept auto& str) {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        }

        u8& back(String_Concept auto& str) {
            return str.data[str.length - 1];
        }

        b8 empty(const String_Concept auto& str) {
            return str.length == 0;
        }

        const s8* c_str(String_Concept auto& str) {
            using T = std::remove_reference_t<decltype(str)>;
            if constexpr (T::is_static::value) {
                assert(str.length + 1 < str.capacity);
            } else {
                if (str.length + 1 > str.capacity)
                    grow(str);
            }
            str.data[str.length] = '\0';
            return (const s8*) str.data;
        }

        u0 append(String_Concept auto& str, u8 value) {
            using T = std::remove_reference_t<decltype(str)>;
            if constexpr (T::is_static::value) {
                assert(str.length + 1 < str.capacity);
            } else {
                if (str.length + 1 > str.capacity)
                    grow(str);
            }
            str.data[str.length++] = value;
        }

        u0 random(String_Concept auto& str, u32 length) {
            while(length--)
                append(str, random_char());
        }

        u0 init(String_Concept auto& str, alloc_t* alloc) {
            using T = std::remove_reference_t<decltype(str)>;
            if constexpr (!T::is_static::value) {
                str.data     = {};
                str.capacity = {};
                str.alloc    = alloc;
            }
            str.length = {};
        }

        u0 trunc(String_Concept auto& str, u32 new_length) {
            str.length = new_length;
        }

        b8 erase(String_Concept auto& str, u32 pos, u32 len) {
            if (pos + len < str.length)
                std::memmove(str.data + pos, str.data + pos + 1, len * sizeof(u8));
            str.length -= len;
            return true;
        }

        u0 insert(String_Concept auto& str, u32 pos, u8 value) {
            insert(str, pos, &value, 1);
        }

        u0 reserve(Dynamic_String_Concept auto& str, u32 new_capacity) {
            if (new_capacity == str.capacity)
                return;

            if (new_capacity == 0) {
                memory::free(str.alloc, str.data);
                str.data = {};
                str.length = str.capacity = {};
                return;
            }

            str.data = (u8*) memory::realloc(str.alloc, str.data, new_capacity * sizeof(u8));
            str.capacity = new_capacity;
            if (new_capacity < str.length)
                str.length = new_capacity;
        }

        u0 resize(Dynamic_String_Concept auto& str, u32 new_length) {
            if (new_length > str.capacity)
                grow(str, new_length);
            str.length = new_length;
        }

        u0 grow(Dynamic_String_Concept auto& str, u32 min_capacity) {
            auto new_capacity = std::max(str.capacity, std::max(min_capacity, (u32) 8));
            reserve(str, new_capacity * 2 + 8);
        }

        u0 append(String_Concept auto& str, const u8* value, s32 len = -1) {
            using T = std::remove_reference_t<decltype(str)>;
            if (len == 0) return;
            const auto n = len != -1 ? len : strlen((const char*) value);
            if constexpr (T::is_static::value) {
                assert(str.length + n < str.capacity);
            } else {
                if (str.length + n > str.capacity)
                    grow(str, n);
            }
            u32 i{};
            while (i < n)
                str.data[str.length++] = value[i++];
        }

        u0 append(String_Concept auto& str, String_Concept auto& value) {
            append(str, value.data, value.length);
        }

        u0 insert(String_Concept auto& str, u32 pos, const u8* value, s32 len = -1) {
            using T = std::remove_reference_t<decltype(str)>;
            if (len == 0) return;
            auto const n = len == -1 ? strlen((const char*) value) : len;
            const auto offset = (ptrdiff_t) str.data + pos;
            if constexpr (T::is_static::value) {
                assert(str.length + n < str.capacity);
            } else {
                if (str.length + n > str.capacity)
                    grow(str, n);
            }
            if (offset < str.length) {
                std::memmove(
                    str.data + offset + n,
                    str.data + offset,
                    (str.length + n - offset) * sizeof(u8));
            }
            std::memcpy(str.data + offset, value, n * sizeof(u8));
            str.length += n;
        }

        u0 append(String_Concept auto& str, const s8* value, s32 len = -1) {
            append(str, (const u8*) value, len != -1 ? len : strlen(value));
        }

        u0 append(String_Concept auto& str, const String_Concept auto& value) {
            append(str, value.data, value.length);
        }

        u0 insert(String_Concept auto& str, u32 pos, const String_Concept auto& value) {
            insert(str, pos, value.data, value.length);
        }

        b8 each_line(const String_Concept auto& str, const line_callback_t& cb, str::slice_t sep = "\n"_ss) {
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
            return true;
        }
    }
}

namespace basecode::hash {
    inline u64 hash64(const String_Concept auto& key) {
        return murmur::hash64(key.data, key.length);
    }
}

FORMAT_TYPE_AS(basecode::str_t, std::string_view);
