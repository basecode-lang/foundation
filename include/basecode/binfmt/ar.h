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

#include <basecode/core/buf.h>
#include <basecode/core/bitset.h>
#include <basecode/binfmt/binfmt.h>

namespace basecode::binfmt::ar {
    struct ar_t;
    struct member_t;

    using symbol_table_t        = hashtab_t<str::slice_t, u32>;
    using member_array_t        = array_t<member_t>;
    using member_ptr_array_t    = array_t<member_t*>;

    constexpr u32 header_size   = 60;

    struct member_t final {
        str::slice_t            name;
        str::slice_t            content;
        struct {
            u32                 header;
            u32                 data;
        }                       offset;
        u32                     date;
        u32                     uid;
        u32                     gid;
        u32                     mode;
    };

    struct ar_t final {
        alloc_t*                alloc;
        buf_t                   buf;
        member_array_t          members;
        symbol_table_t          symbol_map;
        bitset_t                symbol_module_bitmap;
        struct {
            u8                  long_names;
            u8                  symbol_table;
            u8                  ecoff_symbol_table;
            u8                  pad;
        }                       known;
    };

    u0 free(ar_t& ar);

    u0 reset(ar_t& ar);

    status_t read(ar_t& ar, const path_t& path);

    status_t write(ar_t& ar, const path_t& path);

    u0 add_member(ar_t& ar, const member_t& member);

    status_t init(ar_t& ar, alloc_t* alloc = context::top()->alloc.main);

    u0 find_member(ar_t& ar, str::slice_t name, member_ptr_array_t& list);
}
