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

#include <basecode/core/graphviz/gv.h>

namespace basecode::graphviz {
    namespace attr {
        static str::slice_t s_type_names[] = {
            [u32(attr_type_t::rank_dir)]        = "rankdir"_ss,
            [u32(attr_type_t::font_size)]       = "fontsize"_ss,
            [u32(attr_type_t::label)]           = "label"_ss,
            [u32(attr_type_t::fill_color)]      = "fillcolor"_ss,
            [u32(attr_type_t::label_loc)]       = "labelloc"_ss,
            [u32(attr_type_t::shape)]           = "shape"_ss,
            [u32(attr_type_t::style)]           = "style"_ss,
            [u32(attr_type_t::background)]      = "background"_ss,
            [u32(attr_type_t::arrow_head)]      = "arrowhead"_ss,
            [u32(attr_type_t::arrow_size)]      = "arrowsize"_ss,
            [u32(attr_type_t::arrow_tail)]      = "arrowtail"_ss,
            [u32(attr_type_t::bg_color)]        = "bgcolor"_ss,
            [u32(attr_type_t::center)]          = "center"_ss,
            [u32(attr_type_t::charset)]         = "charset"_ss,
            [u32(attr_type_t::cluster_rank)]    = "clusterrank"_ss,
            [u32(attr_type_t::color)]           = "color"_ss,
            [u32(attr_type_t::color_scheme)]    = "colorscheme"_ss,
            [u32(attr_type_t::comment)]         = "comment"_ss,
            [u32(attr_type_t::compound)]        = "compound"_ss,
            [u32(attr_type_t::concentrate)]     = "concentrate"_ss,
            [u32(attr_type_t::constraint)]      = "constraint"_ss,
            [u32(attr_type_t::decorate)]        = "decorate"_ss,
            [u32(attr_type_t::dir)]             = "dir"_ss,
            [u32(attr_type_t::distortion)]      = "distortion"_ss,
            [u32(attr_type_t::esep)]            = "esep"_ss,
            [u32(attr_type_t::fixed_size)]      = "fixedsize"_ss,
            [u32(attr_type_t::font_color)]      = "fontcolor"_ss,
            [u32(attr_type_t::font_name)]       = "fontname"_ss,
            [u32(attr_type_t::font_path)]       = "fontpath"_ss,
            [u32(attr_type_t::force_labels)]    = "forcelabels"_ss,
            [u32(attr_type_t::gradient_angle)]  = "gradientangle"_ss,
            [u32(attr_type_t::group)]           = "group"_ss,
            [u32(attr_type_t::head_clip)]       = "headclip"_ss,
            [u32(attr_type_t::head_label)]      = "headlabel"_ss,
            [u32(attr_type_t::head_port)]       = "headport"_ss,
            [u32(attr_type_t::height)]          = "height"_ss,
            [u32(attr_type_t::image)]           = "image"_ss,
            [u32(attr_type_t::image_path)]      = "imagepath"_ss,
            [u32(attr_type_t::image_pos)]       = "imagepos"_ss,
            [u32(attr_type_t::image_scale)]     = "imagescale"_ss,
            [u32(attr_type_t::label_angle)]     = "labelangle"_ss,
            [u32(attr_type_t::label_distance)]  = "labeldistance"_ss,
            [u32(attr_type_t::label_float)]     = "labelfloat"_ss,
            [u32(attr_type_t::label_font_color)]= "labelfontcolor"_ss,
            [u32(attr_type_t::label_font_name)] = "labelfontname"_ss,
            [u32(attr_type_t::label_font_size)] = "labelfontsize"_ss,
            [u32(attr_type_t::label_just)]      = "labeljust"_ss,
            [u32(attr_type_t::landscape)]       = "landscape"_ss,
            [u32(attr_type_t::layer)]           = "layer"_ss,
            [u32(attr_type_t::layer_list_sep)]  = "layerlistsep"_ss,
            [u32(attr_type_t::layers)]          = "layers"_ss,
            [u32(attr_type_t::layer_select)]    = "layerselect"_ss,
            [u32(attr_type_t::layer_sep)]       = "layersep"_ss,
            [u32(attr_type_t::layout)]          = "layout"_ss,
            [u32(attr_type_t::lhead)]           = "lhead"_ss,
            [u32(attr_type_t::ltail)]           = "ltail"_ss,
            [u32(attr_type_t::margin)]          = "margin"_ss,
            [u32(attr_type_t::mc_limit)]        = "mclimit"_ss,
            [u32(attr_type_t::min_len)]         = "minlen"_ss,
            [u32(attr_type_t::new_rank)]        = "newrank"_ss,
            [u32(attr_type_t::node_sep)]        = "nodesep"_ss,
            [u32(attr_type_t::no_justify)]      = "nojustify"_ss,
            [u32(attr_type_t::normalize)]       = "normalize"_ss,
            [u32(attr_type_t::ns_limit)]        = "nslimit"_ss,
            [u32(attr_type_t::ns_limit1)]       = "nslimit1"_ss,
            [u32(attr_type_t::ordering)]        = "ordering"_ss,
            [u32(attr_type_t::orientation)]     = "orientation"_ss,
            [u32(attr_type_t::output_order)]    = "outputorder"_ss,
            [u32(attr_type_t::overlap)]         = "overlap"_ss,
            [u32(attr_type_t::pack)]            = "pack"_ss,
            [u32(attr_type_t::pack_mode)]       = "packmode"_ss,
            [u32(attr_type_t::pad)]             = "pad"_ss,
            [u32(attr_type_t::page)]            = "page"_ss,
            [u32(attr_type_t::page_dir)]        = "pagedir"_ss,
            [u32(attr_type_t::pen_color)]       = "pencolor"_ss,
            [u32(attr_type_t::pen_width)]       = "penwidth"_ss,
            [u32(attr_type_t::peripheries)]     = "peripheries"_ss,
            [u32(attr_type_t::pos)]             = "pos"_ss,
            [u32(attr_type_t::quantum)]         = "quantum"_ss,
            [u32(attr_type_t::rank)]            = "rank"_ss,
            [u32(attr_type_t::rank_sep)]        = "rank_sep"_ss,
            [u32(attr_type_t::ratio)]           = "ratio"_ss,
            [u32(attr_type_t::regular)]         = "regular"_ss,
            [u32(attr_type_t::re_min_cross)]    = "remincross"_ss,
            [u32(attr_type_t::rotate)]          = "rotate"_ss,
            [u32(attr_type_t::same_head)]       = "samehead"_ss,
            [u32(attr_type_t::same_tail)]       = "sametail"_ss,
            [u32(attr_type_t::sample_points)]   = "samplepoints"_ss,
            [u32(attr_type_t::scale)]           = "scale"_ss,
            [u32(attr_type_t::search_size)]     = "searchsize"_ss,
            [u32(attr_type_t::sep)]             = "sep"_ss,
            [u32(attr_type_t::shape_file)]      = "shapefile"_ss,
            [u32(attr_type_t::show_boxes)]      = "showboxes"_ss,
            [u32(attr_type_t::sides)]           = "sides"_ss,
            [u32(attr_type_t::size)]            = "size"_ss,
            [u32(attr_type_t::skew)]            = "skew"_ss,
            [u32(attr_type_t::sort_v)]          = "sortv"_ss,
            [u32(attr_type_t::splines)]         = "splines"_ss,
            [u32(attr_type_t::tail_clip)]       = "tailclip"_ss,
            [u32(attr_type_t::tail_label)]      = "taillabel"_ss,
            [u32(attr_type_t::tail_port)]       = "tailport"_ss,
            [u32(attr_type_t::viewport)]        = "viewport"_ss,
            [u32(attr_type_t::voro_margin)]     = "voromargin"_ss,
            [u32(attr_type_t::weight)]          = "weight"_ss,
            [u32(attr_type_t::width)]           = "width"_ss,
            [u32(attr_type_t::xlabel)]          = "xlabel"_ss,
            [u32(attr_type_t::z)]               = "z"_ss
        };

