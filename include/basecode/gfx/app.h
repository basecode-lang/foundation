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

#include <imgui.h>
#include <basecode/gfx/gfx.h>
#include <basecode/core/str.h>

namespace basecode {
    using render_callback_t = std::function<b8 ()>;

    struct app_t final {
        alloc_t*                alloc;
        render_callback_t       on_render;
        str_t                   scratch;
        window_t                window;
        vector4_t               bg_color;
        ImGuiID                 dock_root;
    };

    namespace app {
        enum class status_t : u32 {
            ok,
            error,
            gl3w_init_failure,
            glfw_init_failure,
        };

        u0 free(app_t& app);

        status_t run(app_t& app);

        status_t init(app_t& app, alloc_t* alloc = context::top()->alloc);
    }
}
