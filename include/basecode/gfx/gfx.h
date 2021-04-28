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

#include <basecode/core/path.h>
#include <basecode/core/array.h>
#include <basecode/core/stack.h>
#include <basecode/core/queue.h>
#include <basecode/core/gap_buf.h>
#include <basecode/core/hashtab.h>
#include <basecode/gfx/imgui/imgui.h>

#define PT_TO_PX(pt)            ((pt) * 1.3281472327365263157894736842105)

struct ImFont;
struct GLFWwindow;
struct ImPlotContext;

namespace basecode::gfx {
    struct app_t;

    // ------------------------------------------------------------------------
    //
    //  vectors
    //
    // ------------------------------------------------------------------------

    struct vec2_t final {
        f32                     x;
        f32                     y;

        vec2_t() : x(), y() {
        }

        vec2_t(f32 x, f32 y) : x(x), y(y) {
        }

        explicit vec2_t(f32 v) : x(v), y(v) {
        }

        inline b8 operator<(const vec2_t& rhs) const {
            if (x != rhs.x)
                return x < rhs.x;
            return y < rhs.y;
        }

        inline b8 operator==(const vec2_t& rhs) const {
            return x == rhs.x && y == rhs.y;
        }
    };

    inline vec2_t clamp(const vec2_t& val,
                        const vec2_t& min,
                        const vec2_t& max) {
        return vec2_t(std::min(max.x, std::max(min.x, val.x)),
                      std::min(max.y, std::max(min.y, val.y)));
    }

    inline vec2_t operator*(const vec2_t& lhs, f32 rhs) {
        return vec2_t(lhs.x * rhs, lhs.y * rhs);
    }

    inline vec2_t operator/(const vec2_t& lhs, f32 rhs) {
        return vec2_t(lhs.x / rhs, lhs.y / rhs);
    }

    inline vec2_t operator+(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    inline vec2_t operator-(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    inline vec2_t operator/(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x / rhs.x, lhs.y / rhs.y);
    }

    inline vec2_t operator*(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x * rhs.x, lhs.y * rhs.y);
    }

    struct vec3_t final {
        f32                     x;
        f32                     y;
        f32                     z;
    };

    struct vec4_t final {
        f32                     x;
        f32                     y;
        f32                     z;
        f32                     w;
    };

    // ------------------------------------------------------------------------
    //
    //  rect
    //
    // ------------------------------------------------------------------------

    struct rect_t final {
        vec2_t                  tl;
        vec2_t                  br;

        inline f32 top() const {
            return top_right().y;
        }

        inline b8 empty() const {
            return (height() == 0.0f || width() == 0.0f);
        }

        inline f32 left() const {
            return tl.x;
        }

        inline f32 width() const {
            return br.x - tl.x;
        }

        inline f32 right() const {
            return top_right().x;
        }

        inline f32 height() const {
            return br.y - tl.y;
        }

        inline f32 bottom() const {
            return br.y;
        }

        inline vec2_t size() const {
            return br - tl;
        }

        inline vec2_t center() const {
            return (br + tl) * .5f;
        }

        inline u0 adjust(f32 x, f32 y) {
            tl.x += x;
            tl.y += y;
            br.x += x;
            br.y += y;
        }

        u0 set_size(const vec2_t& size) {
            br = tl + size;
        }

        inline vec2_t top_right() const {
            return vec2_t(br.x, tl.y);
        }

        inline vec2_t bottom_left() const {
            return vec2_t(tl.x, br.y);
        }

        inline b8 contains(const vec2_t& pt) const {
            return tl.x <= pt.x
                   && tl.y <= pt.y
                   && br.x > pt.x
                   && br.y > pt.y;
        }

        inline u0 adjust(f32 x, f32 y, f32 z, f32 w) {
            tl.x += x;
            tl.y += y;
            br.x += z;
            br.y += w;
        }

        inline b8 operator==(const rect_t& rhs) const {
            return tl == rhs.tl && br == rhs.br;
        }
    };

    // ------------------------------------------------------------------------
    //
    //  color
    //
    // ------------------------------------------------------------------------

    struct color_t final {
        u8                      r;
        u8                      g;
        u8                      b;
        u8                      a;

