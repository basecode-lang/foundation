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

#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/slice.h>
#include <basecode/core/format.h>

namespace basecode {
    struct baby_name_t final {
        s8                      sex;
        s8                      year[5];
        s8                      state[3];
    };

    struct payload_t final {
        u0*                     ptr;
        u32                     offset;
    };

    struct name_record_t final {
        u32                     idx;
        u32                     len;
    };
}

FORMAT_TYPE(basecode::payload_t, format_to(ctx.out(), "[ptr: {}, offset: {}]", data.ptr, data.offset));
FORMAT_TYPE(basecode::baby_name_t, format_to(ctx.out(), "[state: {}, year: {}, sex: {}]", data.state, data.year, data.sex));