        static str::slice_t s_arrow_type_names[] = {
            [u32(arrow_type_t::normal)]         = "normal"_ss,
            [u32(arrow_type_t::dot)]            = "dot"_ss,
            [u32(arrow_type_t::odot)]           = "odot"_ss,
            [u32(arrow_type_t::none)]           = "none"_ss,
            [u32(arrow_type_t::empty)]          = "empty"_ss,
            [u32(arrow_type_t::diamond)]        = "diamond"_ss,
            [u32(arrow_type_t::ediamond)]       = "ediamond"_ss,
            [u32(arrow_type_t::box)]            = "box"_ss,
            [u32(arrow_type_t::open_)]          = "open"_ss,
            [u32(arrow_type_t::vee)]            = "vee"_ss,
            [u32(arrow_type_t::inv)]            = "inv"_ss,
            [u32(arrow_type_t::invdot)]         = "invdot"_ss,
            [u32(arrow_type_t::invodot)]        = "invodot"_ss,
            [u32(arrow_type_t::tee)]            = "tee"_ss,
            [u32(arrow_type_t::invempty)]       = "invempty"_ss,
            [u32(arrow_type_t::odiamond)]       = "odiamond"_ss,
            [u32(arrow_type_t::crow)]           = "crow"_ss,
            [u32(arrow_type_t::obox)]           = "obox"_ss,
            [u32(arrow_type_t::halfopen)]       = "halfopen"_ss,
        };