        explicit operator u32() const   { return IM_COL32(r, g, b, a); }
    };

    // ------------------------------------------------------------------------
    //
    //  window
    //
    // ------------------------------------------------------------------------

    struct window_t final {
        GLFWwindow*             backing     {};
        s32                     x           = -1;
        s32                     y           = -1;
        s32                     width       = 1280;
        s32                     height      = 1024;
        s32                     iconified   = 0;
        s32                     maximized   = 0;
        s32                     min_width   = 800;
        s32                     min_height  = 600;
        b8                      focused     {};
    };

    // ----------------------------------------------------------------
    //
    // app
    //
    // ----------------------------------------------------------------

    using render_callback_t = std::function<b8 (app_t&)>;

    struct app_t final {
        alloc_t*                alloc;
        ImFont*                 bold_font;
        ImFont*                 large_font;
        render_callback_t       on_render;
        str::slice_t            short_name;
        str::slice_t            title;
        str_t                   scratch;
        window_t                window;
        vec4_t                  bg_color;
        f64                     ticks;
        s32                     dock_root;
    };

    // ------------------------------------------------------------------------
    //
    // message stack
    //
    // ------------------------------------------------------------------------
    constexpr u32 msg_stack_depth = 32;

    struct msg_t final {
        u8*                     data;
        u32                     length;
        u32                     color;
        u32                     alpha;
    };

    struct msg_stack_t final {
        timer_t*                timer;
        ImFont*                 font;
        msg_t                   data[msg_stack_depth];
        u32                     sp;
    };

    // ------------------------------------------------------------------------
    //
    // ed
    //
    // ------------------------------------------------------------------------

    struct ed_t;
    struct ed_cmd_t;
    struct ed_buf_t;
    struct ed_window_t;
    struct ed_airbox_t;
    struct ed_region_t;
    struct ed_register_t;
    struct ed_tree_node_t;
    struct ed_mode_impl_t;
    struct ed_span_info_t;
    struct ed_region_tab_t;
    struct ed_tab_window_t;
    struct ed_line_char_info_t;

    enum class ed_theme_color_t : u8;

    using ed_cmd_flags_t        = u32;
    using ed_cmd_stack_t        = stack_t<ed_cmd_t*>;
    using ed_lci_array_t        = array_t<ed_line_char_info_t>;
    using ed_buf_deque_t        = queue_t<ed_buf_t*>;
    using ed_mode_flags_t       = u8;
    using ed_file_flags_t       = u16;
    using ed_mode_table_t       = hashtab_t<u32, ed_mode_impl_t*>;
    using ed_color_table_t      = hashtab_t<u32, u32>;
    using ed_glyph_table_t      = hashtab_t<u32, vec2_t>;
    using ed_airbox_array_t     = array_t<ed_airbox_t>;
    using ed_region_flags_t     = u8;
    using ed_window_flags_t     = u32;
    using ed_key_notifier_t     = b8 (*)(u32 key, u32 mod);
    using ed_region_array_t     = array_t<ed_region_t*>;
    using ed_window_array_t     = array_t<ed_window_t*>;
    using ed_region_table_t     = hashtab_t<u0*, ed_region_t*>;
    using ed_line_end_array_t   = array_t<u32>;
    using ed_register_table_t   = hashtab_t<str::slice_t, ed_register_t*>;
    using ed_tree_node_flags_t  = u8;
    using ed_span_info_array_t  = array_t<ed_span_info_t*>;
    using ed_tree_node_array_t  = array_t<ed_tree_node_t*>;
    using ed_region_tab_array_t = array_t<ed_region_tab_t*>;
    using ed_tab_window_array_t = array_t<ed_tab_window_t*>;

    enum class ed_dir_t : u8 {
        forward,
        backward,
    };

    enum class ed_mode_t : u8 {
        none,
        normal,
        insert,
        visual,
        ex
    };

    enum class ed_cmd_op_t : u8 {
        none,
        delete_,
        delete_lines,
        insert,
        copy,
        copy_lines,
        replace,
        paste
    };

    enum class ed_buf_type_t : u8 {
        normal,
        search,
        repl,
        data_grid,
        tree
    };

