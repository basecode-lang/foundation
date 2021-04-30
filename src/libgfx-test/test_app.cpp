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
#include <basecode/gfx/icons.h>
#include <basecode/core/string.h>
#include <basecode/gfx/msg_stack.h>
#include <basecode/gfx/tool/alloc.h>
#include <basecode/gfx/tool/errors.h>
#include <basecode/gfx/tool/strings.h>
#include <basecode/core/memory/meta.h>
#include <basecode/gfx/tool/prop_editor.h>
#include "test_app.h"

namespace basecode {
    [[maybe_unused]]
    constexpr u32               toolbar_height      {50};
    static test_app_t           s_test_app          {};
    static usize                s_scratch_free      {};
    static usize                s_scratch_alloc     {};

    static u0 draw_menu_bar(gfx::app_t& app) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                if (gfx::begin_menu_with_texture(*app.icons_atlas,
                                                 ICONS_TOOLBOX_WITH_TOOLS,
                                                 "Tool Windows")) {
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_RAM,
                        "Allocators",
                        nullptr,
                        &s_test_app.alloc_window.visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_DATASHEET_VIEW,
                        "Strings",
                        nullptr,
                        &s_test_app.strings_window.visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_LIST_ERRORS,
                        "Errors",
                        nullptr,
                        &s_test_app.errors_window.visible,
                        true);
                    if (gfx::begin_menu_with_texture(*app.icons_atlas,
                                                     ICONS_SCRIPT,
                                                     "Scheme")) {
                        gfx::menu_item_with_texture(
                            *app.icons_atlas,
                            ICONS_COMMAND_PROMPT,
                            "REPL",
                            nullptr,
                            &s_test_app.scm_repl_visible,
                            true);
                        ImGui::Separator();
                        gfx::menu_item_with_texture(
                            *app.icons_atlas,
                            ICONS_NAME_SPACE,
                            "Environment",
                            nullptr,
                            &s_test_app.scm_env_visible,
                            true);
                        ImGui::EndMenu();
                    }
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_PROCEDURE,
                        "Jobs",
                        nullptr,
                        &s_test_app.jobs_visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_PROCESS,
                        "Threads",
                        nullptr,
                        &s_test_app.threads_visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_CALL_STACK,
                        "FFI",
                        nullptr,
                        &s_test_app.ffi_visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_OBJECT_BROWSER,
                        "Object Pools",
                        nullptr,
                        &s_test_app.obj_pools_visible,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_TASK_SCHEDULE,
                        "Timers",
                        nullptr,
                        &s_test_app.timers_visible,
                        true);
                    ImGui::Separator();
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_PROCESSOR,
                        "Memory Editor",
                        nullptr,
                        &s_test_app.memory_editor.Open,
                        true);
                    gfx::menu_item_with_texture(
                        *app.icons_atlas,
                        ICONS_PROPERTY,
                        "Properties",
                        nullptr,
                        &s_test_app.prop_editor.visible,
                        true);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                gfx::menu_item_with_texture(
                    *app.icons_atlas,
                    ICONS_CHART_AXES,
                    "Frame Time & Rate",
                    nullptr,
                    &s_test_app.show_fps,
                    true);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Test")) {
                if (gfx::menu_item_with_texture(
                    *app.icons_atlas,
                    ICONS_COMMENT,
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
                auto& io = ImGui::GetIO();
                const auto size = ImGui::GetContentRegionAvail();
                ImFormatString(s_test_app.buf,
                               128,
                               "app average %.2f ms/frame (%.2f FPS)",
                               1000.0f / io.Framerate,
                               io.Framerate);
                auto text_size = ImGui::CalcTextSize(s_test_app.buf);
                auto x_pos = ImGui::GetCursorPosX();
                ImGui::SameLine();
                ImGui::SetCursorPosX(x_pos + (size.x - text_size.x));
                ImGui::TextUnformatted(s_test_app.buf);
            }
            ImGui::EndMenuBar();
        }
    }

    static u0 allocator_props(prop_editor_t* editor, alloc_t* alloc) {
        if (prop_editor::begin_nested(*editor, "##system", "System")) {
            if (prop_editor::read_only(*editor, "Type", memory::type_name(alloc->system->type))) {
                prop_editor::description(*editor,
                                         "The type of system used by this allocator."_ss);
            }
            switch (alloc->system->type) {
                case alloc_type_t::bump: {
                    auto sc = &alloc->subclass.bump;
                    if (prop_editor::read_only(*editor, "Buffer Pointer", (u0*) sc->buf)) {
                        prop_editor::description(*editor,
                                                 "Pointer to the backing memory this bump allocator "
                                                 "will parcel out."_ss);
                    }
                    if (prop_editor::read_only(*editor, "Buffer Offset", sc->offset)) {
                        prop_editor::description(*editor,
                                                 "The offset from the Buffer Pointer that "
                                                 "will be the base pointer for the next allocation."_ss);
                    }
                    if (prop_editor::read_only(*editor, "Buffer End Offset", sc->end_offset)) {
                        prop_editor::description(*editor,
                                                 "Any allocation that would exceed this offset forces "
                                                 "this system to request more memory from the backing allocator."_ss);
                    }
                    break;
                }
                case alloc_type_t::page: {
                    auto sc = &alloc->subclass.page;
                    prop_editor::read_only(*editor,
                                           "Page Size",
                                           sc->page_size);
                    prop_editor::read_only(*editor,
                                           "Number of Pages/Alloc",
                                           sc->num_pages);
                    prop_editor::read_only(*editor,
                                           "Page Cursor",
                                           (u0*) sc->cursor);
                    prop_editor::read_only(*editor,
                                           "Head Page",
                                           (u0*) sc->head);
                    prop_editor::read_only(*editor,
                                           "Tail Page",
                                           (u0*) sc->tail);
                    prop_editor::read_only(*editor,
                                           "Active Pages",
                                           sc->count);
                    break;
                }
                case alloc_type_t::slab: {
                    auto sc = &alloc->subclass.slab;
                    prop_editor::read_only(*editor,
                                           "Page Size",
                                           sc->page_size);
                    prop_editor::read_only(*editor,
                                           "Number of Pages/Slab",
                                           sc->num_pages);
                    prop_editor::read_only(*editor,
                                           "Head Slab",
                                           (u0*) sc->head);
                    prop_editor::read_only(*editor,
                                           "Tail Slab",
                                           (u0*) sc->tail);
                    prop_editor::read_only(*editor,
                                           "Buffer Size",
                                           sc->buf_size);
                    prop_editor::read_only(*editor,
                                           "Buffer Align",
                                           sc->buf_align);
                    prop_editor::read_only(*editor,
                                           "Number of Slabs",
                                           sc->count);
                    prop_editor::read_only(*editor,
                                           "Max Buffers/Slab",
                                           sc->buf_max_count);
                    break;
                }
                case alloc_type_t::proxy: {
                    auto sc = &alloc->subclass.proxy;
                    b8 owner = sc->owner;
                    prop_editor::read_only(*editor,
                                           "Owns Backing",
                                           owner);
                    break;
                }
                case alloc_type_t::stack: {
                    auto sc = &alloc->subclass.stack;
                    prop_editor::read_only(*editor,
                                           "Max Size",
                                           sc->max_size);
                    prop_editor::read_only(*editor,
                                           "Buffer Pointer",
                                           (u0*) sc->buf);
                    prop_editor::read_only(*editor,
                                           "Free Pointer",
                                           (u0*) sc->free);
                    break;
                }
                case alloc_type_t::buddy: {
                    auto sc = &alloc->subclass.buddy;
                    prop_editor::read_only(*editor,
                                           "Heap Size",
                                           sc->size);
                    prop_editor::read_only(*editor,
                                           "Min Allocation Size",
                                           sc->min_allocation);
                    prop_editor::read_only(*editor,
                                           "Heap Pointer",
                                           (u0*) sc->heap);
                    prop_editor::read_only(*editor,
                                           "Free Block List",
                                           (u0*) sc->free_blocks);
                    prop_editor::read_only(*editor,
                                           "Max Level",
                                           sc->max_level);
                    prop_editor::read_only(*editor,
                                           "Total Levels",
                                           sc->total_levels);
                    prop_editor::read_only(*editor,
                                           "Max Indexes",
                                           sc->max_indexes);
                    break;
                }
                case alloc_type_t::scratch: {
                    auto sc = &alloc->subclass.scratch;
                    prop_editor::read_only(*editor,
                                           "Ring Buffer Size",
                                           sc->size);
                    prop_editor::read_only(*editor,
                                           "Ring Begin",
                                           (u0*) sc->begin);
                    prop_editor::read_only(*editor,
                                           "Ring End",
                                           (u0*) sc->end);
                    prop_editor::read_only(*editor,
                                           "Next Pointer",
                                           (u0*) sc->alloc);
                    prop_editor::read_only(*editor,
                                           "Free Pointer",
                                           (u0*) sc->free);
                    break;
                }
                case alloc_type_t::dlmalloc: {
                    auto sc = &alloc->subclass.dl;
                    prop_editor::read_only(*editor,
                                           "Heap Size",
                                           sc->size);
                    prop_editor::read_only(*editor,
                                           "Space",
                                           (u0*) sc->heap);
                    prop_editor::read_only(*editor,
                                           "Base Pointer",
                                           (u0*) sc->base);
                    break;
                }
                default:
                    break;
            }
            prop_editor::end_nested(*editor);
        }

        if (prop_editor::read_only(*editor, "Name", memory::name(alloc))) {
            prop_editor::description(*editor,
                                     "The name assigned to this allocator."_ss);
        }

        if (prop_editor::read_only(*editor, "Total Allocated", alloc->total_allocated)) {
            prop_editor::description(*editor,
                                     "The total memory that has been allocated "
                                     "through this allocator.  In the case of proxy "
                                     "allocators, this value represents a pass-through "
                                     "from the backing allocator."_ss);
        }

        if (!alloc->backing)
            return;
        if (prop_editor::begin_nested(*editor, "##backing", "Backing")) {
            allocator_props(editor, alloc->backing);
            prop_editor::end_nested(*editor);
        }
    }

    static b8 type_editor_alloc_info(prop_editor_t* editor) {
        auto alloc_info = (alloc_info_t*) editor->selected;
        auto alloc = alloc_info->tracked;
        if (prop_editor::begin_category(*editor, "##alloc", "Allocator")) {
            allocator_props(editor, alloc);
            prop_editor::end_category(*editor);
        }
        return true;
    }

    b8 on_render(gfx::app_t& app) {
//        auto& io = ImGui::GetIO();

        auto vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar
                                 | ImGuiWindowFlags_NoDocking
                                 | ImGuiWindowFlags_NoTitleBar
                                 | ImGuiWindowFlags_NoCollapse
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoBringToFrontOnFocus
                                 | ImGuiWindowFlags_NoNavFocus
                                 | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("dockspace", nullptr, flags);
        app.dock_root = ImGui::GetID("dock_space");
        ImGui::DockSpace(app.dock_root,
                         ImVec2(0, 0),
                         ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::PopStyleVar(3);
        draw_menu_bar(app);
        ImGui::End();

        gfx::tool::alloc::draw(s_test_app.alloc_window);
        gfx::tool::errors::draw(s_test_app.errors_window);
        gfx::tool::strings::draw(s_test_app.strings_window);

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
            auto& editor = s_test_app.memory_editor;
            auto mem_size = mem ? size : 0;
            size_t base_display_addr = 0x0000;
            MemoryEditor::Sizes s;
            editor.CalcSizes(s, mem_size, base_display_addr);
            ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f),
                                                ImVec2(s.WindowWidth, FLT_MAX));

            editor.Open = true;
            if (gfx::begin_tool_window(*app.icons_atlas,
                                       ICONS_PROCESSOR,
                                       "Memory Editor",
                                       &editor.Open,
                                       ImGuiWindowFlags_NoScrollbar)) {
                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)
                &&  ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    ImGui::OpenPopup("context");
                }
                editor.DrawContents(mem, mem_size, base_display_addr);
                if (editor.ContentsWidthChanged) {
                    editor.CalcSizes(s, mem_size, base_display_addr);
                    ImGui::SetWindowSize(ImVec2(s.WindowWidth,
                                                ImGui::GetWindowSize().y));
                }
            }
            ImGui::End();
        }

        if (s_test_app.alloc_window.selected) {
            prop_editor::selected<alloc_info_t>(s_test_app.prop_editor,
                                                s_test_app.alloc_window.selected);
        }

        gfx::msg_stack::draw(s_test_app.msg_stack);
        prop_editor::draw(s_test_app.prop_editor);

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
        gfx::msg_stack::init(s_test_app.msg_stack, app.large_font, 0);
        alloc::init(s_test_app.alloc_window, &app, s_test_app.alloc);
        errors::init(s_test_app.errors_window, &app, s_test_app.alloc);
        strings::init(s_test_app.strings_window, &app, s_test_app.alloc);
        prop_editor::init(s_test_app.prop_editor, &app, s_test_app.alloc);
        prop_editor::register_type_editor<alloc_info_t>(s_test_app.prop_editor,
                                                        type_editor_alloc_info);
        auto status = gfx::app::run(app);
        s32 rc = !OK(status);

        prop_editor::free(s_test_app.prop_editor);
        strings::free(s_test_app.strings_window);
        errors::free(s_test_app.errors_window);
        alloc::free(s_test_app.alloc_window);
        gfx::msg_stack::free(s_test_app.msg_stack);
        gfx::app::free(app);

        return rc;
    }
}
