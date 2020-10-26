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

#include <basecode/core/types.h>
#include <basecode/core/slice.h>

namespace basecode {
    struct stopwatch_t final {
        u64                     end;
        u64                     start;
    };

    namespace stopwatch {
        u0 init(stopwatch_t& w);

        u0 stop(stopwatch_t& w);

        u0 start(stopwatch_t& w);

        u64 elapsed(stopwatch_t& w);

        u0 print_elapsed(str::slice_t label, s32 width, stopwatch_t& w);
    }
}
