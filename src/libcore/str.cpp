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

#include <random>
#include <basecode/core/str.h>

namespace basecode {
    namespace str {
        static auto s_chars = "0123456789abcdef"
                              "ghijklmnopqrstuv"
                              "wxyzABCDEFGHIJKL"
                              "MNOPQRSTUVWXYZ"_ss;

        thread_local std::mt19937                       t_rg{std::random_device{}()};
        thread_local std::uniform_int_distribution<u32> t_pick(0, s_chars.length - 1);

        u8 random_char() {
            return s_chars[t_pick(t_rg)];
        }

        str_t make(alloc_t* alloc) {
            str_t str{};
            init(str, alloc);
            return str;
        }
    }

    str_t& str_t::operator+(const str_t& other) {
        str::append(*this, other);
        return *this;
    }

    str_t& str_t::operator=(const str_t& other) {
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

    str_t& str_t::operator=(str_t&& other) noexcept {
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

    str_t& str_t::operator=(const slice_t<basecode::u8>& other) {
        BC_ASSERT_NOT_NULL(alloc);
        const auto n = other.length;
        str::grow(*this, n);
        std::memcpy(data, other.data, n * sizeof(u8));
        length = n;
        return *this;
    }

    str_t::str_t(const s8* value, alloc_t* alloc) : alloc(alloc) {
        const auto n = strlen(value) + 1;
        str::grow(*this, n);
        std::memcpy(data, value, n * sizeof(u8));
        data[n] = '\0';
        length = n - 1;
    }

    str_t::str_t(slice_t<u8> value, alloc_t* alloc) : alloc(alloc) {
        str::grow(*this, value.length);
        std::memcpy(data, value.data, value.length * sizeof(u8));
        length = value.length;
    }
}

