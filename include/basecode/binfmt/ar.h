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

#include <basecode/core/buf.h>
#include <basecode/binfmt/binfmt.h>

namespace basecode::binfmt {
   using ar_catalog_t          = hashtab_t<str::slice_t, str::slice_t>;

    struct ar_t final {
        alloc_t*                alloc;
        buf_t                   buf;
        ar_catalog_t            catalog;
    };

    namespace ar {
        u0 free(ar_t& ar);

        u0 reset(ar_t& ar);

        status_t read(ar_t& ar, const path_t& path);

        status_t write(ar_t& ar, const path_t& path);

        status_t init(ar_t& ar, alloc_t* alloc = context::top()->alloc);
    }
}
