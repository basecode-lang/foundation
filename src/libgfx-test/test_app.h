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
#include <basecode/gfx/imgui/imgui.h>
#include <basecode/gfx/imgui/imgui_internal.h>
#include <basecode/gfx/imgui/imgui_memory_editor.h>

namespace basecode {
    using namespace gfx::tool;

    struct test_app_t final {
        alloc_t*                alloc;
        gfx::msg_stack_t        msg_stack;
        prop_editor_t           prop_editor;
        alloc_win_t             alloc_window;
        errors_win_t            errors_window;
        strings_win_t           strings_window;
        MemoryEditor            memory_editor;
        s8                      buf[128];
        b8                      show_fps;
        b8                      ffi_visible;
        b8                      jobs_visible;
        b8                      timers_visible;
        b8                      threads_visible;
        b8                      scm_env_visible;
        b8                      scm_repl_visible;
        b8                      obj_pools_visible;
    };

    s32 run(s32 argc, const s8** argv);
}
