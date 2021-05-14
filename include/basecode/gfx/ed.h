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

namespace basecode::gfx::ed {
    namespace buf {
        u0 free(ed_buf_t& buf);

        u0 init(ed_buf_t& buf, str::slice_t name, ed_buf_type_t type);
    }

    namespace theme {
        u0 free(ed_theme_t& theme);

        u0 init(ed_theme_t& theme);
    }

    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    namespace region {
        u0 free(ed_region_t& region);

        u0 render(ed_region_t& region);

        u0 layout(ed_region_t& region);

        u0 init(ed_region_t& region,
                str::slice_t name,
                ed_region_flags_t flags,
                ed_region_layout_type_t type);

        u0 add_child(ed_region_t& region, ed_region_t* child);

        u0 remove_child(ed_region_t& region, ed_region_t* child);
    }

    namespace window {
        u0 free(ed_window_t& window);

        u0 render(ed_window_t& window);

        u0 init(ed_window_t& window,
                ed_buf_t* buf = {},
                ed_scroller_t* scroller = {},
                ed_tab_window_t* tab = {});

        u0 set_display_region(ed_window_t& window, const rect_t& rect);
    }

    namespace renderer {
        u0 free(ed_renderer_t& renderer);

        u0 init(ed_renderer_t& renderer);

        u0 draw_chars(ed_renderer_t& renderer,
                      const vec2_t& pos,
                      u32 color,
                      const u8* begin,
                      const u8* end);

        u0 draw_line(ed_renderer_t& renderer,
                     const vec2_t& start,
                     const vec2_t& end,
                     u32 color,
                     f32 width);

        u0 draw_filled_rect(ed_renderer_t& renderer,
                            const rect_t& rect,
                            u32 color);

        vec2_t get_text_size(ed_renderer_t& renderer,
                             const s8* begin,
                             const s8* end = nullptr);

        vec2_t get_char_size(ed_renderer_t& renderer, const u8* ch);
    }

    namespace tab_window {
        u0 free(ed_tab_window_t& tab);

        u0 init(ed_tab_window_t& tab);

        u0 display(ed_tab_window_t& tab);

        u0 set_display_region(ed_tab_window_t& tab,
                              const rect_t& region,
                              b8 force);

        u0 set_active_window(ed_tab_window_t& tab,
                             ed_window_t* window);

        u0 remove_window(ed_tab_window_t& tab, ed_window_t* window);

        ed_window_t* add_window(ed_tab_window_t& tab,
                                ed_buf_t* buf,
                                ed_window_t* parent,
                                ed_region_layout_type_t layout_type);
    }

    u0 free(ed_t& ed);

    u0 reset(ed_t& ed);

    u0 resize(ed_t& ed);

    u0 render(ed_t& ed);

    u0 next_tab(ed_t& ed);

    u0 prev_tab(ed_t& ed);

    ed_window_t* make_win(ed_t& ed);

    ed_tab_window_t* add_tab(ed_t& ed);

    u0 remove_tab(ed_t& ed, ed_tab_window_t* tab);

    status_t init(ed_t& ed, s64 ticks, ImFont* font);

    u0 set_display_region(ed_t& ed,
                          const vec2_t& top_left,
                          const vec2_t& bottom_right);

    u0 set_current_tab(ed_t& ed, ed_tab_window_t* tab);

    ed_region_t* make_region(ed_t& ed,
                             str::slice_t name = {},
                             ed_region_flags_t flags = {},
                             ed_region_layout_type_t type = {});

    ed_buf_t* make_buf(ed_t& ed, str::slice_t name, ed_buf_type_t type);
}
