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

#include <basecode/gfx/gfx.h>
#include <basecode/gfx/imgui/imgui.h>
#include <basecode/gfx/imgui/imgui_internal.h>

namespace basecode::gfx {
    b8 menu_item_with_icon(const s8* icon,
                           const s8* label,
                           const s8* shortcut,
                           b8* p_selected,
                           b8 enabled) {
        f32 icon_max_width = ImGui::GetFontSize() * 1.5;
        if (icon) {
            const auto icon_size = ImGui::CalcTextSize(icon);
            const f32 icon_pos_x = ImGui::GetCursorPosX()
                                   + ((icon_max_width - icon_size.x) / 2);
            ImGui::SetCursorPosX(icon_pos_x);
            ImGui::TextUnformatted(icon);
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(icon_max_width * 1.5);
        return ImGui::MenuItem(label, shortcut, p_selected, enabled);
    }

    b8 splitter(b8 split_vertically,
                f32 thickness,
                f32* size1,
                f32* size2,
                f32 min_size1,
                f32 min_size2,
                f32 splitter_long_axis_size) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiID id = window->GetID("##Splitter");
        ImRect bb;
        bb.Min = window->DC.CursorPos
                 + (split_vertically ? ImVec2(*size1, 0.0f) :
                    ImVec2(0.0f, *size1));
        bb.Max = bb.Min + ImGui::CalcItemSize(split_vertically ?
                                              ImVec2(thickness,
                                                     splitter_long_axis_size) :
                                              ImVec2(splitter_long_axis_size,
                                                     thickness),
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

    u0 text_right_align(const s8* text_begin, const s8* text_end) {
        const auto text_size = ImGui::CalcTextSize(text_begin, text_end);
        const auto rect_size = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (rect_size.x - text_size.x));
        ImGui::TextUnformatted(text_begin);
    }

    b8 begin_menu_with_icon(const s8* icon, const s8* label, b8 enabled) {
        f32 icon_max_width = ImGui::GetFontSize() * 1.5;
        if (icon) {
            const auto icon_size = ImGui::CalcTextSize(icon);
            const f32 icon_pos_x = ImGui::GetCursorPosX()
                                   + ((icon_max_width - icon_size.x) / 2);
            ImGui::SetCursorPosX(icon_pos_x);
            ImGui::TextUnformatted(icon);
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(icon_max_width * 1.5);
        return ImGui::BeginMenu(label, enabled);
    }
}
