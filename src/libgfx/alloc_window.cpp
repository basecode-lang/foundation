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
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/core/memory/meta.h>
#include <basecode/gfx/alloc_window.h>
#include <basecode/gfx/implot/implot.h>
#include <basecode/gfx/fonts/IconsFontAwesome5.h>

namespace basecode::alloc_window {
    static alloc_info_t* selected{};

    static u0 draw_allocators(alloc_window_t& win,
                              const alloc_info_array_t& roots) {

        str_t scratch{};
        str::init(scratch, context::top()->alloc.scratch);
        str::reserve(scratch, 64);
        defer(str::free(scratch));

        const auto base_flags = ImGuiTreeNodeFlags_OpenOnArrow
                                | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
        u32 row{};
        for (auto info : roots) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(info);
            auto node_flags = base_flags;
            if (info->selected)
                node_flags |= ImGuiTreeNodeFlags_Selected;
            if (info->children.size == 0)
                node_flags |= ImGuiTreeNodeFlags_Leaf;
            auto node_open = ImGui::TreeNodeEx(info,
                                               node_flags,
                                               "0x%016llX",
                                               (u64) info->tracked);
            if (ImGui::IsItemClicked()) {
                if (!info->selected) {
                    selected = info;
                    info->selected = true;
                } else {
                    selected = {};
                    info->selected = false;
                }
            }
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(memory::name(info->tracked));
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(memory::type_name(info->tracked->system->type));
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::TableSetColumnIndex(3);
            str::reset(scratch); {
                str_buf_t buf(&scratch);
                format::unitized_byte_size(buf,
                                           info->tracked->total_allocated);
            }
            gfx::text_right_align(str::c_str(scratch),
                                  (const s8*) scratch.end());
            ++row;
            if (node_open) {
                if (info->children.size > 0)
                    draw_allocators(win, info->children);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }

    u0 free(alloc_window_t& win) {
        ImPlot::DestroyContext(win.ctx);
    }

    b8 draw(alloc_window_t& win) {
        if (!win.visible)
            return false;
        ImGui::Begin(ICON_FA_MEMORY "  Allocators", &win.visible);
        const auto region_size = ImGui::GetContentRegionAvail();
        win.height = region_size.y;
        if ((win.table_size == 0 && win.graph_size == 0)
        ||  (win.table_size + win.graph_size < region_size.x)
        ||  (win.table_size + win.graph_size > region_size.x)) {
            win.table_size = region_size.x * .5;
            win.graph_size = region_size.x * .5;
        }
        gfx::splitter(true,
                      8.0f,
                      &win.table_size,
                      &win.graph_size,
                      8,
                      8,
                      win.height);
        ImGui::BeginChild("Allocators",
                          ImVec2(win.table_size, win.height),
                          true);
        if (ImGui::BeginTable("Allocators",
                              4,
                              ImGuiTableFlags_Borders
                              | ImGuiTableFlags_RowBg
                              | ImGuiTableFlags_Resizable
                              | ImGuiTableFlags_PreciseWidths
                              | ImGuiTableFlags_NoBordersInBody,
                              ImVec2(ImGui::GetContentRegionAvailWidth(), -1))) {
            ImGui::TableSetupColumn("Allocator");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Total Allocated");
            ImGui::TableHeadersRow();
            draw_allocators(win, memory::meta::system::infos());
            ImGui::EndTable();
        }
        ImGui::EndChild();

        if (selected) {
            if (win.selected != selected) {
                win.selected = selected;
                memory::meta::system::start_plot(selected,
                                                 plot_mode_t::scrolled);
            }
        } else {
            if (win.selected) {
                memory::meta::system::stop_plot(win.selected);
                win.selected = {};
            }
        }

        ImGui::SameLine();
        ImGui::BeginChild("Graph",
                          ImVec2(win.graph_size, win.height),
                          true);
        if (win.selected) {
            const ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
            auto plot = &win.selected->plot.scrolled;
            ImPlot::SetNextPlotLimitsX(plot->time - 10,
                                       plot->time,
                                       ImGuiCond_Always);
            ImPlot::SetNextPlotLimitsY(0,
                                       std::max<u32>(plot->min_y, plot->max_y * 2),
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
        ImGui::EndChild();
        ImGui::End();
        return true;
    }

    u0 init(alloc_window_t& win, alloc_t* alloc) {
        win.ctx        = ImPlot::CreateContext();
        win.alloc      = alloc;
        win.height     = {};
        win.visible    = true;
        win.mem_editor = false;
        win.table_size = {};
        win.graph_size = {};
    }
}
