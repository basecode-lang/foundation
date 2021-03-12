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
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/path.h>
#include <basecode/core/string.h>

namespace basecode::graphviz {
    struct node_t;
    struct edge_t;
    struct graph_t;
    struct attr_set_t;
    struct attr_value_t;

    using edge_list_t           = array_t<edge_t*>;
    using node_list_t           = array_t<node_t*>;
    using graph_list_t          = array_t<graph_t*>;
    using attr_value_list_t     = array_t<attr_value_t*>;

    enum status_t : u8 {
        ok,
    };

    enum class color_t : u16 {
        aliceblue,
        antiquewhite,
        antiquewhite1,
        antiquewhite2,
        antiquewhite3,
        antiquewhite4,
        aqua,
        aquamarine,
        aquamarine1,
        aquamarine2,
        aquamarine3,
        aquamarine4,
        azure,
        azure1,
        azure2,
        azure3,
        azure4,
        beige,
        bisque,
        bisque1,
        bisque2,
        bisque3,
        bisque4,
        black,
        blanchedalmond,
        blue,
        blue1,
        blue2,
        blue3,
        blue4,
        blueviolet,
        brown,
        brown1,
        brown2,
        brown3,
        brown4,
        burlywood,
        burlywood1,
        burlywood2,
        burlywood3,
        burlywood4,
        cadetblue,
        cadetblue1,
        cadetblue2,
        cadetblue3,
        cadetblue4,
        chartreuse,
        chartreuse1,
        chartreuse2,
        chartreuse3,
        chartreuse4,
        chocolate,
        chocolate1,
        chocolate2,
        chocolate3,
        chocolate4,
        coral,
        coral1,
        coral2,
        coral3,
        coral4,
        cornflowerblue,
        cornsilk,
        cornsilk1,
        cornsilk2,
        cornsilk3,
        cornsilk4,
        crimson,
        cyan,
        cyan1,
        cyan2,
        cyan3,
        cyan4,
        darkblue,
        darkcyan,
        darkgoldenrod,
        darkgoldenrod1,
        darkgoldenrod2,
        darkgoldenrod3,
        darkgoldenrod4,
        darkgray,
        darkgreen,
        darkgrey,
        darkkhaki,
        darkmagenta,
        darkolivegreen,
        darkolivegreen1,
        darkolivegreen2,
        darkolivegreen3,
        darkolivegreen4,
        darkorange,
        darkorange1,
        darkorange2,
        darkorange3,
        darkorange4,
        darkorchid,
        darkorchid1,
        darkorchid2,
        darkorchid3,
        darkorchid4,
        darkred,
        darksalmon,
        darkseagreen,
        darkseagreen1,
        darkseagreen2,
        darkseagreen3,
        darkseagreen4,
        darkslateblue,
        darkslategray,
        darkslategray1,
        darkslategray2,
        darkslategray3,
        darkslategray4,
        darkslategrey,
        darkturquoise,
        darkviolet,
        deeppink,
        deeppink1,
        deeppink2,
        deeppink3,
        deeppink4,
        deepskyblue,
        deepskyblue1,
        deepskyblue2,
        deepskyblue3,
        deepskyblue4,
        dimgray,
        dimgrey,
        dodgerblue,
        dodgerblue1,
        dodgerblue2,
        dodgerblue3,
        dodgerblue4,
        firebrick,
        firebrick1,
        firebrick2,
        firebrick3,
        firebrick4,
        floralwhite,
        forestgreen,
        fuchsia,
        gainsboro,
        ghostwhite,
        gold,
        gold1,
        gold2,
        gold3,
        gold4,
        goldenrod,
        goldenrod1,
        goldenrod2,
        goldenrod3,
        goldenrod4,
        gray,
        gray0,
        gray1,
        gray10,
        gray100,
        gray11,
        gray12,
        gray13,
        gray14,
        gray15,
        gray16,
        gray17,
        gray18,
        gray19,
        gray2,
        gray20,
        gray21,
        gray22,
        gray23,
        gray24,
        gray25,
        gray26,
        gray27,
        gray28,
        gray29,
        gray3,
        gray30,
        gray31,
        gray32,
        gray33,
        gray34,
        gray35,
        gray36,
        gray37,
        gray38,
        gray39,
        gray4,
        gray40,
        gray41,
        gray42,
        gray43,
        gray44,
        gray45,
        gray46,
        gray47,
        gray48,
        gray49,
        gray5,
        gray50,
        gray51,
        gray52,
        gray53,
        gray54,
        gray55,
        gray56,
        gray57,
        gray58,
        gray59,
        gray6,
        gray60,
        gray61,
        gray62,
        gray63,
        gray64,
        gray65,
        gray66,
        gray67,
        gray68,
        gray69,
        gray7,
        gray70,
        gray71,
        gray72,
        gray73,
        gray74,
        gray75,
        gray76,
        gray77,
        gray78,
        gray79,
        gray8,
        gray80,
        gray81,
        gray82,
        gray83,
        gray84,
        gray85,
        gray86,
        gray87,
        gray88,
        gray89,
        gray9,
        gray90,
        gray91,
        gray92,
        gray93,
        gray94,
        gray95,
        gray96,
        gray97,
        gray98,
        gray99,
        green,
        green1,
        green2,
        green3,
        green4,
        greenyellow,
        grey,
        grey0,
        grey1,
        grey10,
        grey100,
        grey11,
        grey12,
        grey13,
        grey14,
        grey15,
        grey16,
        grey17,
        grey18,
        grey19,
        grey2,
        grey20,
        grey21,
        grey22,
        grey23,
        grey24,
        grey25,
        grey26,
        grey27,
        grey28,
        grey29,
        grey3,
        grey30,
        grey31,
        grey32,
        grey33,
        grey34,
        grey35,
        grey36,
        grey37,
        grey38,
        grey39,
        grey4,
        grey40,
        grey41,
        grey42,
        grey43,
        grey44,
        grey45,
        grey46,
        grey47,
        grey48,
        grey49,
        grey5,
        grey50,
        grey51,
        grey52,
        grey53,
        grey54,
        grey55,
        grey56,
        grey57,
        grey58,
        grey59,
        grey6,
        grey60,
        grey61,
        grey62,
        grey63,
        grey64,
        grey65,
        grey66,
        grey67,
        grey68,
        grey69,
        grey7,
        grey70,
        grey71,
        grey72,
        grey73,
        grey74,
        grey75,
        grey76,
        grey77,
        grey78,
        grey79,
        grey8,
        grey80,
        grey81,
        grey82,
        grey83,
        grey84,
        grey85,
        grey86,
        grey87,
        grey88,
        grey89,
        grey9,
        grey90,
        grey91,
        grey92,
        grey93,
        grey94,
        grey95,
        grey96,
        grey97,
        grey98,
        grey99,
        honeydew,
        honeydew1,
        honeydew2,
        honeydew3,
        honeydew4,
        hotpink,
        hotpink1,
        hotpink2,
        hotpink3,
        hotpink4,
        indianred,
        indianred1,
        indianred2,
        indianred3,
        indianred4,
        indigo,
        invis,
        ivory,
        ivory1,
        ivory2,
        ivory3,
        ivory4,
        khaki,
        khaki1,
        khaki2,
        khaki3,
        khaki4,
        lavender,
        lavenderblush,
        lavenderblush1,
        lavenderblush2,
        lavenderblush3,
        lavenderblush4,
        lawngreen,
        lemonchiffon,
        lemonchiffon1,
        lemonchiffon2,
        lemonchiffon3,
        lemonchiffon4,
        lightblue,
        lightblue1,
        lightblue2,
        lightblue3,
        lightblue4,
        lightcoral,
        lightcyan,
        lightcyan1,
        lightcyan2,
        lightcyan3,
        lightcyan4,
        lightgoldenrod,
        lightgoldenrod1,
        lightgoldenrod2,
        lightgoldenrod3,
        lightgoldenrod4,
        lightgoldenrodyellow,
        lightgray,
        lightgreen,
        lightgrey,
        lightpink,
        lightpink1,
        lightpink2,
        lightpink3,
        lightpink4,
        lightsalmon,
        lightsalmon1,
        lightsalmon2,
        lightsalmon3,
        lightsalmon4,
        lightseagreen,
        lightskyblue,
        lightskyblue1,
        lightskyblue2,
        lightskyblue3,
        lightskyblue4,
        lightslateblue,
        lightslategray,
        lightslategrey,
        lightsteelblue,
        lightsteelblue1,
        lightsteelblue2,
        lightsteelblue3,
        lightsteelblue4,
        lightyellow,
        lightyellow1,
        lightyellow2,
        lightyellow3,
        lightyellow4,
        lime,
        limegreen,
        linen,
        magenta,
        magenta1,
        magenta2,
        magenta3,
        magenta4,
        maroon,
        maroon1,
        maroon2,
        maroon3,
        maroon4,
        mediumaquamarine,
        mediumblue,
        mediumorchid,
        mediumorchid1,
        mediumorchid2,
        mediumorchid3,
        mediumorchid4,
        mediumpurple,
        mediumpurple1,
        mediumpurple2,
        mediumpurple3,
        mediumpurple4,
        mediumseagreen,
        mediumslateblue,
        mediumspringgreen,
        mediumturquoise,
        mediumvioletred,
        midnightblue,
        mintcream,
        mistyrose,
        mistyrose1,
        mistyrose2,
        mistyrose3,
        mistyrose4,
        moccasin,
        navajowhite,
        navajowhite1,
        navajowhite2,
        navajowhite3,
        navajowhite4,
        navy,
        navyblue,
        none,
        oldlace,
        olive,
        olivedrab,
        olivedrab1,
        olivedrab2,
        olivedrab3,
        olivedrab4,
        orange,
        orange1,
        orange2,
        orange3,
        orange4,
        orangered,
        orangered1,
        orangered2,
        orangered3,
        orangered4,
        orchid,
        orchid1,
        orchid2,
        orchid3,
        orchid4,
        palegoldenrod,
        palegreen,
        palegreen1,
        palegreen2,
        palegreen3,
        palegreen4,
        paleturquoise,
        paleturquoise1,
        paleturquoise2,
        paleturquoise3,
        paleturquoise4,
        palevioletred,
        palevioletred1,
        palevioletred2,
        palevioletred3,
        palevioletred4,
        papayawhip,
        peachpuff,
        peachpuff1,
        peachpuff2,
        peachpuff3,
        peachpuff4,
        peru,
        pink,
        pink1,
        pink2,
        pink3,
        pink4,
        plum,
        plum1,
        plum2,
        plum3,
        plum4,
        powderblue,
        purple,
        purple1,
        purple2,
        purple3,
        purple4,
        rebeccapurple,
        red,
        red1,
        red2,
        red3,
        red4,
        rosybrown,
        rosybrown1,
        rosybrown2,
        rosybrown3,
        rosybrown4,
        royalblue,
        royalblue1,
        royalblue2,
        royalblue3,
        royalblue4,
        saddlebrown,
        salmon,
        salmon1,
        salmon2,
        salmon3,
        salmon4,
        sandybrown,
        seagreen,
        seagreen1,
        seagreen2,
        seagreen3,
        seagreen4,
        seashell,
        seashell1,
        seashell2,
        seashell3,
        seashell4,
        sienna,
        sienna1,
        sienna2,
        sienna3,
        sienna4,
        silver,
        skyblue,
        skyblue1,
        skyblue2,
        skyblue3,
        skyblue4,
        slateblue,
        slateblue1,
        slateblue2,
        slateblue3,
        slateblue4,
        slategray,
        slategray1,
        slategray2,
        slategray3,
        slategray4,
        slategrey,
        snow,
        snow1,
        snow2,
        snow3,
        snow4,
        springgreen,
        springgreen1,
        springgreen2,
        springgreen3,
        springgreen4,
        steelblue,
        steelblue1,
        steelblue2,
        steelblue3,
        steelblue4,
        tan,
        tan1,
        tan2,
        tan3,
        tan4,
        teal,
        thistle,
        thistle1,
        thistle2,
        thistle3,
        thistle4,
        tomato,
        tomato1,
        tomato2,
        tomato3,
        tomato4,
        transparent,
        turquoise,
        turquoise1,
        turquoise2,
        turquoise3,
        turquoise4,
        violet,
        violetred,
        violetred1,
        violetred2,
        violetred3,
        violetred4,
        webgray,
        webgreen,
        webgrey,
        webmaroon,
        webpurple,
        wheat,
        wheat1,
        wheat2,
        wheat3,
        wheat4,
        white,
        whitesmoke,
        x11gray,
        x11green,
        x11grey,
        x11maroon,
        x11purple,
        yellow,
        yellow1,
        yellow2,
        yellow3,
        yellow4,
        yellowgreen,
    };

