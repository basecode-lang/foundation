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

#include "types.h"

#define COL32_R_SHIFT    ((u32)0)
#define COL32_G_SHIFT    ((u32)8)
#define COL32_B_SHIFT    ((u32)16)
#define COL32_A_SHIFT    ((u32)24)
#define COL32_A_MASK     0xff000000
#define COL32(R,G,B,A)   (((u32)(A)<<COL32_A_SHIFT) | ((u32)(B)<<COL32_B_SHIFT) | ((u32)(G)<<COL32_G_SHIFT) | ((u32)(R)<<COL32_R_SHIFT))

struct GLFWwindow;

namespace basecode {
    struct size_t final {
        f32             w;
        f32             h;
    };

    struct point_t final {
        f32             x;
        f32             y;
    };

    struct rect_t final {
        f32             x;
        f32             y;
        f32             w;
        f32             h;
    };

    struct vector3_t final {
        f32             x;
        f32             y;
        f32             z;
    };

    struct vector4_t final {
        f32             x;
        f32             y;
        f32             z;
        f32             w;
    };

    struct color_t final {
        u8              r;
        u8              g;
        u8              b;
        u8              a;

        explicit operator u32() const { return COL32(r, g, b, a); }
    };

    struct window_t final {
        GLFWwindow*     window{};
        s32             x = -1;
        s32             y = -1;
        s32             width = 1280;
        s32             height = 1024;
        s32             iconified = 0;
        s32             maximized = 0;
        s32             min_width = 800;
        s32             min_height = 600;
    };

    namespace gfx {
    }
}
