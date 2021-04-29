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

#include <basecode/gfx/tool/strings.h>
#include <basecode/gfx/fonts/IconsFontAwesome5.h>

namespace basecode::gfx::tool::strings {
    u0 free(strings_win_t& win) {
        UNUSED(win);
    }

    b8 draw(strings_win_t& win) {
        if (!win.visible)
            return false;
        ImGui::Begin(ICON_FA_DOLLAR_SIGN "  Strings", &win.visible);
        const auto region_size = ImGui::GetContentRegionAvail();
        if (region_size.x != win.size.x
        ||  region_size.y != win.size.y) {
            win.size = region_size;
        }
        ImGui::BeginChild("##toolbar", ImVec2(0, 50));
        ImGui::Button("Interned");
        ImGui::SameLine();
        ImGui::Button("Localized");
        ImGui::EndChild();
        ImGui::End();
        return true;
    }

    u0 init(strings_win_t& win, app_t* app, alloc_t* alloc) {
        win.app     = app;
        win.visible = true;
    }
}
