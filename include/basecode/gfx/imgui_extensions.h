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

#include <basecode/gfx/imgui/imgui.h>
#include <basecode/gfx/imgui/imgui_internal.h>

namespace ImGui {
    bool Splitter(bool split_vertically,
                  float thickness,
                  float* size1,
                  float* size2,
                  float min_size1,
                  float min_size2,
                  float splitter_long_axis_size = -1.0f);

    void TextRightAlign(const char* text_begin, const char* text_end = {});
}
