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

#include <basecode/gfx/gfx.h>

namespace basecode::gfx::tool::prop_editor {
    b8 begin_category(prop_editor_t& editor,
                      const s8* str_id,
                      const s8* fmt,
                      ...);

    template <typename T>
    b8 read_only(prop_editor_t& editor,
                 const s8* name,
                 T&& value) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        auto id = ImGui::GetID(name);
        b8 selected = editor.prop_id == id;
        ImGui::Selectable(name, &selected);
        if (selected) {
            str::reset(editor.help_heading);
            str::append(editor.help_heading, name);
            editor.prop_id = id;
        }
        ImGui::TableSetColumnIndex(1);
        auto& scratch = editor.app->scratch;
        str::reset(scratch); {
            str_buf_t buf(&scratch);
            format::format_to(buf, "{}", value);
        }
        ImGui::TextDisabled("%s", str::c_str(scratch));
        return selected;
    }

    u0 free(prop_editor_t& editor);

    b8 draw(prop_editor_t& editor);

    b8 begin_nested(prop_editor_t& editor,
                    const s8* str_id,
                    const s8* fmt, ...);

    u0 end_nested(prop_editor_t& editor);

    u0 end_category(prop_editor_t& editor);

    u0 init(prop_editor_t& editor,
            app_t* app,
            alloc_t* alloc = context::top()->alloc.main);

    template <typename T>
    u0 selected(prop_editor_t& editor, T* value) {
        editor.type_id  = family_t<>::template type<T>;
        editor.selected = (u0*) value;
    }

    template <String_Concept T>
    u0 description(prop_editor_t& editor, const T& text) {
        str::reset(editor.help_text);
        str::append(editor.help_text, text);
    }

    template <typename T>
    u0 register_type_editor(prop_editor_t& editor, type_editor_t type_editor) {
        const auto type_id  = family_t<>::template type<T>;
        hashtab::insert(editor.typetab, type_id, type_editor);
    }
}
