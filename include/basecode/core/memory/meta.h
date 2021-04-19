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

#include <basecode/core/plot.h>

namespace basecode {
    struct alloc_info_t;

    using alloc_info_array_t    = array_t<alloc_info_t*>;

    enum class plot_mode_t : u8 {
        none,
        rolled,
        scrolled,
    };

    struct alloc_info_t final {
        alloc_t*                alloc;
        alloc_info_t*           parent;
        alloc_t*                tracked;
        alloc_info_array_t      children;
        union {
            rolled_view_t       rolled;
            scrolled_view_t     scrolled;
        }                       plot;
        plot_mode_t             mode;
    };

    namespace memory::meta {
        namespace system {
            u0 fini();

            u0 update(f32 dt);

            u0 track(alloc_t* alloc);

            u0 untrack(alloc_t* alloc);

            u0 stop_plot(alloc_info_t* info);

            const alloc_info_array_t& roots();

            u0 init(alloc_t* alloc = context::top()->alloc);

            u0 start_plot(alloc_info_t* info, plot_mode_t mode);
        }

        namespace alloc_info {
            u0 free(alloc_info_t& info);

            u0 stop_plot(alloc_info_t& info);

            u0 init(alloc_info_t& info, alloc_t* alloc);

            u0 append_point(alloc_info_t& info, f32 x, f32 y);

            u0 start_plot(alloc_info_t& info, plot_mode_t mode);
        }
    }
}
