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

#include <basecode/gfx/tool/errors.h>

namespace basecode::gfx::tool::errors {
    u0 free(errors_win_t& win) {
        UNUSED(win);
    }

    b8 draw(errors_win_t& win) {
        UNUSED(win);
        return false;
    }

    u0 init(errors_win_t& win, app_t* app, alloc_t* alloc) {
        win.app     = app;
        win.visible = true;
    }
}