    enum class ed_line_loc_t : u8 {
        none,
        line_first,
        line_last,
        line_last_non_cr,
        line_begin,
        beyond_line_end,
        line_cr_begin,
    };

    enum class ed_disp_mode_t : u8 {
        normal,
        vim,
    };

    enum class ed_expr_type_t : u8 {
        inner,
        outer,
    };

    enum class ed_open_type_t : u8 {
        replace,
        vsplit,
        hsplit,
        tab,
    };

    enum class ed_text_type_t : u8 {
        ui,
        text,
        heading1,
        heading2,
        heading3,
    };

    enum class ed_vim_motion_t : u8 {
        line_begin,
        line_end,
        non_white_space_begin,
        non_white_space_end,
    };

    enum class ed_flash_type_t : u8 {
        flash,
    };

    enum class ed_cursor_type_t : u8 {
        none,
        normal,
        insert,
        visual,
        line_marker,
    };

    enum class ed_tooltip_pos_t : u8 {
        above,
        below,
        right,
    };

    enum class ed_theme_type_t : u8 {
        dark,
        light,
    };

    enum class ed_theme_color_t : u8 {
        none,
        tab_border,
        hidden_text,
        text,
        text_dim,
        background,
        tab_inactive,
        tab_active,
        line_number_background,
        line_number,
        line_number_active,
        cursor_normal,
        cursor_insert,
        light,
        dark,
        visual_select_background,
        cursor_line_background,
        airline_background,
        mode,
        normal,
        keyword,
        identifier,
        number,
        string,
        comment,
        whitespace,
        hidden_char,
        parenthesis,
        error,
        warning,
        info,
        widget_border,
        widget_background,
        widget_active,
        widget_inactive,
        flash_color,
    };

    enum class ed_scroll_state_t : u8 {
        none,
        scroll_down,
        scroll_up,
        page_down,
        page_up,
        drag
    };

    enum class ed_window_motion_t : u8 {
        left,
        right,
        up,
        down
    };

    enum class ed_replace_range_mode_t : u8 {
        fill,
        replace,
    };

    enum class ed_region_layout_type_t : u8 {
        none,
        vbox,
        hbox,
    };

    struct ed_key_map_t final {
    };

    struct ed_airbox_t final {
        str::slice_t            text;
        vec4_t                  background;
    };

    struct ed_airline_t final {
        ed_airbox_array_t       left;
        ed_airbox_array_t       right;
    };

    struct ed_range_t final {
        u32                     beg;
        u32                     end;

        inline b8 contains(u32 loc) const {
            return loc >= beg && loc < end;
        }
    };

    struct ed_line_char_info_t final {
        vec2_t                  pos;
        vec2_t                  size;
        u32                     iter;
    };

    struct ed_span_info_t final {
        ed_lci_array_t          code_points;
        ed_range_t              range;
        vec2_t                  padding;
        vec2_t                  text_size;
        vec2_t                  widget_height;
        f32                     offset_px;
        u32                     buf_line;
        u32                     span_idx;
        b8                      split;

        inline f32 full_line_height() const {
            return padding.x + padding.y + text_size.y;
        }

        inline b8 operator<(const ed_span_info_t& rhs) const {
            if (range.beg != rhs.range.beg)
                return range.beg < rhs.range.beg;
            return range.end < rhs.range.end;
        }
    };

    struct ed_syntax_t final {
    };

    struct ed_theme_t final {
        ed_color_table_t        colortab;
        ed_theme_type_t         type;
    };

    struct ed_change_record_t final {
        u32                     beg;
        u32                     end;
    };

    struct ed_cmd_t final {
        ed_buf_t*               buffer;
        u32                     before;
        u32                     after;
        ed_change_record_t      change;
    };

    struct ed_cmd_result_t final {
        ed_cmd_t*               cmd;
        ed_cmd_flags_t          flags;
        ed_mode_t               mode_switch;
    };

    struct ed_widget_t final {
    };

    struct ed_register_t final {
        str_t                   value;
        b8                      line_wise;
    };

