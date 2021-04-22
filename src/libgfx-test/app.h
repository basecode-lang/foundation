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

#include <basecode/core/types.h>
#include <basecode/gfx/imgui/imgui.h>
#include <basecode/gfx/alloc_window.h>
#include <basecode/gfx/imgui/imgui_internal.h>
#include <basecode/gfx/imgui/imgui_memory_editor.h>

namespace basecode {
    struct test_app_t final {
        alloc_t*                alloc;
        alloc_window_t          alloc_window;
        struct {
            MemoryEditor        window;
            u0*                 data;
            u32                 size;
            b8                  visible;
        }                       memory_editor;
        s8                      buf[128];
        b8                      show_fps;
        b8                      ffi_visible;
        b8                      jobs_visible;
        b8                      errors_visible;
        b8                      timers_visible;
        b8                      threads_visible;
        b8                      strings_visible;
        b8                      scm_env_visible;
        b8                      scm_repl_visible;
        b8                      obj_pools_visible;
    };

    s32 run(s32 argc, const s8** argv);
}