    enum class edge_style_t : u8 {
        dashed,
        dotted,
        solid,
        invis,
        bold,
        tapered
    };

    enum class graph_style_t : u8 {
        radial,
    };

    enum class node_style_t : u8 {
        dashed,
        dotted,
        solid,
        invis,
        bold,
        filled,
        striped,
        wedged,
        diagonals,
        rounded,
    };

    enum class cluster_style_t : u8 {
        filled,
        striped,
        rounded,
    };

    enum class arrow_type_t : u8 {
        normal,
        dot,
        odot,
        none,
        empty,
        diamond,
        ediamond,
        box,
        open_,
        vee,
        inv,
        invdot,
        invodot,
        tee,
        invempty,
        odiamond,
        crow,
        obox,
        halfopen,
    };

    enum class rank_dir_t : u8 {
        tb,
        lr,
        bt,
        rl
    };

    enum class rank_type_t : u8 {
        same,
        min,
        source,
        max,
        sink
    };

    enum class page_dir_t : u8 {
        bl,
        br,
        tl,
        tr,
        rb,
        rt,
        lb,
        lt
    };

    enum class pack_mode_t : u8 {
        node,
        cluster,
        graph,
        array,
    };

    enum class output_mode_t : u8 {
        breadth_first,
        nodes_first,
        edges_first,
    };