    struct ed_region_t final {
        ed_region_t*            parent;
        str::slice_t            name;
        ed_region_array_t       children;
        rect_t                  rect;
        vec2_t                  size;
        vec2_t                  padding;
        vec4_t                  margin;
        ed_region_flags_t       flags;
        ed_region_layout_type_t type;
    };

    struct ed_scroller_t final {
        timer_t*                delay_timer;
        timer_t*                click_timer;
        ed_region_t*            main;
        ed_region_t*            region;
        ed_region_t*            top_button;
        ed_region_t*            bot_button;
        vec2_t                  mouse_pos;
        f32                     pos;
        struct {
            f32                 line;
            f32                 page;
            f32                 mouse;
            f32                 visible;
        }                       percent;
        ed_scroll_state_t       state;
        b8                      vertical;
    };

    struct ed_range_marker_t final {
        ed_buf_t*               buf;
        timer_t*                timer;
        ed_widget_t*            widget;
        u32                     type;
        u32                     disp_row;
        u32                     disp_type;
        f32                     duration;
        ed_flash_type_t         flash_type;
        ed_tooltip_pos_t        tool_tip_pos;
    };

    struct ed_tree_node_t final {
        ed_tree_node_t*         parent;
        ed_tree_node_array_t    children;
        str::slice_t            name;
        ed_tree_node_flags_t    flags;
        b8                      expanded;
    };

    struct ed_cmd_ctx_t final {
        ed_mode_impl_t*         owner;
        ed_register_t*          reg;
        u32                     beg;
        u32                     end;
        ed_mode_t               mode;
        ed_cmd_result_t         result;
        ed_cmd_op_t             op;
        b8                      found;
    };

    struct ed_tab_window_t final {
        ed_t*                   editor;
        ed_region_t*            root;
        ed_window_t*            active;
        ed_window_array_t       windows;
        ed_region_table_t       regiontab;
        rect_t                  last_region;
    };

    struct ed_window_t final {
        ed_buf_t*               buf;
        ed_scroller_t*          scroller;
        ed_tab_window_t*        tab_window;
        rect_t                  disp_rect;
        struct {
            ed_region_t*        buffer;
            ed_region_t*        edit;
            ed_region_t*        text;
            ed_region_t*        airline;
            ed_region_t*        number;
            ed_region_t*        indicator;
            ed_region_t*        scroll;
            ed_region_t*        expanding_edit;
        }                       region;
        ed_airline_t            airline;
        ed_span_info_array_t    spans;
        f32                     x_pad;
        f32                     text_offs;
        vec2_t                  mouse_pos;
        vec2_t                  text_size;
        ed_range_t              vis_lines;
        u32                     max_disp_lines;
        u32                     default_line_size;
        ed_window_flags_t       flags;
        ed_disp_mode_t          disp_mode;
        u8                      dirty:          1;
        u8                      vis_changed:    1;
        u8                      cursor_moved:   1;
        u8                      tip_disabled:   1;
        u8                      pad:            4;
    };

    union ed_mode_kind_t final {
        ed_tree_node_t*         root;
    };

    struct ed_mode_impl_t final {
        struct {
            ed_window_t*        curr;
            ed_window_t*        launch;
        }                       win;
        struct {
            ed_key_map_t*       normal;
            ed_key_map_t*       visual;
            ed_key_map_t*       insert;
        }                       key_map;
        ed_cmd_stack_t          undo;
        ed_cmd_stack_t          redo;
        ed_mode_kind_t          kind;
        struct {
            u32                 beg;
            u32                 end;
        }                       visual;
        struct {
            ed_dir_t            last_dir;
        }                       find;
        struct {
            ed_dir_t            last_dir;
        }                       search;
        u32                     id;
        ed_mode_flags_t         flags;
        ed_mode_t               mode;
        b8                      line_wise;
    };

    struct ed_buf_t final {
        ed_syntax_t*            syntax;
        ed_theme_t*             theme;
        ed_key_notifier_t       notifier;
        gap_buf_t               gap_buf;
        ed_line_end_array_t     line_endings;
        path_t                  path;
        str::slice_t            name;
        ed_range_t              selection;
        ed_file_flags_t         file_flags;
        ed_buf_type_t           type;
    };

