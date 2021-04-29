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

#pragma once

#include <cstring>
#include <basecode/core/types.h>
#include <basecode/core/slice.h>
#include <basecode/core/assert.h>
#include <basecode/core/memory.h>
#include <basecode/core/context.h>
#include <basecode/core/hash/murmur.h>

namespace basecode {
    namespace str {
        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 free(T& str);

        template <Dynamic_String_Concept T>
        u0 grow(T& str, u32 min_capacity = 8);

        template <String_Concept D, String_Concept S>
        inline u0 append(D& str, const S& value);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 append(T& str, const u8* value, s32 len = -1);

        template <String_Concept T>
        inline u0 append(T& str, const s8* value, s32 len = -1);

        str_t make(alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 init(T& str, alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 insert(T& str, u32 pos, const u8* value, s32 len = -1);
    }

    struct str_t final {
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, false>;

        alloc_t*                alloc{};
        u8*                     data{};
        u32                     length{};
        u32                     capacity{};

        str_t() = default;
        ~str_t()                                                  { str::free(*this); }
        str_t(str_t&& other) noexcept                             { operator=(other); }
        str_t(const str_t& other) : alloc(other.alloc)            { operator=(other); }

        explicit str_t(const s8* value,
                       alloc_t* alloc = context::top()->alloc.main) : alloc(alloc) {
            const auto n = strlen(value) + 1;
            str::grow(*this, n);
            std::memcpy(data, value, n * sizeof(u8));
            data[n] = '\0';
            length = n - 1;
        }

        explicit str_t(str::slice_t value,
                       alloc_t* alloc = context::top()->alloc.main) : alloc(alloc) {
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

        str_t& operator=(str_t&& other) noexcept {
            if (this != &other) {
                if (alloc)
                    memory::free(alloc, data);
                data     = other.data;
                length   = other.length;
                capacity = other.capacity;
                alloc    = other.alloc;
                other.data     = {};
                other.length   = {};
                other.capacity = {};
            }
            return *this;
        }

        inline str_t& operator+(const str_t& other) {
            str::append(*this, other);
            return *this;
        }

        str_t& operator=(const str::slice_t& other) {
            BC_ASSERT_NOT_NULL(alloc);
            const auto n = other.length;
            str::grow(*this, n);
            std::memcpy(data, other.data, n * sizeof(u8));
            length = n;
            return *this;
        }

        inline auto operator==(const char* other) const {
            const auto n = strlen(other);
            return length == n && std::memcmp(data, other, length) == 0;
        }

        inline auto operator==(const str_t& other) const {
            const auto cmp = std::lexicographical_compare_three_way(begin(),
                                                          end(),
                                                          other.begin(),
                                                          other.end());
            return cmp == std::strong_ordering::equal;
        }

        inline auto operator<=>(const str_t& other) const {
            return std::lexicographical_compare_three_way(begin(),
                                                          end(),
                                                          other.begin(),
                                                          other.end());
        }

        inline auto operator==(const str::slice_t& other) const {
            return length == other.length
                   && std::memcmp(data, other.data, length) == 0;
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

        template <String_Concept T>
        u0 trim(T& str) {
            ltrim(str);
            rtrim(str);
        }

        template <String_Concept T>
        u0 clear(T& str) {
            free(str);
        }

        template <String_Concept T>
        u0 upper(T& str) {
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        }

        template <String_Concept T>
        u0 reset(T& str) {
            str.length = 0;
        }

        template <String_Concept T>
        u8& back(T& str) {
            return str.data[str.length - 1];
        }

        template <String_Concept T, b8 Is_Static>
        u0 free(T& str) {
            if constexpr (!Is_Static) {
                memory::free(str.alloc, str.data);
                str.data     = {};
                str.capacity = {};
            }
            str.length = {};
        }

        template <String_Concept T>
        u0 lower(T& str) {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        }

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 append(T& str, u8 value) {
            if constexpr (Is_Static) {
                BC_ASSERT(str.length + 1 < str.capacity);
            } else {
                if (str.length + 1 > str.capacity)
                    grow(str, str.capacity + 1);
            }
            str.data[str.length++] = value;
        }

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        const s8* c_str(T& str) {
            if constexpr (Is_Static) {
                BC_ASSERT(str.length + 1 < str.capacity);
            } else {
                if (str.length + 1 > str.capacity)
                    grow(str, str.capacity + 1);
            }
            str.data[str.length] = '\0';
            return (const s8*) str.data;
        }

        template <String_Concept T>
        inline b8 empty(const T& str) {
            return str.length == 0;
        }

        template <String_Concept T>
        u0 random(T& str, u32 length) {
            while(length--)
                append(str, random_char());
        }

        template <String_Concept T, b8 Is_Static>
        u0 init(T& str, alloc_t* alloc) {
            if constexpr (!Is_Static) {
                str.data     = {};
                str.capacity = {};
                str.alloc    = alloc;
            }
            str.length = {};
        }

        template <Dynamic_String_Concept T>
        u0 resize(T& str, u32 new_length) {
            if (new_length > str.capacity)
                grow(str, new_length);
            str.length = new_length;
        }

        template <Dynamic_String_Concept T>
        u0 grow(T& str, u32 min_capacity) {
            auto new_capacity = std::max(str.capacity,
                                         std::max(min_capacity, (u32) 8));
            if (new_capacity > str.capacity)
                reserve(str, new_capacity * 2 + 8);
        }

        template <String_Concept T>
        [[maybe_unused]] u0 ltrim(T& str) {
            erase(str,
                  *str.begin(),
                  *std::find_if(str.begin(),
                                str.end(),
                                [](u8 c) { return !std::isspace(c); }));
        }

        template <String_Concept T>
        [[maybe_unused]] u0 rtrim(T& str) {
            erase(str,
                  *std::find_if(str.rbegin(),
                                str.rend(),
                                [](u8 c) { return !std::isspace(c); }),
                  *str.end());
        }

        template <String_Concept T>
        b8 erase(T& str, u32 pos, u32 len) {
            if (pos + len < str.length) {
                std::memmove(str.data + pos,
                             str.data + pos + 1,
                             len * sizeof(u8));
            }
            str.length -= len;
            return true;
        }

        template <Dynamic_String_Concept T>
        u0 reserve(T& str, u32 new_capacity) {
            if (new_capacity == str.capacity)
                return;

            if (new_capacity == 0) {
                memory::free(str.alloc, str.data);
                str.data = {};
                str.length = str.capacity = {};
                return;
            }

            str.data = (u8*) memory::realloc(str.alloc,
                                             str.data,
                                             new_capacity * sizeof(u8));
            str.capacity = new_capacity;
            if (new_capacity < str.length)
                str.length = new_capacity;
        }

        template <String_Concept T>
        inline u0 trunc(T& str, u32 new_length) {
            str.length = new_length;
        }

        template <String_Concept D, String_Concept S>
        inline u0 append(D& str, const S& value) {
            append(str, value.data, value.length);
        }

        template <String_Concept T>
        b8 each_line(const T& str,
                     const line_callback_t& cb,
                     str::slice_t sep = "\n"_ss) {
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
            if (start_pos < str.length) {
                return cb(slice::make(str.data + start_pos,
                                      str.length - start_pos));
            }
            return true;
        }

        template <String_Concept T, b8 Is_Static>
        u0 append(T& str, const u8* value, s32 len) {
            if (len == 0)
                return;
            const auto n = len != -1 ? len : strlen((const char*) value);
            if constexpr (Is_Static) {
                BC_ASSERT(str.length + n < str.capacity);
            } else {
                if (str.length + n > str.capacity)
                    grow(str, str.capacity + n);
            }
            u32 i{};
            while (i < n)
                str.data[str.length++] = value[i++];
        }

        template <String_Concept T>
        inline u0 insert(T& str, u32 pos, u8 value) {
            insert(str, pos, &value, 1);
        }

        template <String_Concept T>
        inline u0 insert(T& str, u32 pos, const T& value) {
            insert(str, pos, value.data, value.length);
        }

        template <String_Concept T>
        inline u0 append(T& str, const s8* value, s32 len) {
            append(str, (const u8*) value, len != -1 ? len : strlen(value));
        }

        template <String_Concept T, b8 Is_Static>
        u0 insert(T& str, u32 pos, const u8* value, s32 len) {
            if (len == 0)
                return;
            auto const n = len == -1 ? strlen((const char*) value) : len;
            const auto offset = (ptrdiff_t) str.data + pos;
            if constexpr (Is_Static) {
                BC_ASSERT(str.length + n < str.capacity);
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
    }
}

FORMAT_TYPE_AS(basecode::str_t, std::string_view);
