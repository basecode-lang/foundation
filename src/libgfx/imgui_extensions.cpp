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

#include <basecode/gfx/imgui_extensions.h>

namespace ImGui {
    bool Splitter(bool split_vertically,
                  float thickness,
                  float* size1,
                  float* size2,
                  float min_size1,
                  float min_size2,
                  float splitter_long_axis_size) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiID id = window->GetID("##Splitter");
        ImRect bb;
        bb.Min = window->DC.CursorPos
                 + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
        bb.Max = bb.Min + ImGui::CalcItemSize(split_vertically ?
                                              ImVec2(thickness, splitter_long_axis_size) :
                                              ImVec2(splitter_long_axis_size, thickness),
                                              0.0f,
                                              0.0f);
        return ImGui::SplitterBehavior(bb,
                                       id,
                                       split_vertically ? ImGuiAxis_X : ImGuiAxis_Y,
                                       size1,
                                       size2,
                                       min_size1,
                                       min_size2,
                                       0.0f);
    }

    void TextRightAlign(const char* text_begin, const char* text_end) {
        const auto text_size = ImGui::CalcTextSize(text_begin, text_end);
        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - text_size.x);
        ImGui::TextUnformatted(text_begin);
    }
}
