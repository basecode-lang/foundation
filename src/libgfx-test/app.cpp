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

#include <basecode/gfx/app.h>
#include <basecode/gfx/imgui/imgui.h>
#include <basecode/gfx/alloc_window.h>
#include <basecode/gfx/imgui/imgui_memory_editor.h>
#include "app.h"

namespace basecode {
    s32 run(s32 argc, const s8** argv) {
        static MemoryEditor s_mem_edit;
        static alloc_window_t s_alloc_win{};

        alloc_window::init(s_alloc_win);

        app_t app{};
        app::init(app);

        app.on_render = [&]() -> b8 {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Scheme")) {
                    ImGui::MenuItem("REPL", nullptr, nullptr, true);
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking
                                     | ImGuiWindowFlags_NoTitleBar
                                     | ImGuiWindowFlags_NoCollapse
                                     | ImGuiWindowFlags_NoBackground;
            auto vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                ImVec2(2.0f, 2.0f));
            ImGui::Begin("dock", nullptr, flags);
            ImGui::PopStyleVar();
            app.dock_root = ImGui::GetID("dock_space");
            ImGui::DockSpace(app.dock_root,
                             ImVec2(0, 0),
                             ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::End();

            s_mem_edit.DrawWindow("Memory Editor",
                                  &s_mem_edit,
                                  sizeof(MemoryEditor));
            alloc_window::draw(s_alloc_win);
            return true;
        };

        auto status = app::run(app);
        s32 rc = !OK(status);

        app::free(app);
        alloc_window::free(s_alloc_win);

        return rc;
    }
}
