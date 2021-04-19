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

#include <imgui.h>
#include <basecode/gfx/gfx.h>
#include <basecode/core/str.h>
#include <basecode/gfx/implot/implot.h>

namespace basecode {
    struct alloc_window_t final {
        alloc_t*                alloc;
        ImPlotContext*          ctx;
        str_t                   scratch;
        b8                      visible;
        b8                      mem_editor;
    };

    namespace alloc_window {
        u0 free(alloc_window_t& win);

        u0 draw(alloc_window_t& win);

        u0 init(alloc_window_t& win, alloc_t* alloc = context::top()->alloc);
    }
}