    enum class compass_point_t : u8 {
        n,
        ne,
        e,
        se,
        s,
        sw,
        w,
        nw,
        c,
        _
    };

    enum class dir_type_t : u8 {
        none,
        back,
        both,
        forward,
    };

    enum class graph_type_t : u8 {
        directed,
        undirected
    };

    enum class cluster_mode_t : u8 {
        none,
        local,
        global,
    };

    enum class component_type_t : u8 {
        edge,
        node,
        graph,
        subgraph,
        cluster_subgraph
    };

    enum class attr_type_t : u8 {
        rank_dir,
        fontsize,
        label,
        fillcolor,
        label_loc,
        shape,
        style,
        background,
        arrowhead,
        arrow_size,
        arrow_tail,
        bg_color,
        center,
        charset,
        cluster_rank,
        color,
        colorscheme,
        comment,
        compound,
        concentrate,
        constraint,
        decorate,
        dir,
        distortion,
        esep,
        fixed_size,
        fontcolor,
        font_name,
        font_path,
        force_labels,
        gradient_angle,
        group,
        head_clip,
        head_label,
        head_port,
        height,
        image,
        image_path,
        image_pos,
        image_scale,
        label_angle,
        label_distance,
        label_float,
        label_font_color,
        label_font_name,
        label_font_size,
        label_just,
        landscape,
        layer,
        layer_list_sep,
        layers,
        layer_select,
        layer_sep,
        layout,
        lhead,
        ltail,
        margin,
        mc_limit,
        min_len,
        new_rank,
        node_sep,
        no_justify,
        normalize,
        ns_limit,
        ns_limit1,
        ordering,
        orientation,
        output_order,
        overlap,
        pack,
        pack_mode,
        pad,
        page,
        page_dir,
        pencolor,
        pen_width,
        peripheries,
        pos,
        quantum,
        rank,
        rank_sep,
        ratio,
        regular,
        re_min_cross,
        rotate,
        same_head,
        same_tail,
        sample_points,
        scale,
        search_size,
        sep,
        shapefile,
        show_boxes,
        sides,
        size,
        skew,
        sort_v,
        splines,
        tail_clip,
        tail_label,
        tail_port,
        viewport,
        voro_margin,
        weight,
        width,
        xlabel,
        z
    };