    struct ed_renderer_t final {
        ImFont*                 font;
        rect_t                  clip_rect;
        struct {
            ed_glyph_table_t    cache;
            vec2_t              ascii_cache[256];
            vec2_t              dot;
            vec2_t              default_char;
            f32                 height;
        }                       glyph;
        b8                      rebuild;
    };

    struct ed_config_t final {
        vec2_t                  line_margins;
        vec2_t                  widget_margins;
        vec2_t                  inline_widget_margins;
        u8                      show_scroll_bar:        1;
        u8                      show_line_numbers:      1;
        u8                      short_tab_names:        1;
        u8                      show_indicator_region:  1;
        u8                      hide_command_region:    1;
        u8                      show_normal_mode_keys:  1;
        u8                      pad:                    2;
    };

    struct ed_region_tab_t final {
        str::slice_t            name;
        u32                     color;
        ed_tab_window_t*        tab_window;
    };

    struct ed_t final {
        ed_theme_t*             theme;
        ed_mode_impl_t*         mode;
        ed_tab_window_t*        active_tab;
        struct {
            timer_t*            cursor;
            timer_t*            last_edit;
        }                       timer;
        struct {
            ed_region_t*        cmd;
            ed_region_t*        tab;
            ed_region_t*        editor;
            ed_region_t*        tab_content;
        }                       region;
        ed_renderer_t*          renderer;
        ed_buf_deque_t          buffers;
        ed_register_table_t     regtab;
        struct {
            ed_mode_table_t     global;
            ed_mode_table_t     buffer;
        }                       impl;
        ed_tab_window_array_t   tab_windows;
        ed_region_tab_array_t   tabs;
        ed_config_t             config;
        f32                     tab_offs_x;
        u8                      blink:          1;
        u8                      changed:        1;
        u8                      pad:            7;
    };

    namespace tool {
        // ----------------------------------------------------------------
        //
        // allocators tool window
        //
        // ----------------------------------------------------------------

        struct alloc_win_t final {
            alloc_t*                alloc;
            app_t*                  app;
            ImPlotContext*          ctx;
            alloc_info_t*           selected;
            f32                     table_size;
            f32                     graph_size;
            f32                     height;
            b8                      visible;
            b8                      mem_editor;
        };

        // ----------------------------------------------------------------
        //
        // strings tool window
        //
        // ----------------------------------------------------------------

        struct errors_win_t;

        // ----------------------------------------------------------------
        //
        // errors tool window
        //
        // ----------------------------------------------------------------

        struct strings_win_t;

        // ------------------------------------------------------------------------
        //
        // property editor
        //
        // ------------------------------------------------------------------------
        struct prop_editor_t;

        using type_editor_t         = b8 (*)(prop_editor_t*);
        using type_editor_table_t   = hashtab_t<u32, type_editor_t>;

        struct prop_editor_t final {
            app_t*                  app;
            u0*                     selected;
            type_editor_table_t     typetab;
            str_t                   help_heading;
            str_t                   help_text;
            ImVec2                  size;
            u32                     item_id;
            u32                     type_id;
            u32                     prop_id;
            f32                     cmd_height;
            f32                     grid_height;
            b8                      visible;
        };
    }

    namespace ed {
        enum class status_t : u32 {
            ok,
            error
        };

        namespace mod_keys {
            constexpr u8 none       = 0b00000000;
            constexpr u8 ctrl       = 0b00000001;
            constexpr u8 alt        = 0b00000010;
            constexpr u8 shift      = 0b00000100;
        }

        namespace node_flags {
            constexpr u8 none       = 0b00000000;
            constexpr u8 folder     = 0b00000001;
        }

        namespace mode_flags {
            constexpr u8 none       = 0b00000000;
            constexpr u8 insert_undo= 0b00000001;
            constexpr u8 stay_insert= 0b00000010;
        }

