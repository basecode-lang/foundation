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

#include <basecode/core/memory/meta.h>
#include <basecode/gfx/alloc_window.h>
#include <basecode/core/memory/system/proxy.h>

namespace basecode::alloc_window {
    static alloc_info_t* s_selected{};

    static u0 draw_allocators(alloc_window_t& win,
                              const alloc_info_array_t& roots) {

        u32 row{};
        for (auto info : roots) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(info);
            auto node_open = ImGui::TreeNode("Alloc",
                                             "0x%016llX",
                                             (u64) info->tracked);
            ImGui::TableSetColumnIndex(1);
            if (IS_PROXY(info->tracked)) {
                str::reset(win.scratch);
                str::append(win.scratch,
                            memory::proxy::name(info->tracked));
                ImGui::Text("%s", str::c_str(win.scratch));
            } else {
                ImGui::Text("%s", "(none)");
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", memory::type_name(info->tracked->system->type));
            ImGui::TableSetColumnIndex(3);
            str::reset(win.scratch); {
                str_buf_t buf(&win.scratch);
                format::unitized_byte_size(buf,
                                           info->tracked->total_allocated);
            }
            ImGui::Text("%s", str::c_str(win.scratch));
            ++row;
            if (node_open) {
                s_selected = info;
                memory::meta::system::start_plot(info,
                                                 plot_mode_t::scrolled);
                if (info->children.size > 0)
                    draw_allocators(win, info->children);
                ImGui::TreePop();
            } else {
                memory::meta::system::stop_plot(info);
                if (s_selected == info)
                    s_selected = {};
            }
            ImGui::PopID();
        }
    }

    u0 free(alloc_window_t& win) {
        str::free(win.scratch);
        ImPlot::DestroyContext(win.ctx);
    }

    u0 draw(alloc_window_t& win) {
        ImGui::Begin("Allocators", &win.visible);
        if (ImGui::BeginTable("Allocators",
                              4,
                              ImGuiTableFlags_Borders
                              | ImGuiTableFlags_RowBg
                              | ImGuiTableFlags_Resizable
                              | ImGuiTableFlags_PreciseWidths
                              | ImGuiTableFlags_NoBordersInBody,
                              ImVec2(ImGui::GetContentRegionAvailWidth() * .5, -1))) {
            ImGui::TableSetupColumn("Allocator");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Total Allocated");
            ImGui::TableHeadersRow();
            draw_allocators(win, memory::meta::system::roots());
            ImGui::EndTable();
        }
        ImGui::SameLine();
        if (s_selected) {
            const ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
            auto plot = &s_selected->plot.scrolled;
            ImPlot::SetNextPlotLimitsX(plot->time - 10,
                                       plot->time,
                                       ImGuiCond_Always);
            ImPlot::SetNextPlotLimitsY(0,
                                       std::max<u32>(4096, plot->max_y * 2),
                                       ImGuiCond_Always);
            if (plot->values.size > 0
                &&  ImPlot::BeginPlot("##scrolled", {}, {}, ImVec2(-1, -1),
                                      rt_axis, rt_axis | ImPlotAxisFlags_LockMin)) {
                ImPlot::PlotShaded("memory",
                                   &plot->values[0].x,
                                   &plot->values[0].y,
                                   plot->values.size,
                                   0,
                                   plot->offset,
                                   sizeof(data_point_t));
                ImPlot::PlotLine("total_allocated",
                                 &plot->values[0].x,
                                 &plot->values[0].y,
                                 plot->values.size,
                                 plot->offset,
                                 sizeof(data_point_t));
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

    u0 init(alloc_window_t& win, alloc_t* alloc) {
        win.ctx        = ImPlot::CreateContext();
        win.alloc      = alloc;
        win.visible    = true;
        win.mem_editor = false;
        str::init(win.scratch, win.alloc);
        str::reserve(win.scratch, 64);
    }
}