    enum class attr_value_type_t : u8 {
        string,
        boolean,
        integer,
        enumeration,
        floating_point,
    };

    struct hsv_t final {
        f64                     h, s, v;
    };

    struct rgb_t final {
        u8                      r, g, b;
    };

    struct rgba_t final {
        u8                      r, g, b, a;
    };

    struct rect_t final {
        f64                     x1, y1;
        f64                     x2, y2;
    };

    struct point_t final {
        f64                     x, y;
    };

    struct viewport_t final {
        f64                     w, h, z;
        f64                     x, y;
    };

    struct attr_value_t final {
        union {
            b8                  f;
            u32                 dw;
            f64                 fqw;
        }                       value;
        u8                      type:       4;
        u8                      value_type: 4;
    };

    struct attr_set_t final {
        alloc_t*                alloc;
        attr_value_list_t       attrs;
        component_type_t        type;
    };

    struct node_t final {
        attr_set_t              attrs;
        intern_id               name;
    };

    struct edge_t final {
        node_t*                 first;
        node_t*                 second;
        attr_set_t              attrs;
    };

    struct graph_t final {
        alloc_t*                alloc;
        graph_t*                parent;
        edge_list_t             edges;
        node_list_t             nodes;
        graph_list_t            subgraphs;
        attr_set_t              attrs;
        intern_id               name;
        graph_type_t            type;
    };

