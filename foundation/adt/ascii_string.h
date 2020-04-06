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

#include <foundation/types.h>
#include <foundation/memory/system.h>
#include "slice.h"

namespace basecode::string {
    using slice_t = slice::slice_t<u8>;

    [[maybe_unused]] inline static slice_t operator "" _ss(
            const char* value) {
        return slice_t{
            .length = (u32) strlen(value),
            .data = (const u8*) value,
        };
    }

    [[maybe_unused]] inline static slice_t operator "" _ss(
            const char* value,
            std::size_t length) {
        return slice_t{
            .length = (u32) length,
            .data = (const u8*) value,
        };
    }

    [[maybe_unused]] inline static slice_t make_slice(const std::string& str) {
        return slice_t{
            .length = (u32) str.length(),
            .data = (const u8*) str.data()
        };
    }

    [[maybe_unused]] inline static slice_t make_slice(const u8* str, u32 length) {
        return slice_t{
            .length = length,
            .data = str
        };
    }

    [[maybe_unused]] inline static slice_t make_slice(const char* str, u32 length) {
        return slice_t{
            .length = length,
            .data = (const u8*) str
        };
    }

    ///////////////////////////////////////////////////////////////////////////

    struct ascii_t final {
        u8* data{};
        u32 length{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        explicit ascii_t(memory::allocator_t* allocator = context::current()->allocator);

        ascii_t(const ascii_t& other);

        ascii_t(ascii_t&& other) noexcept;

        ascii_t(const char* value, memory::allocator_t* allocator = context::current()->allocator);

        ascii_t(slice_t value, memory::allocator_t* allocator = context::current()->allocator);

        ~ascii_t();

        inline u8* end() { return data + length; }

        inline u8* rend() { return data; }

        inline u8* begin() { return data; }

        inline u8* rbegin() { return data + length; }

        [[nodiscard]] inline const u8* end() const { return data + length; }

        [[nodiscard]] inline const u8* rend() const { return data; }

        [[nodiscard]] inline const u8* begin() const { return data; }

        [[nodiscard]] inline const u8* rbegin() const { return data + length; }

        inline u8& operator[](u32 index);

        inline const u8& operator[](u32 index) const;

        inline b8 operator<(const ascii_t& other) const;

        inline b8 operator>(const ascii_t& other) const;

        inline b8 operator==(const char* other) const;

        inline b8 operator==(const ascii_t& other) const;

        inline b8 operator==(const slice_t& other) const;

        inline ascii_t& operator+(const ascii_t& other);

        inline ascii_t& operator=(const ascii_t& other);

        inline ascii_t& operator=(ascii_t&& other) noexcept;
    };

    [[maybe_unused]] inline static slice_t make_slice(const ascii_t& str) {
        return slice_t{
            .length = str.length,
            .data = str.data
        };
    }

    u0 append(ascii_t& str, u8 value);

    u0 append(ascii_t& str, slice_t value);

    u0 append(ascii_t& str, const ascii_t& value);

    u0 append(ascii_t& str, const char* value, s32 len = -1);

    u0 trim(ascii_t& str);

    b8 empty(ascii_t& str);

    u0 trunc(ascii_t& str, u32 new_length);

    u0 ltrim(ascii_t& str);

    u0 rtrim(ascii_t& str);

    u0 clear(ascii_t& str);

    u0 reset(ascii_t& str);

    u0 upper(ascii_t& str);

    u0 lower(ascii_t& str);

    u8& back(ascii_t& str);

    u0 shrink(ascii_t& str);

    u0 insert(ascii_t& str, u32 pos, u8 value);

    u0 insert(ascii_t& str, u32 pos, slice_t value);

    u0 insert(ascii_t& str, u32 pos, const ascii_t& value);

    u0 insert(ascii_t& str, u32 pos, const char* value, s32 len = -1);

    u0 grow(ascii_t& str, u32 min_capacity = 8);

