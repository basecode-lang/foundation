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

#include <basecode/gfx/gfx.h>
#include <basecode/core/str.h>
#include <basecode/core/path.h>

namespace basecode::gfx {
    using render_callback_t = std::function<b8 (app_t&)>;

    struct app_t final {
        alloc_t*                alloc;
        ImFont*                 large_font;
        render_callback_t       on_render;
        str::slice_t            short_name;
        str::slice_t            title;
        str_t                   scratch;
        window_t                window;
        vec4_t                  bg_color;
        s64                     ticks;
        s32                     dock_root;
    };

    namespace app {
        u0 free(app_t& app);

        status_t run(app_t& app);

        status_t save_config(app_t& app);

        status_t load_config(app_t& app);

        status_t init(app_t& app,
                      alloc_t* alloc = context::top()->alloc.main);
    }
}