    namespace node {
        u0 free(node_t& n);

        u0 z(node_t& n, f64 v);

        u0 skew(node_t& n, f64 v);

        u0 sortv(node_t& n, b8 v);

        u0 width(node_t& n, f64 v);

        u0 sides(node_t& n, u32 v);

        u0 margin(node_t& n, f64 v);

        u0 regular(node_t& n, b8 v);

        u0 xlp(node_t& n, point_t v);

        u0 color(node_t& n, rgb_t v);

        u0 pos(node_t& n, point_t v);

        u0 color(node_t& n, color_t v);

        u0 fixed_size(node_t& n, b8 v);

        u0 font_size(node_t& e, f64 v);

        u0 no_justify(node_t& n, b8 v);

        u0 pen_width(node_t& n, f64 v);

        u0 show_boxes(node_t& n, u32 v);

        u0 margin(node_t& n, point_t v);

        u0 distortion(node_t& n, f64 v);

        u0 image_scale(node_t& n, b8 v);

        u0 peripheries(node_t& n, u32 v);

        u0 orientation(node_t& n, f64 v);

        u0 fill_color(node_t& n, rgb_t v);

        u0 font_color(node_t& n, rgb_t v);

        u0 sample_points(node_t& n, u32 v);

        u0 fill_color(node_t& n, rgba_t v);

        u0 font_color(node_t& n, rgba_t v);

        u0 style(node_t& n, node_style_t v);

        // XXX
        u0 shape(node_t& n, str::slice_t v);

        u0 label(node_t& n, str::slice_t v);

        // XXX
        u0 layer(node_t& n, str::slice_t v);

        u0 image(node_t& n, str::slice_t v);

        u0 fill_color(node_t& n, color_t v);

        u0 font_color(node_t& n, color_t v);

        u0 group(node_t& n, str::slice_t v);

