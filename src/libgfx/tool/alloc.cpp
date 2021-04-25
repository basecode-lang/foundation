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
#include <basecode/core/defer.h>
#include <basecode/core/format.h>
#include <basecode/gfx/tool/alloc.h>
#include <basecode/core/memory/meta.h>
#include <basecode/gfx/implot/implot.h>
#include <basecode/gfx/fonts/IconsFontAwesome5.h>

namespace basecode::gfx::tool::alloc {
    [[maybe_unused]]
    static u0 draw_graph(alloc_win_t& win) {
        if (!win.selected)
            return;

        auto plot = &win.selected->plot.scrolled;
        if (plot->values.size == 0)
            return;

        ImGui::BeginChild("Graph",
                          ImVec2(win.graph_size, win.height),
                          true);
        const ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
        ImPlot::SetNextPlotLimitsX(plot->time - 10,
                                   plot->time,
                                   ImGuiCond_Always);
        ImPlot::SetNextPlotLimitsY(0,
                                   std::max<u32>(plot->min_y, plot->max_y * 2),
                                   ImGuiCond_Always);
        if (ImPlot::BeginPlot("##scrolled", {}, {}, ImVec2(-1, -1),
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
        ImGui::EndChild();
    }

    static u0 draw_details(alloc_win_t& win) {
        if (!win.selected)
            return;
        ImGui::BeginChild("Details",
                          ImVec2(win.graph_size, win.height),
                          true);
        auto tracked = win.selected->tracked;
        ImGui::PushFont(win.app->large_font);
        ImGui::Text("Name: %s", memory::name(tracked));
        ImGui::Text("Type: %s", memory::type_name(tracked->system->type));
        ImGui::PopFont();
        switch (tracked->system->type) {
            case alloc_type_t::base:
                break;
            case alloc_type_t::bump:
                break;
            case alloc_type_t::page:
                break;
            case alloc_type_t::slab: {
                auto sc = &tracked->subclass.slab;
                if (ImGui::BeginTable("AttributeTable",
                                      2,
                                      ImGuiTableFlags_Borders
                                      | ImGuiTableFlags_RowBg
                                      | ImGuiTableFlags_Resizable
                                      | ImGuiTableFlags_PreciseWidths
                                      | ImGuiTableFlags_NoBordersInBody,
                                      ImVec2(ImGui::GetContentRegionAvailWidth(), -1))) {
                    ImGui::TableSetupColumn("Attribute");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Head");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->head);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Tail");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->tail);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Number of Pages");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->num_pages);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Page Size");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->page_size);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Buffer Size");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->buf_size);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Buffer Align");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->buf_align);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Buffer Count");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->count);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Maximum Buffer Count");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->buf_max_count);
                    ImGui::EndTable();
                }
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
                if (ImGui::BeginTable("AttributeTable",
                                      2,
                                      ImGuiTableFlags_Borders
                                      | ImGuiTableFlags_RowBg
                                      | ImGuiTableFlags_Resizable
                                      | ImGuiTableFlags_PreciseWidths
                                      | ImGuiTableFlags_NoBordersInBody,
                                      ImVec2(ImGui::GetContentRegionAvailWidth(), -1))) {
                    ImGui::TableSetupColumn("Attribute");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Ring Begin");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->begin);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Ring End");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->end);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Ring Size");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", sc->size);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Alloc Pointer");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->alloc);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Free Pointer");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("0x%p", sc->free);
                    ImGui::EndTable();
                }
                break;
            }
            case alloc_type_t::dlmalloc:
                break;
            default:
                break;
        }
        ImGui::EndChild();
    }

    static alloc_info_t* draw_table(alloc_win_t& win,
                                    const alloc_info_array_t& roots) {

        str_t scratch{};
        str::init(scratch, context::top()->alloc.scratch);
        str::reserve(scratch, 64);
        defer(str::free(scratch));

        const auto base_flags = ImGuiTreeNodeFlags_OpenOnArrow
                                | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
        alloc_info_t* node_clicked{};
        u32 row{};
        for (auto info : roots) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(info);
            auto node_flags = base_flags;
            if (info == win.selected)
                node_flags |= ImGuiTreeNodeFlags_Selected;
            if (info->children.size == 0)
                node_flags |= ImGuiTreeNodeFlags_Leaf;
            auto node_open = ImGui::TreeNodeEx(info,
                                               node_flags,
                                               "%s",
                                               memory::name(info->tracked));
            if (ImGui::IsItemClicked())
                node_clicked = info;
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(memory::type_name(info->tracked->system->type));
            ImGui::TableSetColumnIndex(2);
            str::reset(scratch); {
                str_buf_t buf(&scratch);
                format::unitized_byte_size(buf,
                                           info->tracked->total_allocated);
            }
            gfx::text_right_align(str::c_str(scratch),
                                  (const s8*) scratch.end());
            ++row;
            if (node_open) {
                if (info->children.size > 0) {
                    auto child_clicked = draw_table(win, info->children);
                    if (child_clicked)
                        node_clicked = child_clicked;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        return node_clicked;
    }

    u0 free(alloc_win_t& win) {
        ImPlot::DestroyContext(win.ctx);
    }

    b8 draw(alloc_win_t& win) {
        if (!win.visible)
            return false;
        ImGui::Begin(ICON_FA_MEMORY "  Allocators", &win.visible);
        alloc_info_t* node_clicked{};
//        const auto region_size = ImGui::GetContentRegionAvail();
//        win.height = region_size.y;
//        if ((win.table_size == 0 && win.graph_size == 0)
//        ||  (win.table_size + win.graph_size < region_size.x)
//        ||  (win.table_size + win.graph_size > region_size.x)) {
//            win.table_size = region_size.x * .5;
//            win.graph_size = region_size.x * .5;
//        }
//        gfx::splitter(true,
//                      8.0f,
//                      &win.table_size,
//                      &win.graph_size,
//                      8,
//                      8,
//                      win.height);
        ImGui::BeginChild("Allocators",
                          ImVec2(win.table_size, win.height),
                          true);
        if (ImGui::BeginTable("Allocators",
                              3,
                              ImGuiTableFlags_Borders
                              | ImGuiTableFlags_RowBg
                              | ImGuiTableFlags_Resizable
                              | ImGuiTableFlags_PreciseWidths
                              | ImGuiTableFlags_NoBordersInBody,
                              ImVec2(ImGui::GetContentRegionAvailWidth(), -1))) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Total Allocated");
            ImGui::TableHeadersRow();
            node_clicked = draw_table(win, memory::meta::system::infos());
            ImGui::EndTable();
        }
        ImGui::EndChild();

        if (node_clicked) {
            if (win.selected != node_clicked) {
                memory::meta::system::stop_plot(win.selected);
                win.selected = node_clicked;
                memory::meta::system::start_plot(win.selected,
                                                 plot_mode_t::scrolled);
            }
        }

//        ImGui::SameLine();
//        draw_details(win);

        ImGui::End();

        return true;
    }

    u0 init(alloc_win_t& win, app_t* app, alloc_t* alloc) {
        win.ctx        = ImPlot::CreateContext();
        win.app        = app;
        win.alloc      = alloc;
        win.height     = {};
        win.visible    = true;
        win.mem_editor = false;
        win.table_size = {};
        win.graph_size = {};
    }
}
