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
#include <basecode/gfx/tool/prop_editor.h>
#include <basecode/gfx/imgui/imgui_internal.h>

namespace basecode::gfx::tool::prop_editor {
    u0 free(prop_editor_t& editor) {
        str::free(editor.help_text);
        str::free(editor.help_heading);
        hashtab::free(editor.typetab);
    }

    b8 draw(prop_editor_t& editor) {
        if (!editor.visible)
            return false;
        editor.item_id = 0;
        str::reset(editor.help_text);
        gfx::begin_tool_window(*editor.app->icons_atlas,
                               ICONS_PROPERTY,
                               "Properties",
                               &editor.visible);
        const auto region_size = ImGui::GetContentRegionAvail();
        if (region_size.x != editor.size.x
        ||  region_size.y != editor.size.y) {
            editor.size = region_size;
            editor.grid_height = editor.size.y * .75;
            editor.cmd_height  = editor.size.y - editor.grid_height;
        }
        ImGui::PushStyleColor(ImGuiCol_Separator,
                              ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
        gfx::splitter(false,
                      8.0f,
                      &editor.grid_height,
                      &editor.cmd_height,
                      8,
                      8);
        ImGui::PopStyleColor();
        ImGui::BeginChild("Grid", ImVec2(-1, editor.grid_height), true);
        if (editor.selected) {
            auto type_editor = hashtab::find(editor.typetab, editor.type_id);
            if (type_editor)
                type_editor(&editor);
        }
        ImGui::EndChild();
        ImGui::BeginChild("Command", ImVec2(-1, editor.cmd_height), true);
        if (!str::empty(editor.help_heading)) {
            ImGui::PushFont(editor.app->bold_font);
            ImGui::TextUnformatted(str::c_str(editor.help_heading));
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(0, 5));
        }
        if (!str::empty(editor.help_text))
            ImGui::TextWrapped("%s", str::c_str(editor.help_text));
        ImGui::EndChild();
        ImGui::End();
        return true;
    }

    b8 begin_nested(prop_editor_t& editor,
                    const s8* str_id,
                    const s8* fmt, ...) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        va_list args;
        va_start(args, fmt);
        ImGui::PushID(++editor.item_id);
        const auto base_flags = ImGuiTreeNodeFlags_OpenOnArrow
                                | ImGuiTreeNodeFlags_FramePadding
                                | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
        auto node_flags = base_flags;
        ImGui::PushFont(editor.app->bold_font);
        b8 is_open = ImGui::TreeNodeExV(str_id, node_flags, fmt, args);
        ImGui::PopFont();
        if (!is_open)
            ImGui::PopID();
        va_end(args);
        return is_open;
    }

    u0 end_nested(prop_editor_t& editor) {
        ImGui::TreePop();
        ImGui::PopID();
    }

    u0 end_category(prop_editor_t& editor) {
        ImGui::EndTable();
        ImGui::TreePop();
        ImGui::PopID();
    }

    b8 begin_category(prop_editor_t& editor,
                      const s8* str_id,
                      const s8* fmt,
                      ...) {
        va_list args;
        va_start(args, fmt);
        ImGui::PushID(++editor.item_id);
        const auto base_flags = ImGuiTreeNodeFlags_OpenOnArrow
                                | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
        auto node_flags = base_flags;
        ImGui::PushFont(editor.app->bold_font);
        b8 is_open = ImGui::TreeNodeExV(str_id, node_flags, fmt, args);
        ImGui::PopFont();
        va_end(args);
        if (is_open) {
            ImGui::BeginTable("##props",
                              2,
                              ImGuiTableFlags_Borders
                              | ImGuiTableFlags_Resizable,
                              ImVec2(-1, 100));
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Value");
        } else {
            ImGui::PopID();
        }
        return is_open;
    }

    u0 init(prop_editor_t& editor, app_t* app, alloc_t* alloc) {
        editor.app        = app;
        editor.item_id    = {};
        editor.visible    = true;
        editor.selected   = {};
        str::init(editor.help_heading, alloc);
        str::reserve(editor.help_heading, 256);
        str::init(editor.help_text, alloc);
        str::reserve(editor.help_text, 1024);
        hashtab::init(editor.typetab, alloc);
    }
}
