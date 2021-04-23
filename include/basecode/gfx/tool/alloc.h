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

namespace basecode::gfx::tool {
    struct alloc_win_t final {
        alloc_t*                alloc;
        app_t*                  app;
        ImPlotContext*          ctx;
        alloc_info_t*           selected;
        f32                     table_size;
        f32                     graph_size;
        f32                     height;
        b8                      visible;
        b8                      mem_editor;
    };

    namespace alloc {
        u0 free(alloc_win_t& win);

        b8 draw(alloc_win_t& win);

        u0 init(alloc_win_t& win,
                app_t* app,
                alloc_t* alloc = context::top()->alloc.main);
    }
}