        namespace file_flags {
            constexpr u16 stripped_cr    = 0b0000000000000001;
            constexpr u16 null_terminated= 0b0000000000000010;
            constexpr u16 read_only      = 0b0000000000000100;
            constexpr u16 locked         = 0b0000000000001000;
            constexpr u16 dirty          = 0b0000000000010000;
            constexpr u16 has_warnings   = 0b0000000000100000;
            constexpr u16 has_errors     = 0b0000000001000000;
            constexpr u16 default_buffer = 0b0000000010000000;
            constexpr u16 has_tabs       = 0b0000000100000000;
            constexpr u16 has_spaces     = 0b0000001000000000;
            constexpr u16 insert_tabs    = 0b0000010000000000;
        }

        namespace search_types {
            constexpr u8 full_word  = 0b00000001;
            constexpr u8 begin      = 0b00000010;
            constexpr u8 end        = 0b00000100;
            constexpr u8 word       = 0b00001000;
            constexpr u8 line       = 0b00010000;
        }

        namespace range_markers {
            namespace type {
                constexpr u8 mark       = 0b00000001;
                constexpr u8 search     = 0b00000010;
                constexpr u8 widget     = 0b00000100;
                constexpr u8 line_widget= 0b00001000;
                constexpr u8 all        = (mark | search | widget | line_widget);
            }

            namespace disp_types {
                constexpr u8 hidden             = 0b00000000;
                constexpr u8 underline          = 0b00000001;
                constexpr u8 background         = 0b00000010;
                constexpr u8 tool_tip           = 0b00000100;
                constexpr u8 tool_tip_line      = 0b00001000;
                constexpr u8 cursor_tip         = 0b00010000;
                constexpr u8 cursor_tip_line    = 0b00100000;
                constexpr u8 indicator          = 0b01000000;
                constexpr u8 timed              = 0b10000000;
                constexpr u8 compile_error      = tool_tip
                                                  | cursor_tip
                                                  | indicator
                                                  | background;
                constexpr u8 background_mark    = background;
                constexpr u8 all                = underline
                                                  | tool_tip
                                                  | tool_tip_line
                                                  | cursor_tip
                                                  | cursor_tip_line
                                                  | indicator
                                                  | background;
            }
        }

        namespace region_flags {
            constexpr u8 fixed  = 0b00000001;
            constexpr u8 expand = 0b00000010;
            constexpr u8 align  = 0b00000100;
        }

        namespace window_flags {
            constexpr u16 none                  = 0b0000000000000000;
            constexpr u16 show_white_space      = 0b0000000000000001;
            constexpr u16 show_cr               = 0b0000000000000010;
            constexpr u16 show_line_numbers     = 0b0000000000000100;
            constexpr u16 show_indicators       = 0b0000000000001000;
            constexpr u16 hide_scroll_bar       = 0b0000000000010000;
            constexpr u16 modal                 = 0b0000000000100000;
            constexpr u16 wrap_text             = 0b0000000001000000;
            constexpr u16 hide_split_mark       = 0b0000000010000000;
            constexpr u16 grid_style            = 0b0000000100000000;
        }
    }

    namespace app {
        enum class status_t : u32 {
            ok,
            error,
            load_config_error,
            save_config_error,
            gl3w_init_failure,
            glfw_init_failure,
        };
    }

    // example usage:
    //
    // const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    // const ImU32 bg  = ImGui::GetColorU32(ImGuiCol_Button);
    //
    // gfx::spinner("##spinner", 15, 6, col);
    // gfx::buffering_bar("##buffer_bar", 0.7f, ImVec2(400, 6), bg, col);
    //

    u0 end_status_bar();

    b8 begin_status_bar();

    b8 spinner(const s8* label,
               f32 radius,
               s32 thickness,
               const ImU32& color);

    b8 menu_item_with_icon(const s8* icon,
                           const s8* label,
                           const s8* shortcut,
                           b8* p_selected,
                           b8 enabled);

    b8 buffering_bar(const s8* label,
                     f32 value,
                     const ImVec2& size_arg,
                     const ImU32& bg_col,
                     const ImU32& fg_col);

    b8 begin_menu_with_icon(const s8* icon,
                            const s8* label,
                            b8 enabled = true);

    b8 splitter(b8 split_vertically,
                f32 thickness,
                f32* size1,
                f32* size2,
                f32 min_size1,
                f32 min_size2,
                f32 splitter_long_axis_size = -1.0f);

    u0 text_right_align(const s8* text_begin, const s8* text_end = {});
}
