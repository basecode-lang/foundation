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

    u8 random_char() {
        return s_chars[t_pick(t_rg)];
    }

    str_t make(alloc_t* alloc) {
        str_t str{};
        init(str, alloc);
        return str;
    }
}
