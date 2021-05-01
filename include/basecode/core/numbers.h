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

namespace basecode::numbers {
    namespace fp {
        status_t parse(str::slice_t value, f32& out);

        status_t parse(str::slice_t value, f64& out);
    }

    namespace integer {
        status_t parse(str::slice_t value, u8 radix, u32& out);

        status_t parse(str::slice_t value, u8 radix, s32& out);

        status_t parse(str::slice_t value, u8 radix, u64& out);

        status_t parse(str::slice_t value, u8 radix, s64& out);
    }
}
