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
    struct data_point_t;

    using data_point_array_t    = array_t<data_point_t>;

    struct data_point_t final {
        f32                     x, y;
    };

    struct rolled_view_t final {
        data_point_array_t      values;
        f32                     span;
        f32                     time;
        f32                     max_y;
    };

    struct scrolled_view_t final {
        data_point_array_t      values;
        f32                     time;
        f32                     max_y;
        s32                     offset;
        s32                     max_size;
    };

    namespace plot {
        namespace rolled {
            u0 free(rolled_view_t& view);

            u0 init(rolled_view_t& view,
                    f32 span = 10.0f,
                    u32 capacity = 1000,
                    alloc_t* alloc = context::top()->alloc);

            u0 append_point(rolled_view_t& view, f32 x, f32 y);
        }

        namespace scrolled {
            u0 free(scrolled_view_t& view);

            u0 erase(scrolled_view_t& view);

            u0 init(scrolled_view_t& view,
                    s32 offset = 0,
                    s32 max_size = 1000,
                    alloc_t* alloc = context::top()->alloc);

            u0 append_point(scrolled_view_t& view, f32 x, f32 y);
        }
    }
}
