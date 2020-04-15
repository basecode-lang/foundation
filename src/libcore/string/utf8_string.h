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
#include <basecode/core/types.h>
#include <basecode/core/memory/system.h>

namespace basecode::string {
    using codepoint_t = s32;

    struct utf8_t final {
        explicit utf8_t(memory::allocator_t* allocator);

        ~utf8_t();

        utf8_t(const utf8_t& other);

        utf8_t(utf8_t&& other) noexcept;

        u8* data{};
        u32 length{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        u8* end() { return data + length; }

        u8* rend() { return data; }

        u8* begin() { return data; }

        u8* rbegin() { return data + length; }

        const u8* end() const { return data + length; }

        const u8* rend() const { return data; }

        const u8* begin() const { return data; }

        const u8* rbegin() const { return data + length; }

        u8& operator[](usize index);

        const u8& operator[](usize index) const;

        utf8_t& operator=(const utf8_t& other);

        utf8_t& operator=(utf8_t&& other) noexcept;
    };
}