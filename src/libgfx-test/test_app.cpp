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
#include <basecode/gfx/msg_stack.h>
#include <basecode/core/memory/meta.h>
#include <basecode/gfx/fonts/IconsFontAwesome5.h>
#include "test_app.h"

namespace basecode {
    static test_app_t           s_test_app          {};
    static usize                s_scratch_free      {};
    static usize                s_scratch_alloc     {};

    b8 on_render(gfx::app_t& app) {
        auto& io = ImGui::GetIO();

        gfx::msg_stack::draw(s_test_app.msg_stack);

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
                                             &s_test_app.memory_editor.Open,
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
            if (ImGui::BeginMenu("Test")) {
                if (gfx::menu_item_with_icon(ICON_FA_BEER,
                                             "Enqueue to Message Stack",
                                             nullptr,
                                             nullptr,
                                             true)) {
                    gfx::msg_stack::push(s_test_app.msg_stack,
                                         "This is a test message!"_ss);
                }
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

        gfx::tool::alloc::draw(s_test_app.alloc_window);
        if (s_test_app.memory_editor.Open) {
            u0* mem     {};
            u32 size    {};
            s_scratch_free  = {};
            s_scratch_alloc = {};
            s_test_app.memory_editor.HighlightFn = {};
            if (s_test_app.alloc_window.selected) {
                auto tracked = s_test_app.alloc_window.selected->tracked;
                switch (tracked->system->type) {
                    case alloc_type_t::base:
                        break;
                    case alloc_type_t::bump:
                        break;
                    case alloc_type_t::page: {
                        auto sc = &tracked->subclass.page;
                        mem  = sc->head;
                        size = 16;
                        break;
                    }
                    case alloc_type_t::slab: {
                        auto sc = &tracked->subclass.slab;
                        mem  = sc->head;
                        size = 64;
                        break;
                    }
                    case alloc_type_t::temp:
                        break;
                    case alloc_type_t::proxy:
                        break;
                    case alloc_type_t::trace:
                        break;
                    case alloc_type_t::stack:
                        break;
                    case alloc_type_t::buddy:
                        break;
                    case alloc_type_t::scratch: {
                        auto sc = &tracked->subclass.scratch;
                        mem             = sc->begin;
                        size            = sc->size;
                        s_scratch_free  = usize(sc->free);
                        s_scratch_alloc = usize(sc->alloc);
                        s_test_app.memory_editor.HighlightFn = [](const u8* mem,
                                                                  usize addr) -> b8 {
                            usize mem_addr = usize(mem + addr);
                            return (mem_addr >= s_scratch_free && mem_addr <= s_scratch_free + 1)
                                   || (mem_addr >= s_scratch_alloc && mem_addr <= s_scratch_alloc + 1);
                        };
                        break;
                    }
                    case alloc_type_t::dlmalloc: {
                        break;
                    }
                    default:
                        break;
                }
            }
            s_test_app.memory_editor.DrawWindow("Memory Editor",
                                                mem,
                                                mem ? size : 0 );
        }

        if (gfx::begin_status_bar()) {
            ImGui::TextUnformatted("Ready.");
            gfx::end_status_bar();
        }

        return true;
    }

    s32 run(s32 argc, const s8** argv) {
//        ImVec4 colors[] = {
//            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
//            ImVec4(0.55f, 0.55f, 0.55f, 1.0f),
//            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
//            ImVec4(.9f, .1f, .1f, 1.0f),
//            ImVec4(.55f, .55f, .55f, 1.0f),
//            ImVec4(.4f, .4f, .4f, .55f),
//            ImVec4(.55f, .55f, .55f, 1.0f),
//            ImVec4(1.0 - .02f, 1.0 - .02f, 1.0 - .02f, 0.0f),
//            ImVec4(.13f, .4f, .13f, 1.0f),
//            ImVec4(.13f, 0.6f, .13f, 1.0f),
//            ImVec4(130.0f / 255.0f, 140.0f / 255.0f, 230.0f / 255.0f, 1.0f),
//            ImVec4(1.0f, 1.0f, 1.0f, .9f),
//            ImVec4(.85f, .85f, .85f, 1.0f),
//            ImVec4(.80f, .80f, .80f, 1.0f),
//            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
//            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
//            ImVec4(.49f, 0.60f, 0.45f, 1.0f),
//            ImVec4(.2f, 0.8f, 0.2f, 1.0f),
//            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
//            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
//            ImVec4(0.1f, .4f, .1f, 1.0f),
//            ImVec4(0.1f, .2f, .3f, 1.0f),
//            ImVec4(0.2f, .2f, .1f, 1.0f),
//            ImVec4(0.1f, .3f, .2f, 1.0f),
//            ImVec4(0.1f, .1f, .4f, 1.0f),
//            ImVec4(0.2f, .2f, .2f, 1.0f),
//            ImVec4(0.89f, .2f, .15f, 1.0f),
//            ImVec4(0.15f, .2f, .89f, 1.0f),
//            ImVec4(0.15f, .85f, .15f, 1.0f),
//            ImVec4(.55f, .55f, .55f, 1.0f),
//            ImVec4(.9f, .1f, .1f, 1.0f),
//            ImVec4(.5f, .5f, .5f, 1.0f),
//            ImVec4(.55f, .55f, .55f, 1.0f),
//            ImVec4(.9f, .1f, .1f, 1.0f),
//            ImVec4(.8f, .8f, .8f, 1.0f),
//            ImVec4(0.8f, .4f, .05f, 1.0f),
//        };
//
//        for (ImVec4 color : colors) {
//            auto col32 = ImGui::ColorConvertFloat4ToU32(color);
//            format::print("IM_COL32({}, {}, {}, {})\n",
//                          ((col32 >> IM_COL32_R_SHIFT) & 0xff),
//                          ((col32 >> IM_COL32_G_SHIFT) & 0xff),
//                          ((col32 >> IM_COL32_B_SHIFT) & 0xff),
//                          ((col32 >> IM_COL32_A_SHIFT) & 0xff));
//        }
//
        gfx::app_t app{};
        app.title      = string::interned::fold("Gfx Library Test Harness");
        app.on_render  = on_render;
        app.short_name = string::interned::fold("libgfx-test");
        gfx::app::init(app);

        s_test_app.alloc = memory::system::main_alloc();
        gfx::tool::alloc::init(s_test_app.alloc_window,
                               &app,
                               s_test_app.alloc);

        gfx::msg_stack::init(s_test_app.msg_stack, app.large_font, 0);

        auto status = gfx::app::run(app);
        s32 rc = !OK(status);

        gfx::tool::alloc::free(s_test_app.alloc_window);
        gfx::msg_stack::free(s_test_app.msg_stack);
        gfx::app::free(app);

        return rc;
    }
}
