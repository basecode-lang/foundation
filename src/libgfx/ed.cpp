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

#include <basecode/gfx/ed.h>
#include <basecode/core/utf.h>
#include <basecode/core/timer.h>
#include <basecode/core/string.h>
#include <basecode/core/obj_pool.h>

namespace basecode::gfx::ed {
    constexpr f32 left_border   = 3;
    constexpr f32 tab_spacing   = 1.0f;
    constexpr f32 text_border   = 2.0f;
    constexpr f32 bottom_border = 2.0f;

    struct system_t final {
        alloc_t*            alloc;
        obj_pool_t          storage;
        obj_type_t*         buf_type;
        obj_type_t*         theme_type;
        obj_type_t*         region_type;
        obj_type_t*         window_type;
        obj_type_t*         renderer_type;
        obj_type_t*         tab_window_type;
    };

    system_t                g_ed_sys{};

    namespace buf {
        u0 free(ed_buf_t& buf) {
            array::free(buf.line_endings);
            gap_buf::free(buf.gap_buf);
        }

        u0 init(ed_buf_t& buf, str::slice_t name, ed_buf_type_t type) {
            buf.type = type;
            buf.name = string::interned::fold(name);
            array::init(buf.line_endings, g_ed_sys.alloc);
            gap_buf::init(buf.gap_buf, 32, g_ed_sys.alloc);
        }
    }

