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

#include <cstdint>

#define COL32_R_SHIFT    0
#define COL32_G_SHIFT    8
#define COL32_B_SHIFT    16
#define COL32_A_SHIFT    24
#define COL32_A_MASK     0xFF000000
#define COL32(R,G,B,A)   (((unsigned int)(A)<<COL32_A_SHIFT) | ((unsigned int)(B)<<COL32_B_SHIFT) | ((unsigned int)(G)<<COL32_G_SHIFT) | ((unsigned int)(R)<<COL32_R_SHIFT))

struct GLFWwindow;

namespace basecode::graphics {

    struct size_t final {
        float w{}, h{};
    };

    struct point_t final {
        float x{}, y{};
    };

    struct rect_t final {
        float x{}, y{};
        float w{}, h{};
    };

    struct vector3_t final {
        float x{}, y{}, z{};
    };

    struct vector4_t final {
        float x{}, y{}, z{}, w{};
    };

    struct color_t final {
        uint8_t r{}, g{}, b{}, a{};

        operator unsigned int() const {
            return COL32(r, g, b, a);
        }
    };

    struct window_t final {
        int x = -1;
        int y = -1;
        int width = 1280;
        int height = 1024;
        int iconified = 0;
        int maximized = 0;
        int min_width = 800;
        int min_height = 600;
        GLFWwindow* window{};
    };

}