    u0 resize(ascii_t& str, u32 new_length);

    u0 reserve(ascii_t& str, u32 new_capacity);

    u0 set_capacity(ascii_t& str, u32 new_capacity);

    u8* erase(ascii_t& str, const u8* begin, const u8* end);

    inline u0 free(ascii_t& str) {
        clear(str);
    }

    inline u0 init(
            ascii_t* str,
            memory::allocator_t* allocator = context::current()->allocator) {
        str->allocator = allocator;
        str->data = {};
        str->length = str->capacity = {};
    }

    inline ascii_t make(memory::allocator_t* allocator = context::current()->allocator) {
        return ascii_t(allocator);
    }

    inline ascii_t::~ascii_t() {
        clear(*this);
    }

    inline ascii_t::ascii_t(memory::allocator_t* allocator) : allocator(allocator) {
        assert(allocator);
    }

    inline ascii_t::ascii_t(const ascii_t& other) {
        operator=(other);
    }

    inline ascii_t::ascii_t(ascii_t&& other) noexcept {
        operator=(other);
    }

    inline ascii_t::ascii_t(const char* value, memory::allocator_t* allocator) : allocator(allocator) {
        const auto n = strlen(value);
        grow(*this, n);
        std::memcpy(data, value, n * sizeof(u8));
        length = n;
    }

    inline ascii_t::ascii_t(slice_t value, memory::allocator_t* allocator) : allocator(allocator) {
        grow(*this, value.length);
        std::memcpy(data, value.data, value.length * sizeof(u8));
        length = value.length;
    }

    inline u8& ascii_t::operator[](u32 index) {
        return data[index];
    }

    inline const u8& ascii_t::operator[](u32 index) const {
        return data[index];
    }

    inline ascii_t& ascii_t::operator=(const ascii_t& other) {
        if (this != &other) {
            const auto n = other.length;
            grow(*this, n);
            std::memcpy(data, other.data, n * sizeof(u8));
            length = n;
        }
        return *this;
    }

    inline ascii_t& ascii_t::operator=(ascii_t&& other) noexcept {
        if (this != &other) {
            assert(allocator == other.allocator);
            memory::deallocate(allocator, data);
            data = other.data;
            length = other.length;
            capacity = other.capacity;
            other.data = {};
            other.length = other.capacity = {};
        }
        return *this;
    }

    inline b8 ascii_t::operator<(const ascii_t& other) const {
        return std::lexicographical_compare(
            begin(),
            end(),
            other.begin(),
            other.end());
    }

    inline b8 ascii_t::operator>(const ascii_t& other) const {
        return !std::lexicographical_compare(
            begin(),
            end(),
            other.begin(),
            other.end());
    }

    inline ascii_t& ascii_t::operator+(const ascii_t& other) {
        append(*this, other);
        return *this;
    }

    inline b8 ascii_t::operator==(const char* other) const {
        const auto n = strlen(other);
        return length == n && std::memcmp(data, other, length) == 0;
    }

    inline b8 ascii_t::operator==(const ascii_t& other) const {
        return length == other.length && std::memcmp(data, other.data, length) == 0;
    }

    inline b8 ascii_t::operator==(const slice_t& other) const {
        return length == other.length && std::memcmp(data, other.data, length) == 0;
    }
}

namespace fmt {
    using namespace basecode::string;

    template<>
    struct formatter<slice_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(slice_t slice, FormatContext& ctx) {
            auto it = format_to_n(
                ctx.out(),
                slice.length,
                "{}",
                (const char*) slice.data);
            return it.out;
        }
    };

    template<>
    struct formatter<ascii_t> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template<typename FormatContext>
        auto format(ascii_t str, FormatContext& ctx) {
            auto it = format_to_n(
                ctx.out(),
                str.length,
                "{}",
                (const char*) str.data);
            return it.out;
        }
    };
}