    namespace theme {
        [[maybe_unused]]
        static u0 set_dark_theme(ed_theme_t* theme) {
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::text),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::text_dim),
                            IM_COL32(115, 115, 115, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::background),
                            IM_COL32(28, 28, 28, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::hidden_text),
                            IM_COL32(230, 26, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_border),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_inactive),
                            IM_COL32(102, 102, 102, 140));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_active),
                            IM_COL32(166, 166, 166, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number_background),
                            IM_COL32(33, 33, 33, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number),
                            IM_COL32(33, 255, 33, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number_active),
                            IM_COL32(33, 255, 33, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_normal),
                            IM_COL32(130, 140, 230, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_insert),
                            IM_COL32(255, 255, 255, 230));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_line_background),
                            IM_COL32(64, 64, 64, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::airline_background),
                            IM_COL32(51, 51, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::light),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::dark),
                            IM_COL32(0, 0, 0, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::visual_select_background),
                            IM_COL32(120, 77, 64, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::mode),
                            IM_COL32(51, 204, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::normal),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::parenthesis),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::comment),
                            IM_COL32(0, 255, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::keyword),
                            IM_COL32(26, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::identifier),
                            IM_COL32(255, 191, 128, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::number),
                            IM_COL32(255, 255, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::string),
                            IM_COL32(255, 128, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::whitespace),
                            IM_COL32(77, 77, 77, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::error),
                            IM_COL32(166, 51, 38, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::warning),
                            IM_COL32(38, 51, 166, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::info),
                            IM_COL32(38, 153, 38, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_border),
                            IM_COL32(128, 128, 128, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_active),
                            IM_COL32(166, 166, 166, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_inactive),
                            IM_COL32(5, 5, 5, 0));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_background),
                            IM_COL32(51, 51, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::flash_color),
                            IM_COL32(204, 102, 13, 255));
        }

        [[maybe_unused]]
        static u0 set_light_theme(ed_theme_t* theme) {
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::text),
                            IM_COL32(0, 0, 0, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::text_dim),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::background),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::hidden_text),
                            IM_COL32(230, 26, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_border),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_inactive),
                            IM_COL32(102, 102, 102, 140));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::tab_active),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number_background),
                            IM_COL32(250, 250, 250, 0));

            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number),
                            IM_COL32(33, 102, 33, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::line_number_active),
                            IM_COL32(130, 153, 33, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_normal),
                            IM_COL32(130, 140, 230, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_insert),
                            IM_COL32(255, 255, 255, 230));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::cursor_line_background),
                            IM_COL32(217, 217, 217, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::airline_background),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::light),
                            IM_COL32(255, 255, 255, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::dark),
                            IM_COL32(0, 0, 0, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::visual_select_background),
                            IM_COL32(125, 153, 115, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::mode),
                            IM_COL32(51, 204, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::normal),
                            IM_COL32(0, 0, 0, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::parenthesis),
                            IM_COL32(0, 0, 0, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::comment),
                            IM_COL32(26, 102, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::keyword),
                            IM_COL32(51, 51, 77, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::identifier),
                            IM_COL32(51, 51, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::number),
                            IM_COL32(26, 77, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::string),
                            IM_COL32(26, 26, 102, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::whitespace),
                            IM_COL32(51, 51, 51, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::error),
                            IM_COL32(227, 51, 38, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::warning),
                            IM_COL32(38, 51, 227, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::info),
                            IM_COL32(38, 217, 38, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_border),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_active),
                            IM_COL32(230, 26, 26, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_inactive),
                            IM_COL32(128, 128, 128, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::widget_background),
                            IM_COL32(140, 140, 140, 255));
            hashtab::insert(theme->colortab,
                            u32(ed_theme_color_t::flash_color),
                            IM_COL32(204, 204, 13, 255));
        }

        u0 free(ed_theme_t& theme) {
            hashtab::free(theme.colortab);
        }

        u0 init(ed_theme_t& theme) {
            hashtab::init(theme.colortab, g_ed_sys.alloc);
        }
    }

    namespace system {
        u0 fini() {
            obj_pool::free(g_ed_sys.storage);
        }

        status_t init(alloc_t* alloc) {
            g_ed_sys.alloc = alloc;
            obj_pool::init(g_ed_sys.storage, g_ed_sys.alloc);
            g_ed_sys.buf_type        = obj_pool::register_type<ed_buf_t>(g_ed_sys.storage);
            g_ed_sys.theme_type      = obj_pool::register_type<ed_theme_t>(g_ed_sys.storage);
            g_ed_sys.region_type     = obj_pool::register_type<ed_region_t>(g_ed_sys.storage);
            g_ed_sys.window_type     = obj_pool::register_type<ed_window_t>(g_ed_sys.storage);
            g_ed_sys.renderer_type   = obj_pool::register_type<ed_renderer_t>(g_ed_sys.storage);
            g_ed_sys.tab_window_type = obj_pool::register_type<ed_tab_window_t>(g_ed_sys.storage);
            return status_t::ok;
        }
    }

    namespace region {
        static f32 size_for_layout(ed_region_layout_type_t type,
                                   const vec2_t& v) {
            return type == ed_region_layout_type_t::hbox ? v.x : v.y;
        }

        u0 free(ed_region_t& region) {
            array::free(region.children);
        }

        u0 init(ed_region_t& region,
                str::slice_t name,
                ed_region_flags_t flags,
                ed_region_layout_type_t type) {
            region.type    = type;
            region.rect    = {};
            region.size    = {};
            region.name    = name;
            region.flags   = flags;
            region.margin  = {};
            region.parent  = {};
            region.padding = {};
            array::init(region.children);
        }

        u0 render(ed_region_t& region) {
        }

        u0 layout(ed_region_t& region) {
            f32 total_fixed_size = 0.0f;
            f32 expanders = 0.0f;
            f32 padding = 0.0f;
            for (auto child : region.children) {
                if ((child->flags & region_flags::fixed) == region_flags::fixed) {
                    total_fixed_size += size_for_layout(region.type,
                                                        child->size);
                } else {
                    expanders += 1.0f;
                }
                padding += child->padding.x + child->padding.y;
            }

            rect_t curr_rect = region.rect;
            curr_rect.adjust(region.margin.x,
                             region.margin.y,
                             -region.margin.z,
                             -region.margin.w);
            auto remaining = (region.type == ed_region_layout_type_t::hbox ?
                              curr_rect.width() : curr_rect.height())
                             - (total_fixed_size + padding);
            auto per_expanding = remaining / expanders;
            per_expanding = std::max(0.0f, per_expanding);
            for (auto child : region.children) {
                child->rect = curr_rect;
                switch (region.type) {
                    case ed_region_layout_type_t::none:
                        break;
                    case ed_region_layout_type_t::vbox: {
                        child->rect.tl.x += child->padding.x;
                        if ((child->flags & region_flags::fixed) == region_flags::fixed) {
                            child->rect.br.y = child->rect.top()
                                               + size_for_layout(region.type, child->size);
                            curr_rect.tl.y = child->rect.bottom()
                                             + child->padding.y;
                        } else {
                            child->rect.br.y = child->rect.top() + per_expanding;
                            curr_rect.tl.y = child->rect.bottom() + child->padding.y;
                        }
                        break;
                    }
                    case ed_region_layout_type_t::hbox: {
                        child->rect.tl.x += child->padding.x;
                        if ((child->flags & region_flags::fixed) == region_flags::fixed) {
                            child->rect.br.x = child->rect.left()
                                               + size_for_layout(region.type, child->size);
                            curr_rect.tl.x = child->rect.right() + child->padding.y;
                        } else {
                            child->rect.br.x = child->rect.left() + per_expanding;
                            curr_rect.tl.x = child->rect.right() + child->padding.y;
                        }
                        break;
                    }
                }

                if (!(child->flags & region_flags::expand)
                &&  child->size.x == 0.0f
                &&  child->size.y == 0.0f) {
                    auto curr_size = child->rect.size();
                    child->rect.set_size(clamp(child->size,
                                               vec2_t(0.0f, 0.0f),
                                               curr_size));
                    curr_rect.tl = region.type == ed_region_layout_type_t::hbox ?
                                   child->rect.top_right() : child->rect.bottom_left();
                    if ((child->flags & region_flags::align) == region_flags::align) {
                        auto diff = curr_size - child->rect.size();
                        diff.x = std::max(0.0f, diff.x);
                        diff.y = std::max(0.0f, diff.y);
                        child->rect.adjust(diff.x / 2.0f, diff.y / 2.0f);
                    }
                }
            }

            for (auto child : region.children)
                region::layout(*child);
        }

        u0 add_child(ed_region_t& region, ed_region_t* child) {
            child->parent = &region;
            array::append(region.children, child);
        }

        u0 remove_child(ed_region_t& region, ed_region_t* child) {
            child->parent = {};
            array::erase(region.children, child);
        }
    }

    namespace window {
        u0 free(ed_window_t& window) {
            array::free(window.spans);
        }

        u0 init(ed_window_t& window,
                ed_buf_t* buf,
                ed_scroller_t* scroller,
                ed_tab_window_t* tab) {
            window.buf        = buf;
            window.scroller   = scroller;
            window.tab_window = tab;
            array::init(window.spans, g_ed_sys.alloc);
        }

        u0 display(ed_window_t& window) {
        }

        u0 set_display_region(ed_window_t& window, const rect_t& rect) {
            window.disp_rect = rect;
        }
    }

    namespace renderer {
        u0 draw_chars(ed_renderer_t& renderer,
                      const vec2_t& pos,
                      u32 color,
                      const u8* begin,
                      const u8* end) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const auto& clip_rect = renderer.clip_rect;
            if (!end)
                end = begin + strlen((const s8*) begin);
            if (clip_rect.width() == 0) {
                draw_list->AddText(renderer.font,
                                   renderer.glyph.height,
                                   ImVec2(pos.x, pos.y),
                                   color,
                                   (const s8*) begin,
                                   (const s8*) end);
            } else {
                draw_list->PushClipRect(ImVec2(clip_rect.tl.x, clip_rect.tl.y),
                                        ImVec2(clip_rect.br.x, clip_rect.br.y));
                draw_list->AddText(renderer.font,
                                   renderer.glyph.height,
                                   ImVec2(pos.x, pos.y),
                                   color,
                                   (const s8*) begin,
                                   (const s8*) end);
                draw_list->PopClipRect();
            }
        }

        u0 free(ed_renderer_t& renderer) {
            hashtab::free(renderer.glyph.cache);
        }

        vec2_t get_text_size(ed_renderer_t& renderer,
                             const s8* begin,
                             const s8* end) {
            auto font = renderer.font;
            ImVec2 text_size = font->CalcTextSizeA(renderer.glyph.height,
                                                   FLT_MAX,
                                                   FLT_MAX,
                                                   begin,
                                                   end,
                                                   nullptr);
            if (text_size.x == 0.0f)
                return renderer.glyph.default_char;
            return vec2_t(text_size.x, text_size.y);
        }

        u0 init(ed_renderer_t& renderer, ImFont* font) {
            renderer.font         = font;
            renderer.glyph.height = font->FontSize;
            hashtab::init(renderer.glyph.cache, g_ed_sys.alloc);

            const s8 a = 'A';
            renderer.glyph.default_char = get_text_size(renderer, &a, &a + 1);
            for (u32 i = 0; i < 256; ++i) {
                const s8 ch = (s8) i;
                renderer.glyph.ascii_cache[i] = get_text_size(renderer,
                                                              &ch,
                                                              &ch + 1);
            }

            auto& dot = renderer.glyph.dot;
            dot = renderer.glyph.default_char / 8.0f;
            dot.x = std::min(dot.x, dot.y);
            dot.y = std::min(dot.x, dot.y);
            dot.x = std::max(1.0f, dot.x);
            dot.y = std::max(1.0f, dot.y);
        }

        vec2_t get_char_size(ed_renderer_t& renderer, const u8* ch) {
            utf32_codepoint_t cp{};
            auto len = utf8proc_iterate(ch, -1, (utf8proc_int32_t* )&cp);
            if (len == 1)
                return renderer.glyph.ascii_cache[*ch];

            {
                auto size = hashtab::find(renderer.glyph.cache, cp);
                if (size)
                    return *size;
            }

            auto size = get_text_size(renderer,
                                      (const s8*) ch,
                                      (const s8*) ch + len);
            hashtab::insert(renderer.glyph.cache, cp, size);
            return size;
        }

        u0 draw_line(ed_renderer_t& renderer,
                     const vec2_t& start,
                     const vec2_t& end,
                     u32 color,
                     f32 width) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const auto& clip_rect = renderer.clip_rect;
            if (clip_rect.width() == 0) {
                draw_list->AddLine(ImVec2(start.x, start.y),
                                   ImVec2(end.x, end.y),
                                   color,
                                   width);
            } else {
                draw_list->PushClipRect(ImVec2(clip_rect.tl.x, clip_rect.tl.y),
                                        ImVec2(clip_rect.br.x, clip_rect.br.y));
                draw_list->AddLine(ImVec2(start.x, start.y),
                                   ImVec2(end.x, end.y),
                                   color,
                                   width);
                draw_list->PopClipRect();
            }
        }

        u0 draw_filled_rect(ed_renderer_t& renderer,
                            const rect_t& rect,
                            u32 color) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const auto& clip_rect = renderer.clip_rect;
            if (clip_rect.width() == 0) {
                draw_list->AddRectFilled(ImVec2(rect.tl.x, rect.tl.y),
                                         ImVec2(rect.br.x, rect.br.y),
                                         color);
            } else {
                draw_list->PushClipRect(ImVec2(clip_rect.tl.x, clip_rect.tl.y),
                                        ImVec2(clip_rect.br.x, clip_rect.br.y));
                draw_list->AddRectFilled(ImVec2(rect.tl.x, rect.tl.y),
                                         ImVec2(rect.br.x, rect.br.y),
                                         color);
                draw_list->PopClipRect();
            }
        }
    }

    namespace tab_window {
        static u0 remove_empty_parent(ed_tab_window_t& tab,
                                      ed_region_t* parent) {
            if (!array::empty(parent->children))
                return;

            for (const auto& pair : tab.regiontab) {
                if (pair.value == parent)
                    return;
            }

            auto gp_region = parent->parent;
            if (gp_region) {
                region::remove_child(*gp_region, parent);
                remove_empty_parent(tab, gp_region);
            }
        }

        u0 free(ed_tab_window_t& tab) {
            array::free(tab.windows);
            hashtab::free(tab.regiontab);
        }

        u0 render(ed_tab_window_t& tab) {
            for (auto win : tab.windows)
                window::display(*win);
        }

        u0 set_display_region(ed_tab_window_t& tab,
                              const rect_t& region,
                              b8 force) {
            if (tab.last_region == region || !force)
                return;

            tab.last_region = region;
            if (tab.root) {
                tab.root->rect = region;
                region::layout(*tab.root);
            }

            for (auto win : tab.windows) {
                auto tab_region = hashtab::find(tab.regiontab, (u0*) win);
                if (!tab_region)
                    continue;
                window::set_display_region(*win, tab_region->rect);
            }
        }

        u0 init(ed_tab_window_t& tab, ed_t* editor) {
            tab.root        = {};
            tab.active      = {};
            tab.editor      = editor;
            tab.last_region = {};
            array::init(tab.windows, g_ed_sys.alloc);
            hashtab::init(tab.regiontab, g_ed_sys.alloc);
        }

        ed_window_t* add_window(ed_tab_window_t& tab,
                                ed_buf_t* buf,
                                ed_window_t* parent,
                                ed_region_layout_type_t layout_type) {
            if (tab.windows.size == 1 && !parent) {
                if ((buf->file_flags & file_flags::default_buffer) == file_flags::default_buffer
                &&  !(buf->file_flags & file_flags::dirty)) {
                    tab.windows[0]->buf = buf;
                    return tab.windows[0];
                }
            }

            auto win = make_win(*tab.editor);
            array::append(tab.windows, win);

            auto r = make_region(*tab.editor,
                                 buf->name,
                                 region_flags::expand,
                                 ed_region_layout_type_t::none);
//            set_active_window(tab, win);

            if (!parent) {
                if (tab.root->children.size > 1) {
                    auto r1 = make_region(*tab.editor,
                                          string::interned::fold("parented root"_ss),
                                          region_flags::expand,
                                          layout_type);
                    region::add_child(*r1, tab.root);
                    region::add_child(*r1, r);
                    tab.root = r1;
                } else {
                    region::add_child(*tab.root, r);
                    tab.root->type = layout_type;
                }

                r->parent = tab.root;
                hashtab::insert(tab.regiontab, (u0*) win, r);
            } else {
                ed_region_t* gp_region  {};

                auto parent_region = hashtab::find(tab.regiontab, (u0*) parent);
                if (parent_region)
                    gp_region = parent_region->parent;

                if (gp_region->children.size == 1) {
                    gp_region->type = layout_type;
                }

                if (gp_region->type == layout_type) {
                    s32 idx = array::contains(gp_region->children, parent_region);
                    if (idx != -1
                    &&  idx + 1 < gp_region->children.size) {
                        array::insert(gp_region->children, idx + 1, r);
                    } else {
                        array::append(gp_region->children, r);
                    }
                    r->parent = gp_region;
                    hashtab::set(tab.regiontab, (u0*) parent, r);
                } else {
                    auto split_region = hashtab::find(tab.regiontab, (u0*) parent);
                    BC_ASSERT_MSG(split_region->children.size == 0,
                                  "unexpected child region with existing splits.");
                    split_region->type = layout_type;
                    auto r1 = make_region(*tab.editor,
                                          "new sub region"_ss,
                                          region_flags::expand);
                    region::add_child(*split_region, r1);
                    region::add_child(*split_region, r);
                    hashtab::set(tab.regiontab, (u0*) parent, r1);
                    hashtab::set(tab.regiontab, (u0*) win, r);
                }
            }

            set_active_window(tab, win);
            set_display_region(tab, tab.last_region, true);

            return win;
        }

        u0 remove_window(ed_tab_window_t& tab, ed_window_t* window) {
            s32 idx = array::contains(tab.windows, window);
            if (idx == -1)
                return;

            auto region = hashtab::find(tab.regiontab, (u0*) window);
            BC_ASSERT_NOT_NULL(region);

            auto parent_region = region->parent;
            region::remove_child(*parent_region, region);
            remove_empty_parent(tab, parent_region);

            array::erase(tab.windows, window);
            hashtab::remove(tab.regiontab, (u0*) window);

            if (array::empty(tab.windows)) {
                set_active_window(tab, nullptr);
                tab.root = {};
            } else {
                if (tab.active == window)
                    set_active_window(tab, tab.windows[tab.windows.size - 1]);
                set_display_region(tab, tab.last_region, true);
            }
        }

        u0 set_active_window(ed_tab_window_t& tab, ed_window_t* window) {
            tab.active = window;
        }
    }

    static u0 free_complex_types(ed_t& ed) {
        for (auto buf : g_ed_sys.buf_type->objects)
            buf::free(*((ed_buf_t*) buf));
        for (auto theme : g_ed_sys.theme_type->objects)
            theme::free(*((ed_theme_t*) theme));
        for (auto window : g_ed_sys.window_type->objects)
            window::free(*((ed_window_t*) window));
        for (auto region : g_ed_sys.region_type->objects)
            region::free(*((ed_region_t*) region));
        for (auto renderer : g_ed_sys.renderer_type->objects)
            renderer::free(*((ed_renderer_t*) renderer));
        for (auto tab_window : g_ed_sys.tab_window_type->objects)
            tab_window::free(*((ed_tab_window_t*) tab_window));
    }

    u0 free(ed_t& ed) {
        timer::stop(ed.timer.cursor);
        timer::stop(ed.timer.last_edit);
        free_complex_types(ed);
        array::free(ed.tabs);
        queue::free(ed.buffers);
        hashtab::free(ed.regtab);
        array::free(ed.tab_windows);
        hashtab::free(ed.impl.buffer);
        hashtab::free(ed.impl.global);
    }

    u0 reset(ed_t& ed) {
        free_complex_types(ed);
        array::reset(ed.tabs);
        queue::reset(ed.buffers);
        hashtab::reset(ed.regtab);
        array::reset(ed.tab_windows);
        hashtab::reset(ed.impl.buffer);
        hashtab::reset(ed.impl.global);
    }

    u0 render(ed_t& ed) {
    }

    u0 resize(ed_t& ed) {
    }

    u0 next_tab(ed_t& ed) {
    }

    u0 prev_tab(ed_t& ed) {
    }

    ed_window_t* make_win(ed_t& ed) {
        auto win = obj_pool::make<ed_window_t>(g_ed_sys.storage);
        window::init(*win);
        return win;
    }

    ed_tab_window_t* add_tab(ed_t& ed) {
        auto tab_window = obj_pool::make<ed_tab_window_t>(g_ed_sys.storage);
        tab_window->editor = &ed;
        array::append(ed.tab_windows, tab_window);
        ed.active_tab = tab_window;
        auto empty_buf = make_buf(ed, "[No Name]"_ss, ed_buf_type_t::normal);
        tab_window::add_window(*tab_window,
                               empty_buf,
                               nullptr,
                               ed_region_layout_type_t::hbox);
        return tab_window;
    }

    u0 remove_tab(ed_t& ed, ed_tab_window_t* tab) {
    }

    status_t init(ed_t& ed, s64 ticks, ImFont* font) {
        ed.pad                = {};
        ed.blink              = {};
        ed.theme              = obj_pool::make<ed_theme_t>(g_ed_sys.storage);
        ed.changed            = {};
        ed.renderer           = obj_pool::make<ed_renderer_t>(g_ed_sys.storage);
        ed.timer.cursor       = timer::start(ticks,
                                             MS_TO_NS(250),
                                             [](auto timer, auto user) -> b8 {
                                                auto ped = (ed_t*) user;
                                                ped->blink = !ped->blink;
                                                return true;
                                             },
                                             &ed);
        ed.timer.last_edit    = timer::start(ticks, MS_TO_NS(500), nullptr);
        ed.region.tab         = make_region(ed);
        ed.region.cmd         = make_region(ed);
        ed.region.editor      = make_region(ed);
        ed.region.tab_content = make_region(ed);

        renderer::init(*ed.renderer, font);
        theme::init(*ed.theme);

        array::init(ed.tabs, g_ed_sys.alloc);
        queue::init(ed.buffers, g_ed_sys.alloc);
        hashtab::init(ed.regtab, g_ed_sys.alloc);
        array::init(ed.tab_windows, g_ed_sys.alloc);
        hashtab::init(ed.impl.buffer, g_ed_sys.alloc);
        hashtab::init(ed.impl.global, g_ed_sys.alloc);

        ed.region.tab->margin = {0, text_border, 0, text_border};
        array::append(ed.region.editor->children, ed.region.tab);
        array::append(ed.region.editor->children, ed.region.cmd);
        array::append(ed.region.editor->children, ed.region.tab_content);

        return status_t::ok;
    }

    u0 set_display_region(ed_t& ed,
                          const vec2_t& top_left,
                          const vec2_t& bottom_right) {
        ed.region.editor->rect.tl = top_left;
        ed.region.editor->rect.br = bottom_right;
        ed::resize(ed);
    }

    u0 set_current_tab(ed_t& ed, ed_tab_window_t* tab) {
    }

    ed_region_t* make_region(ed_t& ed,
                             str::slice_t name,
                             ed_region_flags_t flags,
                             ed_region_layout_type_t type) {
        auto r = obj_pool::make<ed_region_t>(g_ed_sys.storage);
        region::init(*r, name, flags, type);
        return r;
    }

    ed_buf_t* make_buf(ed_t& ed, str::slice_t name, ed_buf_type_t type) {
        auto buf = obj_pool::make<ed_buf_t>(g_ed_sys.storage);
        buf::init(*buf, name, type);
        queue::push_front(ed.buffers, buf);
        return buf;
    }
}