        str::slice_t type_name(attr_type_t type) {
            return s_type_names[u32(type)];
        }

        str::slice_t arrow_type_name(arrow_type_t type) {
            return s_arrow_type_names[u32(type)];
        }

        u0 serialize(graph_t& g, const attr_value_t& attr, mem_buf_t& mb) {
            format::format_to(mb, "{}=", type_name(attr_type_t(attr.type)));
            switch (attr_type_t(attr.type)) {
                // enumeration
                case attr_type_t::dir:
                case attr_type_t::rank:
                case attr_type_t::shape:
                case attr_type_t::style:
                case attr_type_t::charset:
                case attr_type_t::rank_dir:
                case attr_type_t::page_dir:
                case attr_type_t::ordering:
                case attr_type_t::image_pos:
                case attr_type_t::label_loc:
                case attr_type_t::pack_mode:
                    break;
                case attr_type_t::arrow_head:
                case attr_type_t::arrow_tail: {
                    const auto type = arrow_type_t(attr.value.dw);
                    format::format_to(mb, "{}", arrow_type_name(type));
                    break;
                }
                case attr_type_t::color_scheme:
                case attr_type_t::cluster_rank:
                case attr_type_t::output_order:
                    break;

                // string
                case attr_type_t::label:
                case attr_type_t::group:
                case attr_type_t::lhead:
                case attr_type_t::ltail:
                case attr_type_t::image:
                case attr_type_t::xlabel:
                case attr_type_t::comment:
                case attr_type_t::font_name:
                case attr_type_t::same_head:
                case attr_type_t::same_tail:
                case attr_type_t::font_path:
                case attr_type_t::head_label:
                case attr_type_t::tail_label:
                case attr_type_t::image_path:
                case attr_type_t::label_font_name:
                    break;

                // numbers
                case attr_type_t::sep:
                case attr_type_t::pad:
                case attr_type_t::pack:
                case attr_type_t::page:
                case attr_type_t::skew:
                case attr_type_t::size:
                case attr_type_t::esep:
                case attr_type_t::ratio:
                case attr_type_t::sides:
                case attr_type_t::scale:
                case attr_type_t::width:
                case attr_type_t::margin:
                case attr_type_t::sort_v:
                case attr_type_t::height:
                case attr_type_t::weight:
                case attr_type_t::rotate:
                case attr_type_t::quantum:
                case attr_type_t::min_len:
                case attr_type_t::mc_limit:
                case attr_type_t::ns_limit:
                case attr_type_t::rank_sep:
                case attr_type_t::node_sep:
                case attr_type_t::ns_limit1:
                case attr_type_t::normalize:
                case attr_type_t::pen_width:
                case attr_type_t::font_size:
                case attr_type_t::arrow_size:
                case attr_type_t::distortion:
                case attr_type_t::show_boxes:
                case attr_type_t::image_scale:
                case attr_type_t::label_angle:
                case attr_type_t::voro_margin:
                case attr_type_t::search_size:
                case attr_type_t::orientation:
                case attr_type_t::peripheries:
                case attr_type_t::label_distance:
                case attr_type_t::gradient_angle:
                case attr_type_t::label_font_size:
                    break;

                // colors
                case attr_type_t::color:
                case attr_type_t::bg_color:
                case attr_type_t::pen_color:
                case attr_type_t::fill_color:
                case attr_type_t::font_color:
                case attr_type_t::label_font_color:
                    break;

                // boolean
                case attr_type_t::center:
                case attr_type_t::regular:
                case attr_type_t::new_rank:
                case attr_type_t::decorate:
                case attr_type_t::compound:
                case attr_type_t::head_clip:
                case attr_type_t::tail_clip:
                case attr_type_t::landscape:
                case attr_type_t::fixed_size:
                case attr_type_t::constraint:
                case attr_type_t::label_just:
                case attr_type_t::no_justify:
                case attr_type_t::concentrate:
                case attr_type_t::label_float:
                case attr_type_t::force_labels:
                case attr_type_t::re_min_cross:
                    break;

                // custom
                case attr_type_t::pos:
                case attr_type_t::overlap:
                case attr_type_t::viewport:
                case attr_type_t::head_port:
                case attr_type_t::tail_port:
                    break;

                // not supported
                case attr_type_t::z:
                case attr_type_t::layer:
                case attr_type_t::layout:
                case attr_type_t::layers:
                case attr_type_t::splines:
                case attr_type_t::layer_sep:
                case attr_type_t::shape_file:
                case attr_type_t::background:
                case attr_type_t::layer_select:
                case attr_type_t::sample_points:
                case attr_type_t::layer_list_sep:
                    break;
            }
        }
    }

