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
#include <basecode/core/string.h>
#include <basecode/gfx/fonts/IconsFontAwesome5.h>
#include "app.h"

namespace basecode {
    static test_app_t           s_test_app{};

    b8 on_render(app_t& app) {
        auto& io = ImGui::GetIO();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                if (gfx::begin_menu_with_icon(ICON_FA_TOOLS, "Tool Windows")) {
                    gfx::menu_item_with_icon(ICON_FA_MEMORY,
                                             "Allocators",
                                             nullptr,
                                             &s_test_app.alloc_window.visible,
                                             true);
                    gfx::menu_item_with_icon(ICON_FA_DOLLAR_SIGN,
                                             "Strings",
                                             nullptr,
                                             &s_test_app.strings_visible,
                                             true);
                    gfx::menu_item_with_icon(ICON_FA_EXCLAMATION_CIRCLE,
                                             "Errors",
                                             nullptr,
                                             &s_test_app.errors_visible,
                                             true);
                    if (gfx::begin_menu_with_icon(nullptr, "Scheme")) {
                        gfx::menu_item_with_icon(ICON_FA_TERMINAL,
                                                 "REPL",
                                                 nullptr,
                                                 &s_test_app.scm_repl_visible,
                                                 true);
                        ImGui::Separator();
                        gfx::menu_item_with_icon(ICON_FA_STREAM,
                                                 "Environment",
                                                 nullptr,
                                                 &s_test_app.scm_env_visible,
                                                 true);
                        ImGui::EndMenu();
                    }
                    gfx::menu_item_with_icon(ICON_FA_TASKS,
                                             "Jobs",
                                             nullptr,
                                             &s_test_app.jobs_visible,
                                             true);
                    gfx::menu_item_with_icon(nullptr,
                                             "Threads",
                                             nullptr,
                                             &s_test_app.threads_visible,
                                             true);
                    gfx::menu_item_with_icon(nullptr,
                                             "FFI",
                                             nullptr,
                                             &s_test_app.ffi_visible,
                                             true);
                    gfx::menu_item_with_icon(nullptr,
                                             "Object Pools",
                                             nullptr,
                                             &s_test_app.obj_pools_visible,
                                             true);
                    gfx::menu_item_with_icon(ICON_FA_CLOCK,
                                             "Timers",
                                             nullptr,
                                             &s_test_app.timers_visible,
                                             true);
                    ImGui::Separator();
                    gfx::menu_item_with_icon(nullptr,
                                             "Memory Editor",
                                             nullptr,
                                             &s_test_app.memory_editor.visible,
                                             true);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                gfx::menu_item_with_icon(ICON_FA_CHART_BAR,
                                        "Frame Time & Rate",
                                        nullptr,
                                        &s_test_app.show_fps,
                                        true);
                ImGui::EndMenu();
            }
            if (s_test_app.show_fps) {
                const auto size = ImGui::GetContentRegionAvail();
                ImFormatString(s_test_app.buf,
                               128,
                               ICON_FA_CHART_BAR "  app average %.2f ms/frame (%.2f FPS)",
                               1000.0f / io.Framerate,
                               io.Framerate);
                auto text_size = ImGui::CalcTextSize(s_test_app.buf);
                ImGui::SameLine(size.x - (text_size.x - 40));
                ImGui::TextUnformatted(s_test_app.buf);
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

        s_test_app
            .memory_editor
            .window
            .DrawWindow("Memory Editor",
                        &s_test_app.memory_editor.data,
                        s_test_app.memory_editor.size);
        alloc_window::draw(s_test_app.alloc_window);
        return true;
    }

    s32 run(s32 argc, const s8** argv) {
        s_test_app.alloc = memory::system::main_alloc();
        alloc_window::init(s_test_app.alloc_window, s_test_app.alloc);

        app_t app{};
        app::init(app);
        app.title      = string::interned::fold("Gfx Library Test Harness");
        app.on_render  = on_render;
        app.short_name = string::interned::fold("libgfx-test");

        auto status = app::run(app);
        s32 rc = !OK(status);

        app::free(app);
        alloc_window::free(s_test_app.alloc_window);

        return rc;
    }
}