        u0 gradient_angle(node_t& n, u32 v);

        u0 comment(node_t& n, str::slice_t v);

        u0 ordering(node_t& n, str::slice_t v);

        // XXX
        u0 image_pos(node_t& n, str::slice_t v);

        u0 font_name(node_t& n, str::slice_t v);

        // XXX
        u0 label_loc(node_t& n, str::slice_t v);

        u0 shape_file(node_t& n, const path_t& v);

        u0 color_scheme(node_t& n, str::slice_t v);

        status_t init(node_t& n, alloc_t* alloc = context::top()->alloc);
    }

    namespace edge {
        u0 free(edge_t& e);

        u0 weight(edge_t& e, u32 v);

        u0 weight(edge_t& e, f64 v);

        u0 color(edge_t& e, rgb_t v);

        u0 min_len(edge_t& e, u32 v);

        u0 decorate(edge_t& e, b8 v);

        u0 pos(edge_t& e, point_t v);

        u0 color(edge_t& e, rgba_t v);

        u0 tail_clip(edge_t& e, b8 v);

        u0 head_clip(edge_t& e, b8 v);

        u0 color(edge_t& e, color_t v);

        u0 constraint(edge_t& e, b8 v);

        u0 no_justify(edge_t& e, b8 v);

        u0 pen_width(edge_t& e, f64 v);

        u0 show_boxes(edge_t& e, b8 v);

        u0 font_size(edge_t& e, f64 v);

        u0 dir(edge_t& e, dir_type_t v);

        u0 arrow_size(edge_t& e, f64 v);

        u0 label_float(edge_t& e, b8 v);

        u0 label_angle(edge_t& e, f64 v);

        u0 fill_color(edge_t& e, rgb_t v);

        u0 font_color(edge_t& e, rgb_t v);

        u0 fill_color(edge_t& e, rgba_t v);

        u0 font_color(edge_t& e, rgba_t v);

        u0 font_color(edge_t& e, color_t v);

        u0 fill_color(edge_t& e, color_t v);

        u0 label(edge_t& e, str::slice_t v);

        u0 label_distance(edge_t& e, f64 v);

        u0 layer(edge_t& e, str::slice_t v);

        u0 lhead(edge_t& e, str::slice_t v);

        u0 ltail(edge_t& e, str::slice_t v);

        u0 label_font_size(edge_t& e, f64 v);

        u0 comment(edge_t& e, str::slice_t v);

        u0 label_font_color(edge_t& e, rgb_t v);

        u0 font_name(edge_t& e, str::slice_t v);

        u0 same_head(edge_t& e, str::slice_t v);

        u0 same_tail(edge_t& e, str::slice_t v);

        // XXX
        u0 head_port(edge_t& e, str::slice_t v);

        u0 arrow_head(edge_t& e, arrow_type_t v);

        // XXX
        u0 tail_port(edge_t& e, str::slice_t v);

        u0 tail_label(edge_t& e, str::slice_t v);

        u0 arrow_tail(edge_t& e, arrow_type_t v);

        u0 head_label(edge_t& e, str::slice_t v);

        u0 label_font_color(edge_t& e, rgba_t v);

        u0 label_font_color(edge_t& e, color_t v);

        u0 color_scheme(edge_t& e, str::slice_t v);

        u0 label_font_name(edge_t& e, str::slice_t v);

        status_t init(edge_t& e, alloc_t* alloc = context::top()->alloc);
    }

    namespace graph {
        u0 free(graph_t& g);

        u0 pad(graph_t& g, f64 v);

        u0 pack(graph_t& g, b8 v);

        u0 size(graph_t& g, f64 v);

        u0 page(graph_t& g, f64 v);

        u0 pack(graph_t& g, u32 v);

        u0 sortv(graph_t& g, u32 v);

        // XXX:
        u0 ratio(graph_t& g, f64 v);

        u0 scale(graph_t& g, f64 v);

        u0 center(graph_t& g, b8 v);

