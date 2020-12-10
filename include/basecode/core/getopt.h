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

#include <basecode/core/str.h>
#include <basecode/core/array.h>

namespace basecode {
    enum class arg_type_t : u8 {
        none,
        flag,
        string,
        signed_integer,
        floating_point,
        unsigned_integer,
    };

    struct option_t final {
        str::slice_t            name;
        str::slice_t            desc;
        arg_type_t              type;
        struct {
            u32                 min;
            u32                 max;
        }                       required;
    };

    union arg_subclass_t final {
        b8                      flag;
        str::slice_t            string;
        s64                     signed_int;
        u64                     unsigned_int;
        f64                     floating_point;
    };

    struct arg_t final {
        option_t*               option;
        arg_subclass_t          subclass;
        u32                     pos;
        arg_type_t              type;
    };

    using arg_array_t           = array_t<arg_t>;
    using option_array_t        = array_t<option_t>;

    struct getopt_t final {
        alloc_t*                alloc;
        const s8**              argv;
        arg_array_t             args;
        option_array_t          opts;
        s32                     argc;
    };

    namespace getopt {
        enum class status_t : u32 {
            ok                  = 0,
        };

        u0 free(getopt_t& opt);

        status_t init(getopt_t& opt, s32 argc, const s8** argv, alloc_t* alloc = context::top()->alloc);
    }
}
