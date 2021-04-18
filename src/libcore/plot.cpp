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

#include <basecode/core/plot.h>

namespace basecode::plot {
    namespace rolled {
        u0 free(rolled_view_t& view) {
            array::free(view.values);
        }

        u0 append_point(rolled_view_t& view, f32 x, f32 y) {
            if (y > view.max_y)
                view.max_y = y;
            f32 xmod = fmodf(x, view.span);
            if (!array::empty(view.values)
            &&  xmod < array::back(view.values)->x) {
                array::shrink(view.values, 0);
            }
            array::append(view.values, data_point_t{xmod, y});
        }

        u0 init(rolled_view_t& view, f32 span, u32 capacity, alloc_t* alloc) {
            view.span = span;
            view.time = view.max_y = {};
            array::init(view.values, alloc);
            array::reserve(view.values, capacity);
        }
    }

    namespace scrolled {
        u0 init(scrolled_view_t& view,
                s32 offset,
                s32 max_size,
                alloc_t* alloc) {
            view.offset   = offset;
            view.max_size = max_size;
            view.time     = view.max_y = {};
            array::init(view.values, alloc);
            array::reserve(view.values, max_size);
        }

        u0 free(scrolled_view_t& view) {
            array::free(view.values);
        }

        u0 erase(scrolled_view_t& view) {
            if (view.values.size > 0) {
                array::shrink(view.values, 0);
                view.offset = {};
            }
        }

        u0 append_point(scrolled_view_t& view, f32 x, f32 y) {
            if (y > view.max_y)
                view.max_y = y;
            const s32 size = view.values.size;
            if (size < view.max_size) {
                array::append(view.values, data_point_t{x, y});
            } else {
                view.values[view.offset] = data_point_t{x, y};
                view.offset = (view.offset + 1) % view.max_size;
            }
        }
    }
}
