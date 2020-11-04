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

#include <basecode/binfmt/ar.h>

namespace basecode::binfmt::ar {
    u0 free(ar_t& ar) {
        buf::free(ar.buf);
        hashtab::free(ar.catalog);
    }

    u0 reset(ar_t& ar) {
        buf::reset(ar.buf);
        hashtab::reset(ar.catalog);
    }

    status_t init(ar_t& ar, alloc_t* alloc) {
        ar.alloc = alloc;
        buf::init(ar.buf, ar.alloc);
        hashtab::init(ar.catalog, ar.alloc);
        return status_t::ok;
    }

    status_t read(ar_t& ar, const path_t& path) {
        auto status = buf::load(ar.buf, path);
        if (!OK(status))
            return status_t::read_error;
        return status_t::ok;
    }

    status_t write(ar_t& ar, const path_t& path) {
        return status_t::ok;
    }
}