        u0 overlap(graph_t& g, b8 v);

        u0 rotate(graph_t& g, u32 v);

        u0 margin(graph_t& g, f64 v);

        u0 lwidth(graph_t& g, f64 v);

        u0 splines(graph_t& g, b8 v);

        u0 lp(graph_t& g, point_t v);

        u0 pad(graph_t& g, point_t v);

        u0 new_rank(graph_t& g, b8 v);

        u0 min_len(graph_t& g, u32 v);

        u0 lheight(graph_t& g, f64 v);

        u0 compound(graph_t& g, b8 v);

        u0 color(graph_t& g, rgb_t v);

        u0 damping(graph_t& g, f64 v);

        u0 quantum(graph_t& g, f64 v);

        u0 page(graph_t& g, point_t v);

        u0 node_sep(graph_t& g, f64 v);

        u0 color(graph_t& g, rgba_t v);

        u0 size(graph_t& g, point_t v);

        u0 landscape(graph_t& g, b8 v);

        // XXX:
        u0 rank_sep(graph_t& g, f64 v);

        u0 mc_limit(graph_t& g, f64 v);

        u0 ns_limit(graph_t& g, f64 v);

        u0 scale(graph_t& g, point_t v);

        u0 show_boxes(graph_t& g, b8 v);

        u0 color(graph_t& g, color_t v);

        u0 no_justify(graph_t& g, b8 v);

        u0 normalize(graph_t& g, f64 v);

        u0 ns_limit1(graph_t& g, f64 v);

        u0 font_size(graph_t& g, f64 v);

        // XXX: color list?
        u0 bg_color(graph_t& g, rgb_t v);

        u0 margin(graph_t& g, point_t v);

        u0 concentrate(graph_t& g, b8 v);

        u0 voro_margin(graph_t& g, f64 v);

        u0 bg_color(graph_t& g, rgba_t v);

        u0 force_labels(graph_t& g, b8 v);

        u0 orientation(graph_t& g, f64 v);

        u0 re_min_cross(graph_t& g, b8 v);

        u0 bb(graph_t& g, const rect_t& v);

        u0 bg_color(graph_t& g, color_t v);

        u0 label(graph_t& g, str::slice_t v);

        u0 gradient_angle(graph_t& g, u32 v);

        // XXX
        u0 style(graph_t& g, str::slice_t v);

        u0 layout(graph_t& g, str::slice_t v);

        u0 class_(graph_t& g, str::slice_t v);

        u0 layers(graph_t& g, str::slice_t v);

        u0 rank_dir(graph_t& g, rank_dir_t v);

        u0 page_dir(graph_t& g, page_dir_t v);

        u0 viewport(graph_t& g, viewport_t v);

        // XXX:
        u0 splines(graph_t& g, str::slice_t v);

        u0 charset(graph_t& g, str::slice_t v);

        u0 comment(graph_t& g, str::slice_t v);

        u0 pack_mode(graph_t& g, pack_mode_t v);

        u0 ordering(graph_t& g, str::slice_t v);

        u0 label_loc(graph_t& g, str::slice_t v);

        u0 label_just(graph_t& g, str::slice_t v);

        u0 font_path(graph_t& g, const path_t& v);

        u0 background(graph_t& g, str::slice_t v);

        u0 layers_sep(graph_t& g, str::slice_t v);

        u0 image_path(graph_t& g, const path_t& v);

        u0 orientation(graph_t& g, str::slice_t v);

        u0 color_scheme(graph_t& g, str::slice_t v);

        u0 output_order(graph_t& g, output_mode_t v);

        u0 layers_select(graph_t& g, str::slice_t v);

        u0 layer_list_sep(graph_t& g, str::slice_t v);

        status_t init(graph_t& g, graph_type_t type, alloc_t* alloc = context::top()->alloc);
    }

    namespace attr_set {
        u0 free(attr_set_t& set);

        status_t init(attr_set_t& set, alloc_t* alloc = context::top()->alloc);
    }
}