    namespace node {
        u0 free(node_t& n) {
            attr_set::free(n.attrs);
        }

        u0 skew(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::skew, v);
        }

        u0 sortv(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::sort_v, v);
        }

        u0 width(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::width, v);
        }

        u0 sides(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::sides, v);
        }

        u0 height(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::height, v);
        }

        u0 margin(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::margin, v);
        }

        u0 regular(node_t& n, b8 v) {
            attr_set::set(n.attrs, attr_type_t::regular, v);
        }

        u0 color(node_t& n, rgb_t v) {
            attr_set::set(n.attrs, attr_type_t::color, v);
        }

        u0 pos(node_t& n, point_t v) {
            attr_set::set(n.attrs, attr_type_t::pos, v);
        }

        u0 color(node_t& n, color_t v) {
            attr_set::set(n.attrs, attr_type_t::color, v);
        }

        u0 fixed_size(node_t& n, b8 v) {
            attr_set::set(n.attrs, attr_type_t::fixed_size, v);
        }

        u0 font_size(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::font_size, v);
        }

        u0 no_justify(node_t& n, b8 v) {
            attr_set::set(n.attrs, attr_type_t::no_justify, v);
        }

        u0 shape(node_t& n, shape_t v) {
            attr_set::set(n.attrs, attr_type_t::shape, u32(v));
        }

        u0 pen_width(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::pen_width, v);
        }

        u0 show_boxes(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::show_boxes, v);
        }

        u0 margin(node_t& n, point_t v) {
            attr_set::set(n.attrs, attr_type_t::margin, v);
        }

        u0 distortion(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::distortion, v);
        }

        u0 peripheries(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::peripheries, v);
        }

        u0 orientation(node_t& n, f64 v) {
            attr_set::set(n.attrs, attr_type_t::orientation, v);
        }

        u0 fill_color(node_t& n, rgb_t v) {
            attr_set::set(n.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(node_t& n, rgb_t v) {
            attr_set::set(n.attrs, attr_type_t::font_color, v);
        }

        u0 sample_points(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::sample_points, v);
        }

        u0 fill_color(node_t& n, rgba_t v) {
            attr_set::set(n.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(node_t& n, rgba_t v) {
            attr_set::set(n.attrs, attr_type_t::font_color, v);
        }

        u0 style(node_t& n, node_style_t v) {
            attr_set::set(n.attrs, attr_type_t::style, u32(v));
        }

        u0 label(node_t& n, str::slice_t v) {
            attr_set::set(n.attrs, attr_type_t::label, v);
        }

        u0 layer(node_t& n, str::slice_t v) {
        }

        u0 image(node_t& n, str::slice_t v) {
            attr_set::set(n.attrs, attr_type_t::image, v);
        }

        u0 fill_color(node_t& n, color_t v) {
            attr_set::set(n.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(node_t& n, color_t v) {
            attr_set::set(n.attrs, attr_type_t::font_color, v);
        }

        u0 group(node_t& n, str::slice_t v) {
            attr_set::set(n.attrs, attr_type_t::group, v);
        }

        u0 gradient_angle(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::gradient_angle, v);
        }

        u0 ordering(node_t& n, ordering_t v) {
            attr_set::set(n.attrs, attr_type_t::ordering, u32(v));
        }

        u0 comment(node_t& n, str::slice_t v) {
            attr_set::set(n.attrs, attr_type_t::comment, v);
        }

        u0 image_pos(node_t& n, image_pos_t v) {
            attr_set::set(n.attrs, attr_type_t::image_pos, u32(v));
        }

        u0 font_name(node_t& n, str::slice_t v) {
            attr_set::set(n.attrs, attr_type_t::font_name, v);
        }

        u0 image_scale(node_t& n, image_scale_t v) {
            attr_set::set(n.attrs, attr_type_t::image_scale, u32(v));
        }

        u0 label_loc(node_t& n, node_label_loc_t v) {
            attr_set::set(n.attrs, attr_type_t::label_loc, u32(v));
        }

        u0 color_scheme(node_t& n, color_scheme_t v) {
            attr_set::set(n.attrs, attr_type_t::color_scheme, u32(v));
        }

        u0 serialize(graph_t& g, const node_t& n, mem_buf_t& mb) {
            auto node_name = string::interned::get(n.name);
            if (!OK(node_name.status))
                return;
            format::format_to(mb, "{}", node_name.slice);
            if (n.attrs.values.size > 0) {
                format::format_to(mb, "[");
                for (u32 i = 0; i < n.attrs.values.size; ++i) {
                    if (i > 0) format::format_to(mb, ", ");
                    attr::serialize(g, n.attrs.values[i], mb);
                }
                format::format_to(mb, "]");
            }
            format::format_to(mb, ";");
        }

        status_t init(node_t& n, u32 id, str::slice_t name, alloc_t* alloc) {
            auto r = string::interned::fold_for_result(name);
            n.id   = id;
            n.name = r.id;
            attr_set::init(n.attrs, component_type_t::node, alloc);
            return status_t::ok;
        }
    }

    namespace edge {
        u0 free(edge_t& e) {
            attr_set::free(e.attrs);
        }

        u0 weight(edge_t& e, u32 v) {
            attr_set::set(e.attrs, attr_type_t::weight, v);
        }

        u0 weight(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::weight, v);
        }

        u0 color(edge_t& e, rgb_t v) {
            attr_set::set(e.attrs, attr_type_t::color, v);
        }

        u0 min_len(edge_t& e, u32 v) {
            attr_set::set(e.attrs, attr_type_t::min_len, v);
        }

        u0 decorate(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::decorate, v);
        }

        u0 pos(edge_t& e, point_t v) {
            attr_set::set(e.attrs, attr_type_t::pos, v);
        }

        u0 color(edge_t& e, rgba_t v) {
            attr_set::set(e.attrs, attr_type_t::color, v);
        }

        u0 tail_clip(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::tail_clip, v);
        }

        u0 head_clip(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::head_clip, v);
        }

        u0 color(edge_t& e, color_t v) {
            attr_set::set(e.attrs, attr_type_t::color, v);
        }

        u0 constraint(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::constraint, v);
        }

        u0 no_justify(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::no_justify, v);
        }

        u0 pen_width(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::pen_width, v);
        }

        u0 font_size(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::font_size, v);
        }

        u0 show_boxes(edge_t& e, u32 v) {
            attr_set::set(e.attrs, attr_type_t::show_boxes, v);
        }

        u0 dir(edge_t& e, dir_type_t v) {
            attr_set::set(e.attrs, attr_type_t::dir, u32(v));
        }

        u0 arrow_size(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::arrow_size, v);
        }

        u0 label_float(edge_t& e, b8 v) {
            attr_set::set(e.attrs, attr_type_t::label_float, v);
        }

        u0 label_angle(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::label_angle, v);
        }

        u0 fill_color(edge_t& e, rgb_t v) {
            attr_set::set(e.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(edge_t& e, rgb_t v) {
            attr_set::set(e.attrs, attr_type_t::font_color, v);
        }

        u0 fill_color(edge_t& e, rgba_t v) {
            attr_set::set(e.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(edge_t& e, rgba_t v) {
            attr_set::set(e.attrs, attr_type_t::font_color, v);
        }

        u0 font_color(edge_t& e, color_t v) {
            attr_set::set(e.attrs, attr_type_t::font_color, v);
        }

        u0 fill_color(edge_t& e, color_t v) {
            attr_set::set(e.attrs, attr_type_t::fill_color, v);
        }

        u0 label(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::label, v);
        }

        u0 label_distance(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::label_distance, v);
        }

        u0 layer(edge_t& e, str::slice_t v) {
        }

        u0 style(edge_t& e, edge_style_t v) {
            attr_set::set(e.attrs, attr_type_t::style, u32(v));
        }

        u0 lhead(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::lhead, v);
        }

        u0 ltail(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::ltail, v);
        }

        u0 label_font_size(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::label_font_size, v);
        }

        u0 comment(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::comment, v);
        }

        u0 label_font_color(edge_t& e, rgb_t v) {
            attr_set::set(e.attrs, attr_type_t::label_font_color, v);
        }

        u0 font_name(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::font_name, v);
        }

        u0 same_head(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::same_head, v);
        }

        u0 same_tail(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::same_tail, v);
        }

        u0 head_port(edge_t& e, str::slice_t v) {
        }

        u0 arrow_head(edge_t& e, arrow_type_t v) {
            attr_set::set(e.attrs, attr_type_t::arrow_head, u32(v));
        }

        u0 tail_port(edge_t& e, str::slice_t v) {
        }

        u0 tail_label(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::tail_label, v);
        }

        u0 arrow_tail(edge_t& e, arrow_type_t v) {
            attr_set::set(e.attrs, attr_type_t::arrow_tail, u32(v));
        }

        u0 head_label(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::head_label, v);
        }

        u0 label_font_color(edge_t& e, rgba_t v) {
            attr_set::set(e.attrs, attr_type_t::label_font_color, v);
        }

        u0 label_font_color(edge_t& e, color_t v) {
            attr_set::set(e.attrs, attr_type_t::label_font_color, v);
        }

        u0 color_scheme(edge_t& e, color_scheme_t v) {
            attr_set::set(e.attrs, attr_type_t::color_scheme, u32(v));
        }

        u0 label_font_name(edge_t& e, str::slice_t v) {
            attr_set::set(e.attrs, attr_type_t::label_font_name, v);
        }

        u0 serialize(graph_t& g, const edge_t& e, mem_buf_t& mb) {
            const auto node_connector = g.type == graph_type_t::directed ? "->"_ss : "--"_ss;
            auto lhs = graph::get_node(g, e.first);
            auto rhs = graph::get_node(g, e.second);
            if (!lhs || !rhs)
                return;
            auto lhs_name = string::interned::get(lhs->name);
            auto rhs_name = string::interned::get(rhs->name);
            format::format_to(mb, "{} {} {}", lhs_name.slice, node_connector, rhs_name.slice);
            if (e.attrs.values.size > 0) {
                format::format_to(mb, "[");
                for (u32 i = 0; i < e.attrs.values.size; ++i) {
                    if (i > 0) format::format_to(mb, ", ");
                    attr::serialize(g, e.attrs.values[i], mb);
                }
                format::format_to(mb, "]");
            }
            format::format_to(mb, ";");
        }

        status_t init(edge_t& e, u32 id, str::slice_t name, alloc_t* alloc) {
            auto r = string::interned::fold_for_result(name);
            e.id   = id;
            e.name = r.id;
            attr_set::init(e.attrs, component_type_t::edge, alloc);
            return status_t::ok;
        }
    }

    namespace graph {
        u0 free(graph_t& g) {
            for (auto& e : g.edges)
                edge::free(e);
            array::free(g.edges);
            for (auto& n : g.nodes)
                node::free(n);
            array::free(g.nodes);
            array::free(g.subgraphs);
            attr_set::free(g.attrs);
        }

        u0 sep(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::sep, v);
        }

        u0 pad(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::pad, v);
        }

        u0 pack(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::pack, v);
        }

        u0 size(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::size, v);
        }

        u0 page(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::page, v);
        }

        u0 pack(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::pack, v);
        }

        u0 sortv(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::sort_v, v);
        }

        u0 ratio(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::ratio, v);
        }

        u0 scale(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::scale, v);
        }

        u0 center(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::center, v);
        }

        u0 rotate(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::rotate, v);
        }

        u0 margin(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::margin, v);
        }

        u0 pad(graph_t& g, point_t v) {
            attr_set::set(g.attrs, attr_type_t::pad, v);
        }

        u0 new_rank(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::new_rank, v);
        }

        u0 compound(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::compound, v);
        }

        u0 color(graph_t& g, rgb_t v) {
            attr_set::set(g.attrs, attr_type_t::color, v);
        }

        u0 quantum(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::quantum, v);
        }

        u0 page(graph_t& g, point_t v) {
            attr_set::set(g.attrs, attr_type_t::page, v);
        }

        u0 node_sep(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::node_sep, v);
        }

        u0 color(graph_t& g, rgba_t v) {
            attr_set::set(g.attrs, attr_type_t::color, v);
        }

        u0 size(graph_t& g, point_t v) {
            attr_set::set(g.attrs, attr_type_t::size, v);
        }

        u0 landscape(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::landscape, v);
        }

        u0 rank_sep(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::rank_sep, v);
        }

        u0 mc_limit(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::mc_limit, v);
        }

        u0 ns_limit(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::ns_limit, v);
        }

        u0 scale(graph_t& g, point_t v) {
            attr_set::set(g.attrs, attr_type_t::scale, v);
        }

        u0 color(graph_t& g, color_t v) {
            attr_set::set(g.attrs, attr_type_t::color, v);
        }

        u0 no_justify(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::no_justify, v);
        }

        u0 normalize(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::normalize, v);
        }

        u0 ns_limit1(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::ns_limit1, v);
        }

        status_t init(graph_t& g,
                      graph_type_t type,
                      str::slice_t name,
                      graph_t* parent,
                      alloc_t* alloc) {
            auto r = string::interned::fold_for_result(name);
            if (!OK(r.status))
                return status_t::intern_failure;
            g.alloc  = alloc;
            g.type   = type;
            g.name   = r.id;
            g.parent = parent;
            array::init(g.edges, g.alloc);
            array::init(g.nodes, g.alloc);
            array::init(g.subgraphs, g.alloc);
            attr_set::init(g.attrs, component_type_t::graph, g.alloc);
            return status_t::ok;
        }

        u0 font_size(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::font_size, v);
        }

        u0 show_boxes(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::show_boxes, v);
        }

        u0 bg_color(graph_t& g, rgb_t v) {
            attr_set::set(g.attrs, attr_type_t::bg_color, v);
        }

        u0 margin(graph_t& g, point_t v) {
            attr_set::set(g.attrs, attr_type_t::margin, v);
        }

        u0 concentrate(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::concentrate, v);
        }

        u0 voro_margin(graph_t& g, f64 v) {
            attr_set::set(g.attrs, attr_type_t::voro_margin, v);
        }

        u0 bg_color(graph_t& g, rgba_t v) {
            attr_set::set(g.attrs, attr_type_t::bg_color, v);
        }

        u0 force_labels(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::force_labels, v);
        }

        u0 search_size(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::search_size, v);
        }

        u0 re_min_cross(graph_t& g, b8 v) {
            attr_set::set(g.attrs, attr_type_t::re_min_cross, v);
        }

        u0 rank(graph_t& g, rank_type_t v) {
            attr_set::set(g.attrs, attr_type_t::rank, u32(v));
        }

        u0 bg_color(graph_t& g, color_t v) {
            attr_set::set(g.attrs, attr_type_t::bg_color, v);
        }

        u0 overlap(graph_t& g, overlap_t v) {
            attr_set::set(g.attrs, attr_type_t::overlap, u32(v));
        }

        u0 charset(graph_t& g, charset_t v) {
            attr_set::set(g.attrs, attr_type_t::charset, u32(v));
        }

        u0 label(graph_t& g, str::slice_t v) {
            attr_set::set(g.attrs, attr_type_t::label, v);
        }

        edge_t* get_edge(graph_t& g, u32 id) {
            return id == 0 || id > g.edges.size ? nullptr : &g.edges[id - 1];
        }

        node_t* get_node(graph_t& g, u32 id) {
            return id == 0 || id > g.nodes.size ? nullptr : &g.nodes[id - 1];
        }

        u0 gradient_angle(graph_t& g, u32 v) {
            attr_set::set(g.attrs, attr_type_t::gradient_angle, v);
        }

        u0 style(graph_t& g, graph_style_t v) {
            attr_set::set(g.attrs, attr_type_t::style, u32(v));
        }

        u0 layers(graph_t& g, str::slice_t v) {
        }

        u0 rank_dir(graph_t& g, rank_dir_t v) {
            attr_set::set(g.attrs, attr_type_t::rank_dir, u32(v));
        }

        u0 page_dir(graph_t& g, page_dir_t v) {
            attr_set::set(g.attrs, attr_type_t::page_dir, u32(v));
        }

        u0 viewport(graph_t& g, viewport_t v) {
            attr_set::set(g.attrs, attr_type_t::viewport, v);
        }

        u0 ordering(graph_t& g, ordering_t v) {
            attr_set::set(g.attrs, attr_type_t::ordering, u32(v));
        }

        u0 comment(graph_t& g, str::slice_t v) {
            attr_set::set(g.attrs, attr_type_t::comment, v);
        }

        u0 splines(graph_t& g, spline_mode_t v) {
        }

        u0 font_name(graph_t& g, str::slice_t v) {
            attr_set::set(g.attrs, attr_type_t::font_name, v);
        }

        u0 pack_mode(graph_t& g, pack_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::pack_mode, u32(v));
        }

        u0 font_path(graph_t& g, const path_t& v) {
        }

        u0 background(graph_t& g, str::slice_t v) {
        }

        u0 layers_sep(graph_t& g, str::slice_t v) {
        }

        status_t serialize(graph_t& g, buf_t& buf) {
            mem_buf_t mb{&buf};
            auto graph_name = string::interned::get(g.name);
            if (!OK(graph_name.status))
                return status_t::intern_failure;
            format::format_to(mb,
                              g.type == graph_type_t::directed ? "digraph {} {{\n" : "graph {} {{\n",
                              graph_name.slice);

            for (const auto& attr : g.attrs.values) {
                format::format_to(mb, "\t");
                attr::serialize(g, attr, mb);
                format::format_to(mb, ";\n");
            }

            if (g.nodes.size > 0) {
                if (g.attrs.values.size > 0)
                    format::format_to(mb, "\n\n");
                for (u32 i = 0; i < g.nodes.size; ++i) {
                    if (i > 0) format::format_to(mb, "\n");
                    format::format_to(mb, "\t");
                    node::serialize(g, g.nodes[i], mb);
                }
            }

            if (g.edges.size > 0) {
                if (g.nodes.size > 0)
                    format::format_to(mb, "\n\n");
                for (u32 i = 0; i < g.edges.size; ++i) {
                    if (i > 0) format::format_to(mb, "\n");
                    format::format_to(mb, "\t");
                    edge::serialize(g, g.edges[i], mb);
                }
            }

            format::format_to(mb, "\n}}\n");
            return status_t::ok;
        }

        u0 image_path(graph_t& g, const path_t& v) {
        }

        u0 orientation(graph_t& g, orientation_t v) {
            attr_set::set(g.attrs, attr_type_t::orientation, u32(v));
        }

        u0 output_order(graph_t& g, output_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::output_order, u32(v));
        }

        u0 layers_select(graph_t& g, str::slice_t v) {
        }

        u0 color_scheme(graph_t& g, color_scheme_t v) {
            attr_set::set(g.attrs, attr_type_t::color_scheme, u32(v));
        }

        u0 label_loc(graph_t& g, graph_label_loc_t v) {
            attr_set::set(g.attrs, attr_type_t::label_loc, u32(v));
        }

        u0 layer_list_sep(graph_t& g, str::slice_t v) {
        }

        u0 cluster_rank(graph_t& g, cluster_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::cluster_rank, u32(v));
        }

        edge_t* make_edge(graph_t& g, str::slice_t name) {
            auto edge = &array::append(g.edges);
            edge::init(*edge, g.nodes.size, name, g.alloc);
            return edge;
        }

        node_t* make_node(graph_t& g, str::slice_t name) {
            auto node = &array::append(g.nodes);
            node::init(*node, g.nodes.size, name, g.alloc);
            return node;
        }

        u0 label_justification(graph_t& g, justification_t v) {
            attr_set::set(g.attrs, attr_type_t::label_just, u32(v));
        }
    }

    namespace attr_set {
        u0 free(attr_set_t& set) {
            array::free(set.values);
        }

        u0 set(attr_set_t& set, attr_type_t type, b8 flag) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.f    = flag;
            attr->value_type = u8(attr_value_type_t::boolean);
        }

        u0 set(attr_set_t& set, attr_type_t type, u32 value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.dw   = value;
            attr->value_type = u8(attr_value_type_t::integer);
        }

        u0 set(attr_set_t& set, attr_type_t type, f64 value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.fqw  = value;
            attr->value_type = u8(attr_value_type_t::floating_point);
        }

        attr_value_t* get(attr_set_t& set, attr_type_t type) {
            for (u32 i = 0; i < set.values.size; ++i) {
                if (set.values[i].type == u8(type))
                    return &set.values[i];
            }
            return nullptr;
        }

        u0 set(attr_set_t& set, attr_type_t type, hsv_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.hsv  = value;
            attr->value_type = u8(attr_value_type_t::hsv);
        }

        u0 set(attr_set_t& set, attr_type_t type, rgb_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.rgb  = value;
            attr->value_type = u8(attr_value_type_t::rgb);
        }

        u0 set(attr_set_t& set, attr_type_t type, rgba_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.rgba = value;
            attr->value_type = u8(attr_value_type_t::rgba);
        }

        u0 set(attr_set_t& set, attr_type_t type, rect_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.rect = value;
            attr->value_type = u8(attr_value_type_t::rect);
        }

        u0 set(attr_set_t& set, attr_type_t type, point_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type          = u8(type);
            attr->value.point   = value;
            attr->value_type    = u8(attr_value_type_t::point);
        }

        u0 set(attr_set_t& set, attr_type_t type, color_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type          = u8(type);
            attr->value.color   = value;
            attr->value_type    = u8(attr_value_type_t::color);
        }

        u0 set(attr_set_t& set, attr_type_t type, viewport_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type              = u8(type);
            attr->value.viewport    = value;
            attr->value_type        = u8(attr_value_type_t::viewport);
        }

        u0 set(attr_set_t& set, attr_type_t type, str::slice_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            auto r = string::interned::fold_for_result(value);
            attr = &array::append(set.values);
            attr->type       = u8(type);
            attr->value.dw   = r.id;
            attr->value_type = u8(attr_value_type_t::string);
        }

        status_t init(attr_set_t& set, component_type_t type, alloc_t* alloc) {
            set.alloc = alloc;
            set.type  = type;
            array::init(set.values, set.alloc);
            return status_t::ok;
        }
    }
}
