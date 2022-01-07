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
#include <basecode/gfx/icons.h>
#include <basecode/core/string.h>
#include <basecode/gfx/tool/strings.h>
#include <basecode/gfx/imgui/imgui_internal.h>

namespace basecode::gfx::tool::strings {
    u0 free(strings_win_t& win) {
        array::free(win.pairs);
    }

    b8 draw(strings_win_t& win) {
        if (!win.visible)
            return false;
        auto is_open = gfx::begin_tool_window(*win.app->icons_atlas,
                                              ICONS_DATASHEET_VIEW,
                                              "Strings",
                                              &win.visible);
        if (is_open) {
            const auto region_size = ImGui::GetContentRegionAvail();
            if (region_size.x != win.size.x
            ||  region_size.y != win.size.y) {
                win.size = region_size;
            }
            if (gfx::begin_tool_bar()) {
                gfx::end_tool_bar();
            }
            s8 buf[32];
            if (ImGui::BeginTable("localized",
                                  5,
                                  ImGuiTableFlags_RowBg
                                  | ImGuiTableFlags_ScrollY
                                  | ImGuiTableFlags_Resizable
                                  | ImGuiTableFlags_PreciseWidths
                                  | ImGuiTableFlags_NoBordersInBody,
                                  ImVec2(region_size.x, -1))) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Locale");
                ImGui::TableSetupColumn("Intern ID");
                ImGui::TableSetupColumn("Hash");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();
                const auto& localized = string::system::localized();
                array::reset(win.pairs);
                hashtab::pairs(localized, win.pairs);
                std::sort(win.pairs.begin(), win.pairs.end());
                ImGuiListClipper clipper;
                clipper.Begin(win.pairs.size);
                while (clipper.Step()) {
                    for (s32 row = clipper.DisplayStart;
                         row < clipper.DisplayEnd;
                         ++row) {
                        const auto& pair = win.pairs[row];
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushID(pair.key.id);
                        ImFormatString(buf, 32, "%d", pair.key.id);
                        ImGui::Selectable(buf);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", pair.key.locale);
                        ImGui::TableSetColumnIndex(2);
                        auto& r = pair.value;
                        ImGui::Text("%d", r.id);
                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("0x%016llX", r.hash);
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%s", r.slice.data);
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
        return is_open;
    }

    u0 init(strings_win_t& win, app_t* app, alloc_t* alloc) {
        win.app     = app;
        win.visible = true;
        array::init(win.pairs, alloc);
    }
}
