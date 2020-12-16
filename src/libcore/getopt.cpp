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

#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/getopt.h>
#include <basecode/core/format.h>

namespace basecode::getopt {
    u0 free(getopt_t& opt) {
        array::free(opt.args);
        array::free(opt.opts);
    }

    status_t parse(getopt_t& opt) {
        UNUSED(opt);
        return status_t::ok;
    }

    u0 format_help(getopt_t& opt, str_t& buf) {
        path_t self{};
        path::init(self);
        path::set(self, opt.argv[0]);
        defer(path::free(self));

        const auto self_stem = path::stem(self);
        str_buf_t sb{&buf};
        format::format_to(sb, "usage: {} ", self_stem);
    }

    status_t init(getopt_t& opt, s32 argc, const s8** argv, alloc_t* alloc) {
        opt.alloc = alloc;
        opt.argc  = argc;
        opt.argv  = argv;
        array::init(opt.args, opt.alloc);
        array::init(opt.opts, opt.alloc);
        return status_t::ok;
    }
}
