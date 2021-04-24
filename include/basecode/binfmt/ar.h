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
#include <basecode/binfmt/binfmt.h>

namespace basecode::binfmt::ar {
    u0 free(ar_t& ar);

    u0 reset(ar_t& ar);

    status_t read(ar_t& ar, const path_t& path);

    status_t write(ar_t& ar, const path_t& path);

    u0 add_member(ar_t& ar, const ar_member_t& member);

    status_t init(ar_t& ar, alloc_t* alloc = context::top()->alloc.main);

    u0 find_member(ar_t& ar, str::slice_t name, ar_member_ptr_array_t& list);
}
