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

#include <utf8proc.h>
#include <foundation/types.h>
#include <foundation/memory/system.h>

namespace basecode::utf8_string {
    struct string_t final {
        explicit string_t(memory::allocator_t* allocator);

        ~string_t();

        string_t(const string_t& other);

        string_t(string_t&& other) noexcept;

        u8* data{};
        u32 size{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        u8* end() { return data + size; }

        u8* rend() { return data; }

        u8* begin() { return data; }

        u8* rbegin() { return data + size; }

        const u8* end() const { return data + size; }

        const u8* rend() const { return data; }

        const u8* begin() const { return data; }

        const u8* rbegin() const { return data + size; }

        u8& operator[](usize index);

        const u8& operator[](usize index) const;

        string_t& operator=(const string_t& other);

        string_t& operator=(string_t&& other) noexcept;
    };
}