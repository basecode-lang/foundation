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
#include <basecode/gfx/imgui/imgui_internal.h>

namespace basecode::gfx {
    struct input_text_callback_t {
        str_t*                  str;
        ImGuiInputTextCallback  chain;
        u0*                     chain_user_data;
    };

    static s32 input_text_callback(ImGuiInputTextCallbackData* data) {
        auto user_data = (input_text_callback_t*)data->UserData;
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            str_t* str = user_data->str;
            str::resize(*str, data->BufTextLen);
            data->Buf = (s8*) str::c_str(*str);
        } else if (user_data->chain) {
            data->UserData = user_data->chain_user_data;
            return user_data->chain(data);
        }
        return 0;
    }

    u0 end_status_bar() {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;

        if (ImGui::NavMoveRequestButNoResultYet()
        &&  (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right)
        &&  (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu)) {
            ImGuiWindow* nav_earliest_child = g.NavWindow;
            while (nav_earliest_child->ParentWindow
                   && (nav_earliest_child->ParentWindow->Flags
                       & ImGuiWindowFlags_ChildMenu)) {
                nav_earliest_child = nav_earliest_child->ParentWindow;
            }
            if (nav_earliest_child->ParentWindow == window
            &&  nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal
            &&  g.NavMoveRequestForward == ImGuiNavForward_None)
            {
                const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
                IM_ASSERT(window->DC.NavLayerActiveMaskNext & (1 << layer));
                ImGui::FocusWindow(window);
                ImGui::SetNavID(window->NavLastIds[layer],
                                layer,
                                0,
                                window->NavRectRel[layer]);
                g.NavDisableHighlight = true;
                g.NavDisableMouseHover = g.NavMousePosDirty = true;
                g.NavMoveRequestForward = ImGuiNavForward_ForwardQueued;
                ImGui::NavMoveRequestCancel();
            }
        }

        IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar);
        IM_ASSERT(window->DC.MenuBarAppending);
        ImGui::PopClipRect();
        ImGui::PopID();
        window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x;
        g.GroupStack.back().EmitItem = false;
        ImGui::EndGroup(); // Restore position on layer 0
        window->DC.LayoutType = ImGuiLayoutType_Vertical;
        window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
        window->DC.MenuBarAppending = false;

        if (g.CurrentWindow == g.NavWindow
        &&  g.NavLayer == ImGuiNavLayer_Main
        &&  !g.NavAnyRequest) {
            ImGui::FocusTopMostWindowUnderOne(g.NavWindow, nullptr);
        }

        ImGui::End();
    }

    b8 begin_status_bar() {
        ImGuiContext& g = *GImGui;
        ImGuiViewportP* viewport = (ImGuiViewportP*)(u0*)ImGui::GetMainViewport();

        ImGui::SetCurrentViewport(nullptr, viewport);

        g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 2.0f);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar
                                        | ImGuiWindowFlags_NoSavedSettings
                                        | ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        f32 height = ImGui::GetFrameHeight() + 2.0f;
        b8 is_open = ImGui::BeginViewportSideBar("##main_status_bar",
                                                 viewport,
                                                 ImGuiDir_Down,
                                                 height,
                                                 window_flags);
        if (is_open) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems)
                return false;
            if (!(window->Flags & ImGuiWindowFlags_MenuBar))
                return false;

            window->Pos.y -= 2;

            IM_ASSERT(!window->DC.MenuBarAppending);
            ImGui::BeginGroup();
            ImGui::PushID("##status_bar");

            ImRect bar_rect = window->MenuBarRect();
            bar_rect.Min.y += 2;
            bar_rect.Max.y += 2;
            const auto border_size = ImMax(window->WindowRounding,
                                           window->WindowBorderSize);
            ImRect clip_rect(IM_ROUND(bar_rect.Min.x + window->WindowBorderSize),
                             IM_ROUND(bar_rect.Min.y + window->WindowBorderSize),
                             IM_ROUND(ImMax(bar_rect.Min.x,
                                            bar_rect.Max.x - border_size)),
                             IM_ROUND(bar_rect.Max.y));
            clip_rect.ClipWith(window->OuterRectClipped);
            ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);

            window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(
                bar_rect.Min.x + window->DC.MenuBarOffset.x,
                bar_rect.Min.y + window->DC.MenuBarOffset.y);
            window->DC.LayoutType = ImGuiLayoutType_Horizontal;
            window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
            window->DC.MenuBarAppending = true;
            ImGui::AlignTextToFramePadding();
            auto draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImVec2(bar_rect.Min.x, bar_rect.Min.y + 2),
                                     ImVec2(bar_rect.Max.x, bar_rect.Min.y + 4),
                                     IM_COL32_BLACK);
        } else {
            ImGui::End();
        }

        ImGui::PopStyleVar(1);

        return is_open;
    }

    b8 buffering_bar(const s8* label,
                     f32 value,
                     const ImVec2& size_arg,
                     const ImU32& bg_col,
                     const ImU32& fg_col) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        const f32 circleStart = size.x * 0.7f;
        const f32 circleEnd = size.x;
        const f32 circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min,
                                        ImVec2(pos.x + circleStart, bb.Max.y),
                                        bg_col);
        window->DrawList->AddRectFilled(bb.Min,
                                        ImVec2(pos.x + circleStart*value, bb.Max.y),
                                        fg_col);

        const f32 t = g.Time;
        const f32 r = size.y / 2;
        const f32 speed = 1.5f;

        const f32 a = speed*0;
        const f32 b = speed*0.333f;
        const f32 c = speed*0.666f;

        const f32 o1 = (circleWidth + r) * (t + a - speed * (int) ((t + a) / speed))
                       / speed;
        const f32 o2 = (circleWidth + r) * (t + b - speed * (int) ((t + b) / speed))
                       / speed;
        const f32 o3 = (circleWidth + r) * (t + c - speed * (int) ((t + c) / speed))
                       / speed;

        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r),
                                          r,
                                          bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r),
                                          r,
                                          bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r),
                                          r,
                                          bg_col);

        return true;
    }

    b8 spinner(const s8* label,
               f32 radius,
               s32 thickness,
               const ImU32& color) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        window->DrawList->PathClear();

        s32 num_segments = 30;
        s32 start        = abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

        const f32 a_min = IM_PI*2.0f * ((f32)start) / (f32)num_segments;
        const f32 a_max = IM_PI*2.0f * ((f32)num_segments-3) / (f32)num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);

        for (s32 i = 0; i < num_segments; i++) {
            const float a = a_min + ((f32) i / (f32) num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
                                                centre.y + ImSin(a + g.Time * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);

        return true;
    }

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

    b8 input_text(const s8* label,
                  str_t* str,
                  ImGuiInputTextFlags flags,
                  ImGuiInputTextCallback callback,
                  u0* user_data) {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        input_text_callback_t cb;
        cb.str             = str;
        cb.chain           = callback;
        cb.chain_user_data = user_data;
        return ImGui::InputText(label,
                                (s8*) str::c_str(*str),
                                str->capacity + 1,
                                flags,
                                input_text_callback,
                                &cb);
    }

    b8 input_text_multiline(const s8* label,
                            str_t* str,
                            const ImVec2& size,
                            ImGuiInputTextFlags flags,
                            ImGuiInputTextCallback callback,
                            u0* user_data) {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        input_text_callback_t cb;
        cb.str             = str;
        cb.chain           = callback;
        cb.chain_user_data = user_data;
        return ImGui::InputTextMultiline(label,
                                         (s8*) str::c_str(*str),
                                         str->capacity + 1,
                                         size,
                                         flags,
                                         input_text_callback,
                                         &cb);
    }

    b8 input_text_with_hint(const s8* label,
                            const s8* hint,
                            str_t* str,
                            ImGuiInputTextFlags flags,
                            ImGuiInputTextCallback callback,
                            u0* user_data) {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        input_text_callback_t cb;
        cb.str             = str;
        cb.chain           = callback;
        cb.chain_user_data = user_data;
        return ImGui::InputTextWithHint(label,
                                        hint,
                                        (s8*) str::c_str(*str),
                                        str->capacity + 1,
                                        flags,
                                        input_text_callback,
                                        &cb);
    }
}
