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

#include <basecode/core/array.h>

namespace basecode {
    struct data_point_t final {
        f32                     x, y;
    };

    struct rolled_view_t final {
        array_t<data_point_t>   values;
        f32                     span;
        f32                     time;
    };

    struct scrolled_view_t final {
        array_t<data_point_t>   values;
        f32                     time;
        s32                     offset;
        s32                     max_size;
    };

    namespace plot {
        namespace rolled {
            u0 free(rolled_view_t& view);

            u0 append_point(rolled_view_t& view, f32 x, f32 y);

            u0 init(rolled_view_t& view, f32 span = 10.0f, u32 capacity = 1000, alloc_t* alloc = context::top()->alloc);
        }

        namespace scrolled {
            u0 free(scrolled_view_t& view);

            u0 erase(scrolled_view_t& view);

            u0 append_point(scrolled_view_t& view, f32 x, f32 y);

            u0 init(scrolled_view_t& view, s32 offset = 0, s32 max_size = 1000, alloc_t* alloc = context::top()->alloc);
        }
    }
}
