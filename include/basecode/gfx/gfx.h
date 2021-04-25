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

#include <basecode/core/types.h>
#include <basecode/core/array.h>
#include <basecode/core/hashtab.h>

#define COL32_R_SHIFT           ((u32)0)
#define COL32_G_SHIFT           ((u32)8)
#define COL32_B_SHIFT           ((u32)16)
#define COL32_A_SHIFT           ((u32)24)
#define COL32_A_MASK            0xff000000
#define COL32(R,G,B,A)          (((u32)(A)<<COL32_A_SHIFT)                      \
                                | ((u32)(B)<<COL32_B_SHIFT)                     \
                                | ((u32)(G)<<COL32_G_SHIFT)                     \
                                | ((u32)(R)<<COL32_R_SHIFT))
#define PT_TO_PX(pt)            ((pt) * 1.3281472327365263157894736842105)

struct ImFont;
struct GLFWwindow;
struct ImPlotContext;

namespace basecode {
    struct alloc_info_t;
    struct prop_decl_t;
    struct prop_editor_t;
    struct prop_decl_cmd_t;
    struct prop_decl_fld_t;

    using cmd_activate_t        = b8 (*)(prop_editor_t*, prop_decl_cmd_t*);
    using prop_decl_table_t     = hashtab_t<u32, prop_decl_t*>;
    using prop_decl_cmd_array_t = array_t<prop_decl_cmd_t>;
    using prop_decl_fld_array_t = array_t<prop_decl_fld_t>;

    enum prop_fld_type_t : u8 {
        none,
    };

    enum prop_editor_type_t : u8 {
        drag,
        color,
        label,
        range,
        slider,
        list_box,
        combo_box,
        check_box,
        input_int,
        input_box,
        input_float3,
    };

    struct rect_t final {
        f32                     x;
        f32                     y;
        f32                     w;
        f32                     h;
    };

    struct color_t final {
        u8                      r;
        u8                      g;
        u8                      b;
        u8                      a;

        explicit operator u32() const { return COL32(r, g, b, a); }
    };

    struct vector2_t final {
        f32                     x;
        f32                     y;
    };

    struct vector3_t final {
        f32                     x;
        f32                     y;
        f32                     z;
    };

    struct vector4_t final {
        f32                     x;
        f32                     y;
        f32                     z;
        f32                     w;
    };

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

    struct prop_decl_cmd_t final {
        str::slice_t            name;
        cmd_activate_t          on_activate;
    };

    union prop_fld_value_t final {
    };

    struct prop_decl_fld_t final {
        prop_fld_value_t        value;
        prop_fld_type_t         type;
    };

    struct prop_decl_t final {
        str::slice_t            name;
        str::slice_t            description;
        prop_decl_fld_array_t   fields;
        prop_decl_cmd_array_t   commands;
        u32                     family_id;
    };

    struct property_editor_t final {
        u0*                     selected;
        prop_decl_table_t       decltab;
        b8                      visible;
    };

    namespace gfx {
        struct app_t;

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

        namespace tool {
            struct alloc_win_t;
            struct errors_win_t;
            struct strings_win_t;
        }

        b8 menu_item_with_icon(const s8* icon,
                               const s8* label,
                               const s8* shortcut,
                               b8* p_selected,
                               b8 enabled);

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
}
