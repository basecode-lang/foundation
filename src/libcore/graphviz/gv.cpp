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
    static u0 escape_chars(str::slice_t value, str_t& buf) {
        str::reserve(buf, value.length * 2);
        str::reset(buf);
        for (u32 i = 0 ; i < value.length; i++) {
            const auto c = value[i];
            if (c == '\\') {
                ++i;
                if (value[i] == '|') {
                    str::append(buf, '|');
                } else if (value[i] == '{') {
                    str::append(buf, '{');
                } else if (value[i] == '}') {
                    str::append(buf, '}');
                }
            } else {
                if (c == '\"') {
                    str::append(buf, "\\\"");
                } else if (c == '{') {
                    str::append(buf, "\\{");
                } else if (c == '}') {
                    str::append(buf, "\\}");
                } else if (c == '.') {
                    str::append(buf, "\\.");
                } else if (c == ',') {
                    str::append(buf, "\\,");
                } else if (c == '|') {
                    str::append(buf, "\\|");
                } else if (c == '<') {
                    str::append(buf, "\\<");
                } else if (c == '>') {
                    str::append(buf, "\\>");
                } else if (c == '=') {
                    str::append(buf, "\\=");
                } else {
                    str::append(buf, c);
                }
            }
        }
    }

    namespace attr {
        static str::slice_t s_dir_names[] = {
            [u32(dir_type_t::none)]             = "none"_ss,
            [u32(dir_type_t::back)]             = "back"_ss,
            [u32(dir_type_t::both)]             = "both"_ss,
            [u32(dir_type_t::forward)]          = "forward"_ss,
        };

        static str::slice_t s_color_names[] = {
            [u32(color_t::aliceblue)]           = "aliceblue"_ss,
            [u32(color_t::antiquewhite)]        = "antiquewhite"_ss,
            [u32(color_t::antiquewhite1)]       = "antiquewhite1"_ss,
            [u32(color_t::antiquewhite2)]       = "antiquewhite2"_ss,
            [u32(color_t::antiquewhite3)]       = "antiquewhite3"_ss,
            [u32(color_t::antiquewhite4)]       = "antiquewhite4"_ss,
            [u32(color_t::aqua)]                = "aqua"_ss,
            [u32(color_t::aquamarine)]          = "aquamarine"_ss,
            [u32(color_t::aquamarine1)]         = "aquamarine1"_ss,
            [u32(color_t::aquamarine2)]         = "aquamarine2"_ss,
            [u32(color_t::aquamarine3)]         = "aquamarine3"_ss,
            [u32(color_t::aquamarine4)]         = "aquamarine4"_ss,
            [u32(color_t::azure)]               = "azure"_ss,
            [u32(color_t::azure1)]              = "azure1"_ss,
            [u32(color_t::azure2)]              = "azure2"_ss,
            [u32(color_t::azure3)]              = "azure3"_ss,
            [u32(color_t::azure4)]              = "azure4"_ss,
            [u32(color_t::beige)]               = "beige"_ss,
            [u32(color_t::bisque)]              = "bisque"_ss,
            [u32(color_t::bisque1)]             = "bisque1"_ss,
            [u32(color_t::bisque2)]             = "bisque2"_ss,
            [u32(color_t::bisque3)]             = "bisque3"_ss,
            [u32(color_t::bisque4)]             = "bisque4"_ss,
            [u32(color_t::black)]               = "black"_ss,
            [u32(color_t::blanchedalmond)]      = "blanchedalmond"_ss,
            [u32(color_t::blue)]                = "blue"_ss,
            [u32(color_t::blue1)]               = "blue1"_ss,
            [u32(color_t::blue2)]               = "blue2"_ss,
            [u32(color_t::blue3)]               = "blue3"_ss,
            [u32(color_t::blue4)]               = "blue4"_ss,
            [u32(color_t::blueviolet)]          = "blueviolet"_ss,
            [u32(color_t::brown)]               = "brown"_ss,
            [u32(color_t::brown1)]              = "brown1"_ss,
            [u32(color_t::brown2)]              = "brown2"_ss,
            [u32(color_t::brown3)]              = "brown3"_ss,
            [u32(color_t::brown4)]              = "brown4"_ss,
            [u32(color_t::burlywood)]           = "burlywood"_ss,
            [u32(color_t::burlywood1)]          = "burlywood1"_ss,
            [u32(color_t::burlywood2)]          = "burlywood2"_ss,
            [u32(color_t::burlywood3)]          = "burlywood3"_ss,
            [u32(color_t::burlywood4)]          = "burlywood4"_ss,
            [u32(color_t::cadetblue)]           = "cadetblue"_ss,
            [u32(color_t::cadetblue1)]          = "cadetblue1"_ss,
            [u32(color_t::cadetblue2)]          = "cadetblue2"_ss,
            [u32(color_t::cadetblue3)]          = "cadetblue3"_ss,
            [u32(color_t::cadetblue4)]          = "cadetblue4"_ss,
            [u32(color_t::chartreuse)]          = "chartreuse"_ss,
            [u32(color_t::chartreuse1)]         = "chartreuse1"_ss,
            [u32(color_t::chartreuse2)]         = "chartreuse2"_ss,
            [u32(color_t::chartreuse3)]         = "chartreuse3"_ss,
            [u32(color_t::chartreuse4)]         = "chartreuse4"_ss,
            [u32(color_t::chocolate)]           = "chocolate"_ss,
            [u32(color_t::chocolate1)]          = "chocolate1"_ss,
            [u32(color_t::chocolate2)]          = "chocolate2"_ss,
            [u32(color_t::chocolate3)]          = "chocolate3"_ss,
            [u32(color_t::chocolate4)]          = "chocolate4"_ss,
            [u32(color_t::coral)]               = "coral"_ss,
            [u32(color_t::coral1)]              = "coral1"_ss,
            [u32(color_t::coral2)]              = "coral2"_ss,
            [u32(color_t::coral3)]              = "coral3"_ss,
            [u32(color_t::coral4)]              = "coral4"_ss,
            [u32(color_t::cornflowerblue)]      = "cornflowerblue"_ss,
            [u32(color_t::cornsilk)]            = "cornsilk"_ss,
            [u32(color_t::cornsilk1)]           = "cornsilk1"_ss,
            [u32(color_t::cornsilk2)]           = "cornsilk2"_ss,
            [u32(color_t::cornsilk3)]           = "cornsilk3"_ss,
            [u32(color_t::cornsilk4)]           = "cornsilk4"_ss,
            [u32(color_t::crimson)]             = "crimson"_ss,
            [u32(color_t::cyan)]                = "cyan"_ss,
            [u32(color_t::cyan1)]               = "cyan1"_ss,
            [u32(color_t::cyan2)]               = "cyan2"_ss,
            [u32(color_t::cyan3)]               = "cyan3"_ss,
            [u32(color_t::cyan4)]               = "cyan4"_ss,
            [u32(color_t::darkblue)]            = "darkblue"_ss,
            [u32(color_t::darkcyan)]            = "darkcyan"_ss,
            [u32(color_t::darkgoldenrod)]       = "darkgoldenrod"_ss,
            [u32(color_t::darkgoldenrod1)]      = "darkgoldenrod1"_ss,
            [u32(color_t::darkgoldenrod2)]      = "darkgoldenrod2"_ss,
            [u32(color_t::darkgoldenrod3)]      = "darkgoldenrod3"_ss,
            [u32(color_t::darkgoldenrod4)]      = "darkgoldenrod4"_ss,
            [u32(color_t::darkgray)]            = "darkgray"_ss,
            [u32(color_t::darkgreen)]           = "darkgreen"_ss,
            [u32(color_t::darkgrey)]            = "darkgrey"_ss,
            [u32(color_t::darkkhaki)]           = "darkkhaki"_ss,
            [u32(color_t::darkmagenta)]         = "darkmagenta"_ss,
            [u32(color_t::darkolivegreen)]      = "darkolivegreen"_ss,
            [u32(color_t::darkolivegreen1)]     = "darkolivegreen1"_ss,
            [u32(color_t::darkolivegreen2)]     = "darkolivegreen2"_ss,
            [u32(color_t::darkolivegreen3)]     = "darkolivegreen3"_ss,
            [u32(color_t::darkolivegreen4)]     = "darkolivegreen4"_ss,
            [u32(color_t::darkorange)]          = "darkorange"_ss,
            [u32(color_t::darkorange1)]         = "darkorange1"_ss,
            [u32(color_t::darkorange2)]         = "darkorange2"_ss,
            [u32(color_t::darkorange3)]         = "darkorange3"_ss,
            [u32(color_t::darkorange4)]         = "darkorange4"_ss,
            [u32(color_t::darkorchid)]          = "darkorchid"_ss,
            [u32(color_t::darkorchid1)]         = "darkorchid1"_ss,
            [u32(color_t::darkorchid2)]         = "darkorchid2"_ss,
            [u32(color_t::darkorchid3)]         = "darkorchid3"_ss,
            [u32(color_t::darkorchid4)]         = "darkorchid4"_ss,
            [u32(color_t::darkred)]             = "darkred"_ss,
            [u32(color_t::darksalmon)]          = "darksalmon"_ss,
            [u32(color_t::darkseagreen)]        = "darkseagreen"_ss,
            [u32(color_t::darkseagreen1)]       = "darkseagreen1"_ss,
            [u32(color_t::darkseagreen2)]       = "darkseagreen2"_ss,
            [u32(color_t::darkseagreen3)]       = "darkseagreen3"_ss,
            [u32(color_t::darkseagreen4)]       = "darkseagreen4"_ss,
            [u32(color_t::darkslateblue)]       = "darkslateblue"_ss,
            [u32(color_t::darkslategray)]       = "darkslategray"_ss,
            [u32(color_t::darkslategray1)]      = "darkslategray1"_ss,
            [u32(color_t::darkslategray2)]      = "darkslategray2"_ss,
            [u32(color_t::darkslategray3)]      = "darkslategray3"_ss,
            [u32(color_t::darkslategray4)]      = "darkslategray4"_ss,
            [u32(color_t::darkslategrey)]       = "darkslategrey"_ss,
            [u32(color_t::darkturquoise)]       = "darkturquoise"_ss,
            [u32(color_t::darkviolet)]          = "darkviolet"_ss,
            [u32(color_t::deeppink)]            = "deeppink"_ss,
            [u32(color_t::deeppink1)]           = "deeppink1"_ss,
            [u32(color_t::deeppink2)]           = "deeppink2"_ss,
            [u32(color_t::deeppink3)]           = "deeppink3"_ss,
            [u32(color_t::deeppink4)]           = "deeppink4"_ss,
            [u32(color_t::deepskyblue)]         = "deepskyblue"_ss,
            [u32(color_t::deepskyblue1)]        = "deepskyblue1"_ss,
            [u32(color_t::deepskyblue2)]        = "deepskyblue2"_ss,
            [u32(color_t::deepskyblue3)]        = "deepskyblue3"_ss,
            [u32(color_t::deepskyblue4)]        = "deepskyblue4"_ss,
            [u32(color_t::dimgray)]             = "dimgray"_ss,
            [u32(color_t::dimgrey)]             = "dimgrey"_ss,
            [u32(color_t::dodgerblue)]          = "dodgerblue"_ss,
            [u32(color_t::dodgerblue1)]         = "dodgerblue1"_ss,
            [u32(color_t::dodgerblue2)]         = "dodgerblue2"_ss,
            [u32(color_t::dodgerblue3)]         = "dodgerblue3"_ss,
            [u32(color_t::dodgerblue4)]         = "dodgerblue4"_ss,
            [u32(color_t::firebrick)]           = "firebrick"_ss,
            [u32(color_t::firebrick1)]          = "firebrick1"_ss,
            [u32(color_t::firebrick2)]          = "firebrick2"_ss,
            [u32(color_t::firebrick3)]          = "firebrick3"_ss,
            [u32(color_t::firebrick4)]          = "firebrick4"_ss,
            [u32(color_t::floralwhite)]         = "floralwhite"_ss,
            [u32(color_t::forestgreen)]         = "forestgreen"_ss,
            [u32(color_t::fuchsia)]             = "fuchsia"_ss,
            [u32(color_t::gainsboro)]           = "gainsboro"_ss,
            [u32(color_t::ghostwhite)]          = "ghostwhite"_ss,
            [u32(color_t::gold)]                = "gold"_ss,
            [u32(color_t::gold1)]               = "gold1"_ss,
            [u32(color_t::gold2)]               = "gold2"_ss,
            [u32(color_t::gold3)]               = "gold3"_ss,
            [u32(color_t::gold4)]               = "gold4"_ss,
            [u32(color_t::goldenrod)]           = "goldenrod"_ss,
            [u32(color_t::goldenrod1)]          = "goldenrod1"_ss,
            [u32(color_t::goldenrod2)]          = "goldenrod2"_ss,
            [u32(color_t::goldenrod3)]          = "goldenrod3"_ss,
            [u32(color_t::goldenrod4)]          = "goldenrod4"_ss,
            [u32(color_t::gray)]                = "gray"_ss,
            [u32(color_t::gray0)]               = "gray0"_ss,
            [u32(color_t::gray1)]               = "gray1"_ss,
            [u32(color_t::gray10)]              = "gray10"_ss,
            [u32(color_t::gray100)]             = "gray100"_ss,
            [u32(color_t::gray11)]              = "gray11"_ss,
            [u32(color_t::gray12)]              = "gray12"_ss,
            [u32(color_t::gray13)]              = "gray13"_ss,
            [u32(color_t::gray14)]              = "gray14"_ss,
            [u32(color_t::gray15)]              = "gray15"_ss,
            [u32(color_t::gray16)]              = "gray16"_ss,
            [u32(color_t::gray17)]              = "gray17"_ss,
            [u32(color_t::gray18)]              = "gray18"_ss,
            [u32(color_t::gray19)]              = "gray19"_ss,
            [u32(color_t::gray2)]               = "gray2"_ss,
            [u32(color_t::gray20)]              = "gray20"_ss,
            [u32(color_t::gray21)]              = "gray21"_ss,
            [u32(color_t::gray22)]              = "gray22"_ss,
            [u32(color_t::gray23)]              = "gray23"_ss,
            [u32(color_t::gray24)]              = "gray24"_ss,
            [u32(color_t::gray25)]              = "gray25"_ss,
            [u32(color_t::gray26)]              = "gray26"_ss,
            [u32(color_t::gray27)]              = "gray27"_ss,
            [u32(color_t::gray28)]              = "gray28"_ss,
            [u32(color_t::gray29)]              = "gray29"_ss,
            [u32(color_t::gray3)]               = "gray3"_ss,
            [u32(color_t::gray30)]              = "gray30"_ss,
            [u32(color_t::gray31)]              = "gray31"_ss,
            [u32(color_t::gray32)]              = "gray32"_ss,
            [u32(color_t::gray33)]              = "gray33"_ss,
            [u32(color_t::gray34)]              = "gray34"_ss,
            [u32(color_t::gray35)]              = "gray35"_ss,
            [u32(color_t::gray36)]              = "gray36"_ss,
            [u32(color_t::gray37)]              = "gray37"_ss,
            [u32(color_t::gray38)]              = "gray38"_ss,
            [u32(color_t::gray39)]              = "gray39"_ss,
            [u32(color_t::gray4)]               = "gray4"_ss,
            [u32(color_t::gray40)]              = "gray40"_ss,
            [u32(color_t::gray41)]              = "gray41"_ss,
            [u32(color_t::gray42)]              = "gray42"_ss,
            [u32(color_t::gray43)]              = "gray43"_ss,
            [u32(color_t::gray44)]              = "gray44"_ss,
            [u32(color_t::gray45)]              = "gray45"_ss,
            [u32(color_t::gray46)]              = "gray46"_ss,
            [u32(color_t::gray47)]              = "gray47"_ss,
            [u32(color_t::gray48)]              = "gray48"_ss,
            [u32(color_t::gray49)]              = "gray49"_ss,
            [u32(color_t::gray5)]               = "gray5"_ss,
            [u32(color_t::gray50)]              = "gray50"_ss,
            [u32(color_t::gray51)]              = "gray51"_ss,
            [u32(color_t::gray52)]              = "gray52"_ss,
            [u32(color_t::gray53)]              = "gray53"_ss,
            [u32(color_t::gray54)]              = "gray54"_ss,
            [u32(color_t::gray55)]              = "gray55"_ss,
            [u32(color_t::gray56)]              = "gray56"_ss,
            [u32(color_t::gray57)]              = "gray57"_ss,
            [u32(color_t::gray58)]              = "gray58"_ss,
            [u32(color_t::gray59)]              = "gray59"_ss,
            [u32(color_t::gray6)]               = "gray6"_ss,
            [u32(color_t::gray60)]              = "gray60"_ss,
            [u32(color_t::gray61)]              = "gray61"_ss,
            [u32(color_t::gray62)]              = "gray62"_ss,
            [u32(color_t::gray63)]              = "gray63"_ss,
            [u32(color_t::gray64)]              = "gray64"_ss,
            [u32(color_t::gray65)]              = "gray65"_ss,
            [u32(color_t::gray66)]              = "gray66"_ss,
            [u32(color_t::gray67)]              = "gray67"_ss,
            [u32(color_t::gray68)]              = "gray68"_ss,
            [u32(color_t::gray69)]              = "gray69"_ss,
            [u32(color_t::gray7)]               = "gray7"_ss,
            [u32(color_t::gray70)]              = "gray70"_ss,
            [u32(color_t::gray71)]              = "gray71"_ss,
            [u32(color_t::gray72)]              = "gray72"_ss,
            [u32(color_t::gray73)]              = "gray73"_ss,
            [u32(color_t::gray74)]              = "gray74"_ss,
            [u32(color_t::gray75)]              = "gray75"_ss,
            [u32(color_t::gray76)]              = "gray76"_ss,
            [u32(color_t::gray77)]              = "gray77"_ss,
            [u32(color_t::gray78)]              = "gray78"_ss,
            [u32(color_t::gray79)]              = "gray79"_ss,
            [u32(color_t::gray8)]               = "gray8"_ss,
            [u32(color_t::gray80)]              = "gray80"_ss,
            [u32(color_t::gray81)]              = "gray81"_ss,
            [u32(color_t::gray82)]              = "gray82"_ss,
            [u32(color_t::gray83)]              = "gray83"_ss,
            [u32(color_t::gray84)]              = "gray84"_ss,
            [u32(color_t::gray85)]              = "gray85"_ss,
            [u32(color_t::gray86)]              = "gray86"_ss,
            [u32(color_t::gray87)]              = "gray87"_ss,
            [u32(color_t::gray88)]              = "gray88"_ss,
            [u32(color_t::gray89)]              = "gray89"_ss,
            [u32(color_t::gray9)]               = "gray9"_ss,
            [u32(color_t::gray90)]              = "gray90"_ss,
            [u32(color_t::gray91)]              = "gray91"_ss,
            [u32(color_t::gray92)]              = "gray92"_ss,
            [u32(color_t::gray93)]              = "gray93"_ss,
            [u32(color_t::gray94)]              = "gray94"_ss,
            [u32(color_t::gray95)]              = "gray95"_ss,
            [u32(color_t::gray96)]              = "gray96"_ss,
            [u32(color_t::gray97)]              = "gray97"_ss,
            [u32(color_t::gray98)]              = "gray98"_ss,
            [u32(color_t::gray99)]              = "gray99"_ss,
            [u32(color_t::green)]               = "green"_ss,
            [u32(color_t::green1)]              = "green1"_ss,
            [u32(color_t::green2)]              = "green2"_ss,
            [u32(color_t::green3)]              = "green3"_ss,
            [u32(color_t::green4)]              = "green4"_ss,
            [u32(color_t::greenyellow)]         = "greenyellow"_ss,
            [u32(color_t::grey)]                = "grey"_ss,
            [u32(color_t::grey0)]               = "grey0"_ss,
            [u32(color_t::grey1)]               = "grey1"_ss,
            [u32(color_t::grey10)]              = "grey10"_ss,
            [u32(color_t::grey100)]             = "grey100"_ss,
            [u32(color_t::grey11)]              = "grey11"_ss,
            [u32(color_t::grey12)]              = "grey12"_ss,
            [u32(color_t::grey13)]              = "grey13"_ss,
            [u32(color_t::grey14)]              = "grey14"_ss,
            [u32(color_t::grey15)]              = "grey15"_ss,
            [u32(color_t::grey16)]              = "grey16"_ss,
            [u32(color_t::grey17)]              = "grey17"_ss,
            [u32(color_t::grey18)]              = "grey18"_ss,
            [u32(color_t::grey19)]              = "grey19"_ss,
            [u32(color_t::grey2)]               = "grey2"_ss,
            [u32(color_t::grey20)]              = "grey20"_ss,
            [u32(color_t::grey21)]              = "grey21"_ss,
            [u32(color_t::grey22)]              = "grey22"_ss,
            [u32(color_t::grey23)]              = "grey23"_ss,
            [u32(color_t::grey24)]              = "grey24"_ss,
            [u32(color_t::grey25)]              = "grey25"_ss,
            [u32(color_t::grey26)]              = "grey26"_ss,
            [u32(color_t::grey27)]              = "grey27"_ss,
            [u32(color_t::grey28)]              = "grey28"_ss,
            [u32(color_t::grey29)]              = "grey29"_ss,
            [u32(color_t::grey3)]               = "grey3"_ss,
            [u32(color_t::grey30)]              = "grey30"_ss,
            [u32(color_t::grey31)]              = "grey31"_ss,
            [u32(color_t::grey32)]              = "grey32"_ss,
            [u32(color_t::grey33)]              = "grey33"_ss,
            [u32(color_t::grey34)]              = "grey34"_ss,
            [u32(color_t::grey35)]              = "grey35"_ss,
            [u32(color_t::grey36)]              = "grey36"_ss,
            [u32(color_t::grey37)]              = "grey37"_ss,
            [u32(color_t::grey38)]              = "grey38"_ss,
            [u32(color_t::grey39)]              = "grey39"_ss,
            [u32(color_t::grey4)]               = "grey4"_ss,
            [u32(color_t::grey40)]              = "grey40"_ss,
            [u32(color_t::grey41)]              = "grey41"_ss,
            [u32(color_t::grey42)]              = "grey42"_ss,
            [u32(color_t::grey43)]              = "grey43"_ss,
            [u32(color_t::grey44)]              = "grey44"_ss,
            [u32(color_t::grey45)]              = "grey45"_ss,
            [u32(color_t::grey46)]              = "grey46"_ss,
            [u32(color_t::grey47)]              = "grey47"_ss,
            [u32(color_t::grey48)]              = "grey48"_ss,
            [u32(color_t::grey49)]              = "grey49"_ss,
            [u32(color_t::grey5)]               = "grey5"_ss,
            [u32(color_t::grey50)]              = "grey50"_ss,
            [u32(color_t::grey51)]              = "grey51"_ss,
            [u32(color_t::grey52)]              = "grey52"_ss,
            [u32(color_t::grey53)]              = "grey53"_ss,
            [u32(color_t::grey54)]              = "grey54"_ss,
            [u32(color_t::grey55)]              = "grey55"_ss,
            [u32(color_t::grey56)]              = "grey56"_ss,
            [u32(color_t::grey57)]              = "grey57"_ss,
            [u32(color_t::grey58)]              = "grey58"_ss,
            [u32(color_t::grey59)]              = "grey59"_ss,
            [u32(color_t::grey6)]               = "grey6"_ss,
            [u32(color_t::grey60)]              = "grey60"_ss,
            [u32(color_t::grey61)]              = "grey61"_ss,
            [u32(color_t::grey62)]              = "grey62"_ss,
            [u32(color_t::grey63)]              = "grey63"_ss,
            [u32(color_t::grey64)]              = "grey64"_ss,
            [u32(color_t::grey65)]              = "grey65"_ss,
            [u32(color_t::grey66)]              = "grey66"_ss,
            [u32(color_t::grey67)]              = "grey67"_ss,
            [u32(color_t::grey68)]              = "grey68"_ss,
            [u32(color_t::grey69)]              = "grey69"_ss,
            [u32(color_t::grey7)]               = "grey7"_ss,
            [u32(color_t::grey70)]              = "grey70"_ss,
            [u32(color_t::grey71)]              = "grey71"_ss,
            [u32(color_t::grey72)]              = "grey72"_ss,
            [u32(color_t::grey73)]              = "grey73"_ss,
            [u32(color_t::grey74)]              = "grey74"_ss,
            [u32(color_t::grey75)]              = "grey75"_ss,
            [u32(color_t::grey76)]              = "grey76"_ss,
            [u32(color_t::grey77)]              = "grey77"_ss,
            [u32(color_t::grey78)]              = "grey78"_ss,
            [u32(color_t::grey79)]              = "grey79"_ss,
            [u32(color_t::grey8)]               = "grey8"_ss,
            [u32(color_t::grey80)]              = "grey80"_ss,
            [u32(color_t::grey81)]              = "grey81"_ss,
            [u32(color_t::grey82)]              = "grey82"_ss,
            [u32(color_t::grey83)]              = "grey83"_ss,
            [u32(color_t::grey84)]              = "grey84"_ss,
            [u32(color_t::grey85)]              = "grey85"_ss,
            [u32(color_t::grey86)]              = "grey86"_ss,
            [u32(color_t::grey87)]              = "grey87"_ss,
            [u32(color_t::grey88)]              = "grey88"_ss,
            [u32(color_t::grey89)]              = "grey89"_ss,
            [u32(color_t::grey9)]               = "grey9"_ss,
            [u32(color_t::grey90)]              = "grey90"_ss,
            [u32(color_t::grey91)]              = "grey91"_ss,
            [u32(color_t::grey92)]              = "grey92"_ss,
            [u32(color_t::grey93)]              = "grey93"_ss,
            [u32(color_t::grey94)]              = "grey94"_ss,
            [u32(color_t::grey95)]              = "grey95"_ss,
            [u32(color_t::grey96)]              = "grey96"_ss,
            [u32(color_t::grey97)]              = "grey97"_ss,
            [u32(color_t::grey98)]              = "grey98"_ss,
            [u32(color_t::grey99)]              = "grey99"_ss,
            [u32(color_t::honeydew)]            = "honeydew"_ss,
            [u32(color_t::honeydew1)]           = "honeydew1"_ss,
            [u32(color_t::honeydew2)]           = "honeydew2"_ss,
            [u32(color_t::honeydew3)]           = "honeydew3"_ss,
            [u32(color_t::honeydew4)]           = "honeydew4"_ss,
            [u32(color_t::hotpink)]             = "hotpink"_ss,
            [u32(color_t::hotpink1)]            = "hotpink1"_ss,
            [u32(color_t::hotpink2)]            = "hotpink2"_ss,
            [u32(color_t::hotpink3)]            = "hotpink3"_ss,
            [u32(color_t::hotpink4)]            = "hotpink4"_ss,
            [u32(color_t::indianred)]           = "indianred"_ss,
            [u32(color_t::indianred1)]          = "indianred1"_ss,
            [u32(color_t::indianred2)]          = "indianred2"_ss,
            [u32(color_t::indianred3)]          = "indianred3"_ss,
            [u32(color_t::indianred4)]          = "indianred4"_ss,
            [u32(color_t::indigo)]              = "indigo"_ss,
            [u32(color_t::invis)]               = "invis"_ss,
            [u32(color_t::ivory)]               = "ivory"_ss,
            [u32(color_t::ivory1)]              = "ivory1"_ss,
            [u32(color_t::ivory2)]              = "ivory2"_ss,
            [u32(color_t::ivory3)]              = "ivory3"_ss,
            [u32(color_t::ivory4)]              = "ivory4"_ss,
            [u32(color_t::khaki)]               = "khaki"_ss,
            [u32(color_t::khaki1)]              = "khaki1"_ss,
            [u32(color_t::khaki2)]              = "khaki2"_ss,
            [u32(color_t::khaki3)]              = "khaki3"_ss,
            [u32(color_t::khaki4)]              = "khaki4"_ss,
            [u32(color_t::lavender)]            = "lavender"_ss,
            [u32(color_t::lavenderblush)]       = "lavenderblush"_ss,
            [u32(color_t::lavenderblush1)]      = "lavenderblush1"_ss,
            [u32(color_t::lavenderblush2)]      = "lavenderblush2"_ss,
            [u32(color_t::lavenderblush3)]      = "lavenderblush3"_ss,
            [u32(color_t::lavenderblush4)]      = "lavenderblush4"_ss,
            [u32(color_t::lawngreen)]           = "lawngreen"_ss,
            [u32(color_t::lemonchiffon)]        = "lemonchiffon"_ss,
            [u32(color_t::lemonchiffon1)]       = "lemonchiffon1"_ss,
            [u32(color_t::lemonchiffon2)]       = "lemonchiffon2"_ss,
            [u32(color_t::lemonchiffon3)]       = "lemonchiffon3"_ss,
            [u32(color_t::lemonchiffon4)]       = "lemonchiffon4"_ss,
            [u32(color_t::lightblue)]           = "lightblue"_ss,
            [u32(color_t::lightblue1)]          = "lightblue1"_ss,
            [u32(color_t::lightblue2)]          = "lightblue2"_ss,
            [u32(color_t::lightblue3)]          = "lightblue3"_ss,
            [u32(color_t::lightblue4)]          = "lightblue4"_ss,
            [u32(color_t::lightcoral)]          = "lightcoral"_ss,
            [u32(color_t::lightcyan)]           = "lightcyan"_ss,
            [u32(color_t::lightcyan1)]          = "lightcyan1"_ss,
            [u32(color_t::lightcyan2)]          = "lightcyan2"_ss,
            [u32(color_t::lightcyan3)]          = "lightcyan3"_ss,
            [u32(color_t::lightcyan4)]          = "lightcyan4"_ss,
            [u32(color_t::lightgoldenrod)]      = "lightgoldenrod"_ss,
            [u32(color_t::lightgoldenrod1)]     = "lightgoldenrod1"_ss,
            [u32(color_t::lightgoldenrod2)]     = "lightgoldenrod2"_ss,
            [u32(color_t::lightgoldenrod3)]     = "lightgoldenrod3"_ss,
            [u32(color_t::lightgoldenrod4)]     = "lightgoldenrod4"_ss,
            [u32(color_t::lightgoldenrodyellow)]= "lightgoldenrodyellow"_ss,
            [u32(color_t::lightgray)]           = "lightgray"_ss,
            [u32(color_t::lightgreen)]          = "lightgreen"_ss,
            [u32(color_t::lightgrey)]           = "lightgrey"_ss,
            [u32(color_t::lightpink)]           = "lightpink"_ss,
            [u32(color_t::lightpink1)]          = "lightpink1"_ss,
            [u32(color_t::lightpink2)]          = "lightpink2"_ss,
            [u32(color_t::lightpink3)]          = "lightpink3"_ss,
            [u32(color_t::lightpink4)]          = "lightpink4"_ss,
            [u32(color_t::lightsalmon)]         = "lightsalmon"_ss,
            [u32(color_t::lightsalmon1)]        = "lightsalmon1"_ss,
            [u32(color_t::lightsalmon2)]        = "lightsalmon2"_ss,
            [u32(color_t::lightsalmon3)]        = "lightsalmon3"_ss,
            [u32(color_t::lightsalmon4)]        = "lightsalmon4"_ss,
            [u32(color_t::lightseagreen)]       = "lightseagreen"_ss,
            [u32(color_t::lightskyblue)]        = "lightskyblue"_ss,
            [u32(color_t::lightskyblue1)]       = "lightskyblue1"_ss,
            [u32(color_t::lightskyblue2)]       = "lightskyblue2"_ss,
            [u32(color_t::lightskyblue3)]       = "lightskyblue3"_ss,
            [u32(color_t::lightskyblue4)]       = "lightskyblue4"_ss,
            [u32(color_t::lightslateblue)]      = "lightslateblue"_ss,
            [u32(color_t::lightslategray)]      = "lightslategray"_ss,
            [u32(color_t::lightslategrey)]      = "lightslategrey"_ss,
            [u32(color_t::lightsteelblue)]      = "lightsteelblue"_ss,
            [u32(color_t::lightsteelblue1)]     = "lightsteelblue1"_ss,
            [u32(color_t::lightsteelblue2)]     = "lightsteelblue2"_ss,
            [u32(color_t::lightsteelblue3)]     = "lightsteelblue3"_ss,
            [u32(color_t::lightsteelblue4)]     = "lightsteelblue4"_ss,
            [u32(color_t::lightyellow)]         = "lightyellow"_ss,
            [u32(color_t::lightyellow1)]        = "lightyellow1"_ss,
            [u32(color_t::lightyellow2)]        = "lightyellow2"_ss,
            [u32(color_t::lightyellow3)]        = "lightyellow3"_ss,
            [u32(color_t::lightyellow4)]        = "lightyellow4"_ss,
            [u32(color_t::lime)]                = "lime"_ss,
            [u32(color_t::limegreen)]           = "limegreen"_ss,
            [u32(color_t::linen)]               = "linen"_ss,
            [u32(color_t::magenta)]             = "magenta"_ss,
            [u32(color_t::magenta1)]            = "magenta1"_ss,
            [u32(color_t::magenta2)]            = "magenta2"_ss,
            [u32(color_t::magenta3)]            = "magenta3"_ss,
            [u32(color_t::magenta4)]            = "magenta4"_ss,
            [u32(color_t::maroon)]              = "maroon"_ss,
            [u32(color_t::maroon1)]             = "maroon1"_ss,
            [u32(color_t::maroon2)]             = "maroon2"_ss,
            [u32(color_t::maroon3)]             = "maroon3"_ss,
            [u32(color_t::maroon4)]             = "maroon4"_ss,
            [u32(color_t::mediumaquamarine)]    = "mediumaquamarine"_ss,
            [u32(color_t::mediumblue)]          = "mediumblue"_ss,
            [u32(color_t::mediumorchid)]        = "mediumorchid"_ss,
            [u32(color_t::mediumorchid1)]       = "mediumorchid1"_ss,
            [u32(color_t::mediumorchid2)]       = "mediumorchid2"_ss,
            [u32(color_t::mediumorchid3)]       = "mediumorchid3"_ss,
            [u32(color_t::mediumorchid4)]       = "mediumorchid4"_ss,
            [u32(color_t::mediumpurple)]        = "mediumpurple"_ss,
            [u32(color_t::mediumpurple1)]       = "mediumpurple1"_ss,
            [u32(color_t::mediumpurple2)]       = "mediumpurple2"_ss,
            [u32(color_t::mediumpurple3)]       = "mediumpurple3"_ss,
            [u32(color_t::mediumpurple4)]       = "mediumpurple4"_ss,
            [u32(color_t::mediumseagreen)]      = "mediumseagreen"_ss,
            [u32(color_t::mediumslateblue)]     = "mediumslateblue"_ss,
            [u32(color_t::mediumspringgreen)]   = "mediumspringgreen"_ss,
            [u32(color_t::mediumturquoise)]     = "mediumturquoise"_ss,
            [u32(color_t::mediumvioletred)]     = "mediumvioletred"_ss,
            [u32(color_t::midnightblue)]        = "midnightblue"_ss,
            [u32(color_t::mintcream)]           = "mintcream"_ss,
            [u32(color_t::mistyrose)]           = "mistyrose"_ss,
            [u32(color_t::mistyrose1)]          = "mistyrose1"_ss,
            [u32(color_t::mistyrose2)]          = "mistyrose2"_ss,
            [u32(color_t::mistyrose3)]          = "mistyrose3"_ss,
            [u32(color_t::mistyrose4)]          = "mistyrose4"_ss,
            [u32(color_t::moccasin)]            = "moccasin"_ss,
            [u32(color_t::navajowhite)]         = "navajowhite"_ss,
            [u32(color_t::navajowhite1)]        = "navajowhite1"_ss,
            [u32(color_t::navajowhite2)]        = "navajowhite2"_ss,
            [u32(color_t::navajowhite3)]        = "navajowhite3"_ss,
            [u32(color_t::navajowhite4)]        = "navajowhite4"_ss,
            [u32(color_t::navy)]                = "navy"_ss,
            [u32(color_t::navyblue)]            = "navyblue"_ss,
            [u32(color_t::none)]                = "none"_ss,
            [u32(color_t::oldlace)]             = "oldlace"_ss,
            [u32(color_t::olive)]               = "olive"_ss,
            [u32(color_t::olivedrab)]           = "olivedrab"_ss,
            [u32(color_t::olivedrab1)]          = "olivedrab1"_ss,
            [u32(color_t::olivedrab2)]          = "olivedrab2"_ss,
            [u32(color_t::olivedrab3)]          = "olivedrab3"_ss,
            [u32(color_t::olivedrab4)]          = "olivedrab4"_ss,
            [u32(color_t::orange)]              = "orange"_ss,
            [u32(color_t::orange1)]             = "orange1"_ss,
            [u32(color_t::orange2)]             = "orange2"_ss,
            [u32(color_t::orange3)]             = "orange3"_ss,
            [u32(color_t::orange4)]             = "orange4"_ss,
            [u32(color_t::orangered)]           = "orangered"_ss,
            [u32(color_t::orangered1)]          = "orangered1"_ss,
            [u32(color_t::orangered2)]          = "orangered2"_ss,
            [u32(color_t::orangered3)]          = "orangered3"_ss,
            [u32(color_t::orangered4)]          = "orangered4"_ss,
            [u32(color_t::orchid)]              = "orchid"_ss,
            [u32(color_t::orchid1)]             = "orchid1"_ss,
            [u32(color_t::orchid2)]             = "orchid2"_ss,
            [u32(color_t::orchid3)]             = "orchid3"_ss,
            [u32(color_t::orchid4)]             = "orchid4"_ss,
            [u32(color_t::palegoldenrod)]       = "palegoldenrod"_ss,
            [u32(color_t::palegreen)]           = "palegreen"_ss,
            [u32(color_t::palegreen1)]          = "palegreen1"_ss,
            [u32(color_t::palegreen2)]          = "palegreen2"_ss,
            [u32(color_t::palegreen3)]          = "palegreen3"_ss,
            [u32(color_t::palegreen4)]          = "palegreen4"_ss,
            [u32(color_t::paleturquoise)]       = "paleturquoise"_ss,
            [u32(color_t::paleturquoise1)]      = "paleturquoise1"_ss,
            [u32(color_t::paleturquoise2)]      = "paleturquoise2"_ss,
            [u32(color_t::paleturquoise3)]      = "paleturquoise3"_ss,
            [u32(color_t::paleturquoise4)]      = "paleturquoise4"_ss,
            [u32(color_t::palevioletred)]       = "palevioletred"_ss,
            [u32(color_t::palevioletred1)]      = "palevioletred1"_ss,
            [u32(color_t::palevioletred2)]      = "palevioletred2"_ss,
            [u32(color_t::palevioletred3)]      = "palevioletred3"_ss,
            [u32(color_t::palevioletred4)]      = "palevioletred4"_ss,
            [u32(color_t::papayawhip)]          = "papayawhip"_ss,
            [u32(color_t::peachpuff)]           = "peachpuff"_ss,
            [u32(color_t::peachpuff1)]          = "peachpuff1"_ss,
            [u32(color_t::peachpuff2)]          = "peachpuff2"_ss,
            [u32(color_t::peachpuff3)]          = "peachpuff3"_ss,
            [u32(color_t::peachpuff4)]          = "peachpuff4"_ss,
            [u32(color_t::peru)]                = "peru,"_ss,
            [u32(color_t::pink)]                = "pink"_ss,
            [u32(color_t::pink1)]               = "pink1"_ss,
            [u32(color_t::pink2)]               = "pink2"_ss,
            [u32(color_t::pink3)]               = "pink3"_ss,
            [u32(color_t::pink4)]               = "pink4"_ss,
            [u32(color_t::plum)]                = "plum"_ss,
            [u32(color_t::plum1)]               = "plum1"_ss,
            [u32(color_t::plum2)]               = "plum2"_ss,
            [u32(color_t::plum3)]               = "plum3"_ss,
            [u32(color_t::plum4)]               = "plum4"_ss,
            [u32(color_t::powderblue)]          = "powderblue"_ss,
            [u32(color_t::purple)]              = "purple"_ss,
            [u32(color_t::purple1)]             = "purple1"_ss,
            [u32(color_t::purple2)]             = "purple2"_ss,
            [u32(color_t::purple3)]             = "purple3"_ss,
            [u32(color_t::purple4)]             = "purple4"_ss,
            [u32(color_t::rebeccapurple)]       = "rebeccapurple"_ss,
            [u32(color_t::red)]                 = "red"_ss,
            [u32(color_t::red1)]                = "red1"_ss,
            [u32(color_t::red2)]                = "red2"_ss,
            [u32(color_t::red3)]                = "red3"_ss,
            [u32(color_t::red4)]                = "red4"_ss,
            [u32(color_t::rosybrown)]           = "rosybrown"_ss,
            [u32(color_t::rosybrown1)]          = "rosybrown1"_ss,
            [u32(color_t::rosybrown2)]          = "rosybrown2"_ss,
            [u32(color_t::rosybrown3)]          = "rosybrown3"_ss,
            [u32(color_t::rosybrown4)]          = "rosybrown4"_ss,
            [u32(color_t::royalblue)]           = "royalblue"_ss,
            [u32(color_t::royalblue1)]          = "royalblue1"_ss,
            [u32(color_t::royalblue2)]          = "royalblue2"_ss,
            [u32(color_t::royalblue3)]          = "royalblue3"_ss,
            [u32(color_t::royalblue4)]          = "royalblue4"_ss,
            [u32(color_t::saddlebrown)]         = "saddlebrown"_ss,
            [u32(color_t::salmon)]              = "salmon"_ss,
            [u32(color_t::salmon1)]             = "salmon1"_ss,
            [u32(color_t::salmon2)]             = "salmon2"_ss,
            [u32(color_t::salmon3)]             = "salmon3"_ss,
            [u32(color_t::salmon4)]             = "salmon4"_ss,
            [u32(color_t::sandybrown)]          = "sandybrown"_ss,
            [u32(color_t::seagreen)]            = "seagreen"_ss,
            [u32(color_t::seagreen1)]           = "seagreen1"_ss,
            [u32(color_t::seagreen2)]           = "seagreen2"_ss,
            [u32(color_t::seagreen3)]           = "seagreen3"_ss,
            [u32(color_t::seagreen4)]           = "seagreen4"_ss,
            [u32(color_t::seashell)]            = "seashell"_ss,
            [u32(color_t::seashell1)]           = "seashell1"_ss,
            [u32(color_t::seashell2)]           = "seashell2"_ss,
            [u32(color_t::seashell3)]           = "seashell3"_ss,
            [u32(color_t::seashell4)]           = "seashell4"_ss,
            [u32(color_t::sienna)]              = "sienna"_ss,
            [u32(color_t::sienna1)]             = "sienna1"_ss,
            [u32(color_t::sienna2)]             = "sienna2"_ss,
            [u32(color_t::sienna3)]             = "sienna3"_ss,
            [u32(color_t::sienna4)]             = "sienna4"_ss,
            [u32(color_t::silver)]              = "silver"_ss,
            [u32(color_t::skyblue)]             = "skyblue"_ss,
            [u32(color_t::skyblue1)]            = "skyblue1"_ss,
            [u32(color_t::skyblue2)]            = "skyblue2"_ss,
            [u32(color_t::skyblue3)]            = "skyblue3"_ss,
            [u32(color_t::skyblue4)]            = "skyblue4"_ss,
            [u32(color_t::slateblue)]           = "slateblue"_ss,
            [u32(color_t::slateblue1)]          = "slateblue1"_ss,
            [u32(color_t::slateblue2)]          = "slateblue2"_ss,
            [u32(color_t::slateblue3)]          = "slateblue3"_ss,
            [u32(color_t::slateblue4)]          = "slateblue4"_ss,
            [u32(color_t::slategray)]           = "slategray"_ss,
            [u32(color_t::slategray1)]          = "slategray1"_ss,
            [u32(color_t::slategray2)]          = "slategray2"_ss,
            [u32(color_t::slategray3)]          = "slategray3"_ss,
            [u32(color_t::slategray4)]          = "slategray4"_ss,
            [u32(color_t::slategrey)]           = "slategrey"_ss,
            [u32(color_t::snow)]                = "snow"_ss,
            [u32(color_t::snow1)]               = "snow1"_ss,
            [u32(color_t::snow2)]               = "snow2"_ss,
            [u32(color_t::snow3)]               = "snow3"_ss,
            [u32(color_t::snow4)]               = "snow4"_ss,
            [u32(color_t::springgreen)]         = "springgreen"_ss,
            [u32(color_t::springgreen1)]        = "springgreen1"_ss,
            [u32(color_t::springgreen2)]        = "springgreen2"_ss,
            [u32(color_t::springgreen3)]        = "springgreen3"_ss,
            [u32(color_t::springgreen4)]        = "springgreen4"_ss,
            [u32(color_t::steelblue)]           = "steelblue"_ss,
            [u32(color_t::steelblue1)]          = "steelblue1"_ss,
            [u32(color_t::steelblue2)]          = "steelblue2"_ss,
            [u32(color_t::steelblue3)]          = "steelblue3"_ss,
            [u32(color_t::steelblue4)]          = "steelblue4"_ss,
            [u32(color_t::tan)]                 = "tan"_ss,
            [u32(color_t::tan1)]                = "tan1"_ss,
            [u32(color_t::tan2)]                = "tan2"_ss,
            [u32(color_t::tan3)]                = "tan3"_ss,
            [u32(color_t::tan4)]                = "tan4"_ss,
            [u32(color_t::teal)]                = "teal"_ss,
            [u32(color_t::thistle)]             = "thistle"_ss,
            [u32(color_t::thistle1)]            = "thistle1"_ss,
            [u32(color_t::thistle2)]            = "thistle2"_ss,
            [u32(color_t::thistle3)]            = "thistle3"_ss,
            [u32(color_t::thistle4)]            = "thistle4"_ss,
            [u32(color_t::tomato)]              = "tomato"_ss,
            [u32(color_t::tomato1)]             = "tomato1"_ss,
            [u32(color_t::tomato2)]             = "tomato2"_ss,
            [u32(color_t::tomato3)]             = "tomato3"_ss,
            [u32(color_t::tomato4)]             = "tomato4"_ss,
            [u32(color_t::transparent)]         = "transparent"_ss,
            [u32(color_t::turquoise)]           = "turquoise"_ss,
            [u32(color_t::turquoise1)]          = "turquoise1"_ss,
            [u32(color_t::turquoise2)]          = "turquoise2"_ss,
            [u32(color_t::turquoise3)]          = "turquoise3"_ss,
            [u32(color_t::turquoise4)]          = "turquoise4"_ss,
            [u32(color_t::violet)]              = "violet"_ss,
            [u32(color_t::violetred)]           = "violetred"_ss,
            [u32(color_t::violetred1)]          = "violetred1"_ss,
            [u32(color_t::violetred2)]          = "violetred2"_ss,
            [u32(color_t::violetred3)]          = "violetred3"_ss,
            [u32(color_t::violetred4)]          = "violetred4"_ss,
            [u32(color_t::webgray)]             = "webgray"_ss,
            [u32(color_t::webgreen)]            = "webgreen"_ss,
            [u32(color_t::webgrey)]             = "webgrey"_ss,
            [u32(color_t::webmaroon)]           = "webmaroon"_ss,
            [u32(color_t::webpurple)]           = "webpurple"_ss,
            [u32(color_t::wheat)]               = "wheat"_ss,
            [u32(color_t::wheat1)]              = "wheat1"_ss,
            [u32(color_t::wheat2)]              = "wheat2"_ss,
            [u32(color_t::wheat3)]              = "wheat3"_ss,
            [u32(color_t::wheat4)]              = "wheat4"_ss,
            [u32(color_t::white)]               = "white"_ss,
            [u32(color_t::whitesmoke)]          = "whitesmoke"_ss,
            [u32(color_t::x11gray)]             = "x11gray"_ss,
            [u32(color_t::x11green)]            = "x11green"_ss,
            [u32(color_t::x11grey)]             = "x11grey"_ss,
            [u32(color_t::x11maroon)]           = "x11maroon"_ss,
            [u32(color_t::x11purple)]           = "x11purple"_ss,
            [u32(color_t::yellow)]              = "yellow"_ss,
            [u32(color_t::yellow1)]             = "yellow1"_ss,
            [u32(color_t::yellow2)]             = "yellow2"_ss,
            [u32(color_t::yellow3)]             = "yellow3"_ss,
            [u32(color_t::yellow4)]             = "yellow4"_ss,
            [u32(color_t::yellowgreen)]         = "yellowgreen"_ss,
        };

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

        static str::slice_t s_shape_names[] = {
            [u32(shape_t::none)]                = "none"_ss,
            [u32(shape_t::box)]                 = "box"_ss,
            [u32(shape_t::polygon)]             = "polygon"_ss,
            [u32(shape_t::ellipse)]             = "ellipse"_ss,
            [u32(shape_t::oval)]                = "oval"_ss,
            [u32(shape_t::circle)]              = "circle"_ss,
            [u32(shape_t::point)]               = "point"_ss,
            [u32(shape_t::egg)]                 = "egg"_ss,
            [u32(shape_t::triangle)]            = "triangle"_ss,
            [u32(shape_t::plaintext)]           = "plaintext"_ss,
            [u32(shape_t::plain)]               = "plain"_ss,
            [u32(shape_t::diamond)]             = "diamond"_ss,
            [u32(shape_t::trapezium)]           = "trapezium"_ss,
            [u32(shape_t::parallelogram)]       = "parallelogram"_ss,
            [u32(shape_t::house)]               = "house"_ss,
            [u32(shape_t::pentagon)]            = "pentagon"_ss,
            [u32(shape_t::hexagon)]             = "hexagon"_ss,
            [u32(shape_t::septagon)]            = "septagon"_ss,
            [u32(shape_t::octagon)]             = "octagon"_ss,
            [u32(shape_t::doublecircle)]        = "doublecircle"_ss,
            [u32(shape_t::doubleoctagon)]       = "doubleoctagon"_ss,
            [u32(shape_t::tripleoctagon)]       = "tripleoctagon"_ss,
            [u32(shape_t::invtriangle)]         = "invtriangle"_ss,
            [u32(shape_t::invtrapezium)]        = "invtrapezium"_ss,
            [u32(shape_t::invhouse)]            = "invhouse"_ss,
            [u32(shape_t::mdiamond)]            = "mdiamond"_ss,
            [u32(shape_t::msquare)]             = "msquare"_ss,
            [u32(shape_t::mcircle)]             = "mcircle"_ss,
            [u32(shape_t::mrecord)]             = "mrecord"_ss,
            [u32(shape_t::record)]              = "record"_ss,
            [u32(shape_t::rect)]                = "rect"_ss,
            [u32(shape_t::rectangle)]           = "rectangle"_ss,
            [u32(shape_t::square)]              = "square"_ss,
            [u32(shape_t::star)]                = "star"_ss,
            [u32(shape_t::underline)]           = "underline"_ss,
            [u32(shape_t::box3d)]               = "box3d"_ss,
            [u32(shape_t::component)]           = "component"_ss,
            [u32(shape_t::promoter)]            = "promoter"_ss,
            [u32(shape_t::cds)]                 = "cds"_ss,
            [u32(shape_t::terminator)]          = "terminator"_ss,
            [u32(shape_t::utr)]                 = "utr"_ss,
            [u32(shape_t::primersite)]          = "primersite"_ss,
            [u32(shape_t::restrictionsite)]     = "restrictionsite"_ss,
            [u32(shape_t::fiveproverhang)]      = "fiveproverhang"_ss,
            [u32(shape_t::threepoverhange)]     = "threepoverhange"_ss,
            [u32(shape_t::noverhang)]           = "noverhang"_ss,
            [u32(shape_t::assembly)]            = "assembly"_ss,
            [u32(shape_t::signature)]           = "signature"_ss,
            [u32(shape_t::insulator)]           = "insulator"_ss,
            [u32(shape_t::ribosite)]            = "ribosite"_ss,
            [u32(shape_t::rnastab)]             = "rnastab"_ss,
            [u32(shape_t::proteasesite)]        = "proteasesite"_ss,
            [u32(shape_t::proteinstab)]         = "proteinstab"_ss,
            [u32(shape_t::rpromoter)]           = "rpromoter"_ss,
            [u32(shape_t::rarrow)]              = "rarrow"_ss,
            [u32(shape_t::larrow)]              = "larrow"_ss,
            [u32(shape_t::lpromoter)]           = "lpromoter"_ss,
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

        static str::slice_t s_image_pos_names[] = {
            [u32(image_pos_t::tl)]              = "tl"_ss,
            [u32(image_pos_t::tc)]              = "tc"_ss,
            [u32(image_pos_t::tr)]              = "tr"_ss,
            [u32(image_pos_t::ml)]              = "ml"_ss,
            [u32(image_pos_t::mc)]              = "mc"_ss,
            [u32(image_pos_t::mr)]              = "mr"_ss,
            [u32(image_pos_t::bl)]              = "bl"_ss,
            [u32(image_pos_t::bc)]              = "bc"_ss,
            [u32(image_pos_t::br)]              = "br"_ss,
        };

        static str::slice_t s_page_dir_names[] = {
            [u32(page_dir_t::bl)]               = "bl"_ss,
            [u32(page_dir_t::br)]               = "br"_ss,
            [u32(page_dir_t::tl)]               = "tl"_ss,
            [u32(page_dir_t::tr)]               = "tr"_ss,
            [u32(page_dir_t::rb)]               = "rb"_ss,
            [u32(page_dir_t::rt)]               = "rt"_ss,
            [u32(page_dir_t::lb)]               = "lb"_ss,
            [u32(page_dir_t::lt)]               = "lt"_ss
        };

        static str::slice_t s_label_loc_names[] = {
            [u32(node_label_loc_t::top)]        = "top"_ss,
            [u32(node_label_loc_t::bottom)]     = "bottom"_ss,
            [u32(node_label_loc_t::center)]     = "center"_ss,
        };

        static str::slice_t s_pack_mode_names[] = {
            [u32(pack_mode_t::node)]            = "node"_ss,
            [u32(pack_mode_t::cluster)]         = "cluster"_ss,
            [u32(pack_mode_t::graph)]           = "graph"_ss,
            [u32(pack_mode_t::array)]           = "array"_ss,
        };

        static str::slice_t s_rank_type_names[] = {
            [u32(rank_type_t::same)]            = "same"_ss,
            [u32(rank_type_t::min)]             = "min"_ss,
            [u32(rank_type_t::source)]          = "source"_ss,
            [u32(rank_type_t::max)]             = "max"_ss,
            [u32(rank_type_t::sink)]            = "sink"_ss
        };

        static str::slice_t s_edge_style_names[] = {
            [u32(edge_style_t::dashed)]         = "dashed"_ss,
            [u32(edge_style_t::dotted)]         = "dotted"_ss,
            [u32(edge_style_t::solid)]          = "solid"_ss,
            [u32(edge_style_t::invis)]          = "invis"_ss,
            [u32(edge_style_t::bold)]           = "bold"_ss,
            [u32(edge_style_t::tapered)]        = "tapered"_ss
        };

        static str::slice_t s_node_style_names[] = {
            [u32(node_style_t::dashed)]         = "dashed"_ss,
            [u32(node_style_t::dotted)]         = "dotted"_ss,
            [u32(node_style_t::solid)]          = "solid"_ss,
            [u32(node_style_t::invis)]          = "invis"_ss,
            [u32(node_style_t::bold)]           = "bold"_ss,
            [u32(node_style_t::filled)]         = "filled"_ss,
            [u32(node_style_t::striped)]        = "striped"_ss,
            [u32(node_style_t::wedged)]         = "wedged"_ss,
            [u32(node_style_t::diagonals)]      = "diagonals"_ss,
            [u32(node_style_t::rounded)]        = "rounded"_ss,
        };

        static str::slice_t s_spline_mode_names[] = {
            [u32(spline_mode_t::none)]          = "none"_ss,
            [u32(spline_mode_t::spline)]        = "spline"_ss,
            [u32(spline_mode_t::line)]          = "line"_ss,
            [u32(spline_mode_t::polyline)]      = "polyline"_ss,
            [u32(spline_mode_t::ortho)]         = "ortho"_ss,
            [u32(spline_mode_t::curved)]        = "curved"_ss,
        };

        static str::slice_t s_output_mode_names[] = {
            [u32(output_mode_t::breadth_first)] = "breadthfirst"_ss,
            [u32(output_mode_t::nodes_first )]  = "nodesfirst"_ss,
            [u32(output_mode_t::edges_first )]  = "edgesfirst"_ss,
        };

        static str::slice_t s_color_scheme_names[] = {
            [u32(color_scheme_t::x11)]          = "X11"_ss,
            [u32(color_scheme_t::svg)]          = "SVG"_ss,
        };

        static str::slice_t s_overlap_names[] = {
            [u32(overlap_t::retain)]            = "retain"_ss,
            [u32(overlap_t::scale)]             = "scale"_ss,
            [u32(overlap_t::prism)]             = "prism"_ss,
            [u32(overlap_t::voronoi)]           = "voronoi"_ss,
            [u32(overlap_t::scalexy)]           = "scalexy"_ss,
            [u32(overlap_t::compress)]          = "compress"_ss,
            [u32(overlap_t::vpsc)]              = "vpsc"_ss,
            [u32(overlap_t::ortho)]             = "ortho"_ss,
            [u32(overlap_t::orthoxy)]           = "orthoxy"_ss,
            [u32(overlap_t::orthoyx)]           = "orthoyx"_ss,
            [u32(overlap_t::portho)]            = "portho"_ss,
            [u32(overlap_t::porthoxy)]          = "porthoxy"_ss,
            [u32(overlap_t::porthoyx)]          = "porthoyx"_ss,
        };

        static str::slice_t s_compass_point_names[] = {
            [u32(compass_point_t::n)]           = "n"_ss,
            [u32(compass_point_t::ne)]          = "ne"_ss,
            [u32(compass_point_t::e)]           = "e"_ss,
            [u32(compass_point_t::se)]          = "se"_ss,
            [u32(compass_point_t::s)]           = "s"_ss,
            [u32(compass_point_t::sw)]          = "sw"_ss,
            [u32(compass_point_t::w)]           = "w"_ss,
            [u32(compass_point_t::nw)]          = "nw"_ss,
            [u32(compass_point_t::c)]           = "c"_ss,
            [u32(compass_point_t::_)]           = "_"_ss
        };

        u0 serialize(graph_t& g,
                     const attr_value_t& attr,
                     mem_buf_t& mb,
                     const node_t* node) {
            format::format_to(mb, "{}=", type_name(attr.type));
            switch (attr_type_t(attr.type)) {
                // enumeration
                case attr_type_t::dir: {
                    const auto type = dir_type_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      dir_name(type));
                    break;
                }
                case attr_type_t::rank: {
                    format::format_to(mb,
                                      "{}",
                                      rank_type_name(rank_type_t(attr.value.dw)));
                    break;
                }
                case attr_type_t::style: {
                    switch (attr.value_type) {
                        case attr_value_type_t::edge_style:
                            format::format_to(mb,
                                              "{}",
                                              edge_style_name(edge_style_t(attr.value.dw)));
                            break;
                        case attr_value_type_t::node_style:
                            format::format_to(mb,
                                              "{}",
                                              node_style_name(node_style_t(attr.value.dw)));
                            break;
                        case attr_value_type_t::graph_style:
                            format::format_to(mb,
                                              "{}",
                                              graph_style_name(graph_style_t(attr.value.dw)));
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case attr_type_t::shape: {
                    const auto type = shape_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      shape_name(type));
                    break;
                }
                case attr_type_t::overlap: {
                    format::format_to(mb,
                                      "{}",
                                      overlap_name(overlap_t(attr.value.dw)));
                    break;
                }
                case attr_type_t::charset: {
                    const auto type = charset_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      charset_name(type));
                    break;
                }
                case attr_type_t::rank_dir: {
                    const auto type = rank_dir_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      rank_dir_name(type));
                    break;
                }
                case attr_type_t::page_dir: {
                    const auto type = page_dir_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      page_dir_name(type));
                    break;
                }
                case attr_type_t::ordering: {
                    const auto type = ordering_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      ordering_name(type));
                    break;
                }
                case attr_type_t::image_pos: {
                    const auto type = image_pos_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      image_pos_name(type));
                    break;
                }
                case attr_type_t::label_loc: {
                    format::format_to(mb,
                                      "{}",
                                      label_loc_name(node_label_loc_t(attr.value.dw)));
                    break;
                }
                case attr_type_t::pack_mode: {
                    format::format_to(mb,
                                      "{}",
                                      pack_mode_name(pack_mode_t(attr.value.dw)));
                    break;
                }
                case attr_type_t::arrow_head:
                case attr_type_t::arrow_tail: {
                    const auto type = arrow_type_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      arrow_type_name(type));
                    break;
                }
                case attr_type_t::color_scheme: {
                    const auto type = color_scheme_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      color_scheme_name(type));
                    break;
                }
                case attr_type_t::cluster_rank: {
                    format::format_to(mb,
                                      "{}",
                                      cluster_mode_name(cluster_mode_t(attr.value.dw)));
                    break;
                }
                case attr_type_t::output_order: {
                    const auto type = output_mode_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      output_mode_name(type));
                    break;
                }
                case attr_type_t::label: {
                    auto r = string::interned::get(attr.value.dw);
                    if (OK(r.status)) {
                        if (node && node->fields.size > 0) {
                            format::format_to(mb, "\"");
                            for (u32 i = 0; i < node->fields.size; ++i) {
                                const auto& field = node->fields[i];
                                if (i > 0)
                                    format::format_to(mb, "|");
                                format::format_to(mb,
                                                  "<f{}>",
                                                  field.id);
                                if (field.label) {
                                    r = string::interned::get(field.label);
                                    if (OK(r.status)) {
                                        escape_chars(r.slice, g.scratch);
                                        format::format_to(mb,
                                                          "{}",
                                                          g.scratch);
                                    }
                                }
                            }
                            format::format_to(mb, "\"");
                        } else {
                            escape_chars(r.slice, g.scratch);
                            format::format_to(mb,
                                              "\"{}\"",
                                              g.scratch);
                        }
                    }
                    break;
                }
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
                case attr_type_t::label_font_name: {
                    auto r = string::interned::get(attr.value.dw);
                    if (OK(r.status)) {
                        escape_chars(r.slice, g.scratch);
                        format::format_to(mb,
                                          "\"{}\"",
                                          g.scratch);
                    }
                    break;
                }

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
                case attr_type_t::label_font_size: {
                    switch (attr.value_type) {
                        case attr_value_type_t::point: {
                            const auto& p = attr.value.point;
                            format::format_to(mb,
                                              "{},{}",
                                              p.x,
                                              p.y);
                            break;
                        }
                        case attr_value_type_t::integer: {
                            format::format_to(mb,
                                              "{}",
                                              attr.value.dw);
                            break;
                        }
                        case attr_value_type_t::floating_point: {
                            format::format_to(mb,
                                              "{}",
                                              attr.value.fqw);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }

                // colors
                case attr_type_t::color:
                case attr_type_t::bg_color:
                case attr_type_t::pen_color:
                case attr_type_t::fill_color:
                case attr_type_t::font_color:
                case attr_type_t::label_font_color: {
                    switch (attr.value_type) {
                        case attr_value_type_t::hsv: {
                            const auto& hsv = attr.value.hsv;
                            format::format_to(mb,
                                              "{},{},{}",
                                              hsv.h,
                                              hsv.s,
                                              hsv.v);
                            break;
                        }
                        case attr_value_type_t::rgb: {
                            const auto& rgb = attr.value.rgb;
                            format::format_to(mb,
                                              "#{:02x}{:02x}{:02x}",
                                              rgb.r,
                                              rgb.g,
                                              rgb.b);
                            break;
                        }
                        case attr_value_type_t::rgba: {
                            const auto& rgba = attr.value.rgba;
                            format::format_to(mb,
                                              "#{:02x}{:02x}{:02x}{:02x}",
                                              rgba.r,
                                              rgba.g,
                                              rgba.b,
                                              rgba.a);
                            break;
                        }
                        case attr_value_type_t::color:
                            format::format_to(mb,
                                              "{}",
                                              attr::color_name(color_t(attr.value.dw)));
                            break;
                        default:
                            break;
                    }
                    break;
                }

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
                case attr_type_t::re_min_cross: {
                    format::format_to(mb,
                                      "{}",
                                      attr.value.f ? "yes"_ss : "no"_ss);
                    break;
                }

                // custom
                case attr_type_t::pos: {
                    format::format_to(mb,
                                      "{},{}",
                                      attr.value.point.x,
                                      attr.value.point.y);
                    break;
                }
                case attr_type_t::splines: {
                    const auto type = spline_mode_t(attr.value.dw);
                    format::format_to(mb,
                                      "{}",
                                      spline_mode_name(type));
                    break;
                }
                case attr_type_t::viewport: {
                    const auto& vp = attr.value.viewport;
                    format::format_to(mb,
                                      "{},{},{},{},{}",
                                      vp.w,
                                      vp.h,
                                      vp.z,
                                      vp.x,
                                      vp.y);
                    break;
                }
                case attr_type_t::head_port:
                case attr_type_t::tail_port: {
                    break;
                }

                // not supported
                case attr_type_t::z:
                case attr_type_t::layer:
                case attr_type_t::layout:
                case attr_type_t::layers:
                case attr_type_t::layer_sep:
                case attr_type_t::shape_file:
                case attr_type_t::background:
                case attr_type_t::layer_select:
                case attr_type_t::sample_points:
                case attr_type_t::layer_list_sep: {
                    break;
                }
            }
        }

        str::slice_t dir_name(dir_type_t dir) {
            return s_dir_names[u32(dir)];
        }

        str::slice_t color_name(color_t color) {
            return s_color_names[u32(color)];
        }

        str::slice_t shape_name(shape_t shape) {
            return s_shape_names[u32(shape)];
        }

        str::slice_t charset_name(charset_t cs) {
            switch (cs) {
                case charset_t::utf8:           return "UTF-8"_ss;
                case charset_t::iso_8859_1:     return "iso-8859-1"_ss;
                default:                        return "Latin1"_ss;
            }
        }

        str::slice_t overlap_name(overlap_t ol) {
            return s_overlap_names[u32(ol)];
        }

        str::slice_t type_name(attr_type_t type) {
            return s_type_names[u32(type)];
        }

        str::slice_t rank_dir_name(rank_dir_t dir) {
            switch (dir) {
                case rank_dir_t::tb: return "tb"_ss;
                case rank_dir_t::lr: return "lr"_ss;
                case rank_dir_t::bt: return "bt"_ss;
                case rank_dir_t::rl: return "rl"_ss;
            }
        }

        str::slice_t page_dir_name(page_dir_t dir) {
            return s_page_dir_names[u32(dir)];
        }

        str::slice_t image_pos_name(image_pos_t pos) {
            return s_image_pos_names[u32(pos)];
        }

        str::slice_t ordering_name(ordering_t order) {
            switch (order) {
                case ordering_t::none:  return "none"_ss;
                case ordering_t::out:   return "out"_ss;
                case ordering_t::in:    return "in"_ss;
            }
        }

        str::slice_t pack_mode_name(pack_mode_t mode) {
            return s_pack_mode_names[u32(mode)];
        }

        str::slice_t rank_type_name(rank_type_t type) {
            return s_rank_type_names[u32(type)];
        }

        str::slice_t arrow_type_name(arrow_type_t type) {
            return s_arrow_type_names[u32(type)];
        }

        str::slice_t edge_style_name(edge_style_t style) {
            return s_edge_style_names[u32(style)];
        }

        str::slice_t node_style_name(node_style_t style) {
            return s_node_style_names[u32(style)];
        }

        str::slice_t spline_mode_name(spline_mode_t mode) {
            return s_spline_mode_names[u32(mode)];
        }

        str::slice_t output_mode_name(output_mode_t mode) {
            return s_output_mode_names[u32(mode)];
        }

        str::slice_t label_loc_name(node_label_loc_t loc) {
            return s_label_loc_names[u32(loc)];
        }

        str::slice_t graph_style_name(graph_style_t style) {
            if (style == graph_style_t::radial) {
                return "radial"_ss;
            } else {
                return "none"_ss;
            }
        }

        str::slice_t label_loc_name(graph_label_loc_t loc) {
            return s_label_loc_names[u32(loc)];
        }

        str::slice_t cluster_mode_name(cluster_mode_t mode) {
            switch (mode) {
                case cluster_mode_t::none:      return "none"_ss;
                case cluster_mode_t::local:     return "local"_ss;
                case cluster_mode_t::global:    return "global"_ss;
            }
        }

        str::slice_t color_scheme_name(color_scheme_t scheme) {
            return s_color_scheme_names[u32(scheme)];
        }

        str::slice_t compass_point_name(compass_point_t point) {
            return s_compass_point_names[u32(point)];
        }
    }

    namespace node {
        u0 free(node_t& n) {
            array::free(n.fields);
            attr_set::free(n.attrs);
        }

        u32 make_field(node_t& n) {
            auto& field = array::append(n.fields);
            field.id = n.fields.size;
            field.label = {};
            attr_set::set(n.attrs,
                          attr_type_t::label,
                          str::slice_t{});
            return field.id;
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
            attr_set::set(n.attrs,
                          attr_type_t::style,
                          u32(v),
                          attr_value_type_t::node_style);
        }

        u0 fill_color(node_t& n, color_t v) {
            attr_set::set(n.attrs, attr_type_t::fill_color, v);
        }

        u0 font_color(node_t& n, color_t v) {
            attr_set::set(n.attrs, attr_type_t::font_color, v);
        }

        u0 gradient_angle(node_t& n, u32 v) {
            attr_set::set(n.attrs, attr_type_t::gradient_angle, v);
        }

        // XXX:
        u0 image(node_t& n, const path_t& v) {
            attr_set::set(n.attrs, attr_type_t::image, path::c_str(v));
        }

        u0 ordering(node_t& n, ordering_t v) {
            attr_set::set(n.attrs, attr_type_t::ordering, u32(v));
        }

        u0 image_pos(node_t& n, image_pos_t v) {
            attr_set::set(n.attrs, attr_type_t::image_pos, u32(v));
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
            if (!OK(node_name.status)) {
                format::format_to(mb,
                                  "// node has invalid name: {}",
                                  n.id);
                return;
            }
            format::format_to(mb, "{}", node_name.slice);
            if (n.attrs.values.size > 0) {
                format::format_to(mb, "[");
                for (u32 i = 0; i < n.attrs.values.size; ++i) {
                    if (i > 0) format::format_to(mb, ", ");
                    attr::serialize(g, n.attrs.values[i], mb, &n);
                }
                format::format_to(mb, "]");
            }
            format::format_to(mb, ";");
        }

        u0 set_field_label(node_t& n, u32 id, str::slice_t label) {
            if (id == 0 || id > n.fields.size)
                return;
            auto& field = n.fields[id - 1];
            auto rc = string::interned::fold_for_result(label);
            field.label = rc.id;
        }

        status_t init(node_t& n, u32 id, str::slice_t name, alloc_t* alloc) {
            auto r = string::interned::fold_for_result(name);
            n.id   = id;
            n.name = r.id;
            array::init(n.fields, alloc);
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

        u0 label_distance(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::label_distance, v);
        }

        u0 style(edge_t& e, edge_style_t v) {
            attr_set::set(e.attrs,
                          attr_type_t::style,
                          u32(v),
                          attr_value_type_t::edge_style);
        }

        u0 label_font_size(edge_t& e, f64 v) {
            attr_set::set(e.attrs, attr_type_t::label_font_size, v);
        }

        u0 label_font_color(edge_t& e, rgb_t v) {
            attr_set::set(e.attrs, attr_type_t::label_font_color, v);
        }

        u0 arrow_head(edge_t& e, arrow_type_t v) {
            attr_set::set(e.attrs, attr_type_t::arrow_head, u32(v));
        }

        u0 arrow_tail(edge_t& e, arrow_type_t v) {
            attr_set::set(e.attrs, attr_type_t::arrow_tail, u32(v));
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

        status_t init(edge_t& e, u32 id, alloc_t* alloc) {
            e.id   = id;
            attr_set::init(e.attrs, component_type_t::edge, alloc);
            return status_t::ok;
        }

        u0 serialize(graph_t& g, const edge_t& e, mem_buf_t& mb) {
            const auto node_connector = g.type == graph_type_t::directed ?
                                        "->"_ss : "--"_ss;
            auto lhs = graph::get_node(*e.first.graph, e.first.id);
            auto rhs = graph::get_node(*e.second.graph, e.second.id);
            if (!lhs || !rhs) {
                format::format_to(mb, "// empty edge: {}", e.id);
                return;
            }

            auto lhs_name = string::interned::get(lhs->name);
            format::format_to(mb, "\"{}\"", lhs_name.slice);
            if (e.first.field.id) {
                format::format_to(mb, ":f{}", e.first.field.id);
                if (e.first.field.point != compass_point_t::_) {
                    format::format_to(mb,
                                      ":{}",
                                      attr::compass_point_name(e.first.field.point));
                }
            }
            format::format_to(mb, " {} ", node_connector);
            auto rhs_name = string::interned::get(rhs->name);
            format::format_to(mb, "\"{}\"", rhs_name.slice);
            if (e.second.field.id) {
                format::format_to(mb, ":f{}", e.second.field.id);
                if (e.second.field.point != compass_point_t::_) {
                    format::format_to(mb,
                                      ":{}",
                                      attr::compass_point_name(e.second.field.point));
                }
            }

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
            str::free(g.scratch);
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

        edge_t* make_edge(graph_t& g) {
            auto edge = &array::append(g.edges);
            edge::init(*edge, g.edges.size, g.alloc);
            return edge;
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
                      alloc_t* alloc) {
            auto r = string::interned::fold_for_result(name);
            if (!OK(r.status))
                return status_t::intern_failure;
            g.type   = type;
            g.name   = r.id;
            g.alloc  = alloc;
            g.parent = {};
            str::init(g.scratch, g.alloc);
            str::reserve(g.scratch, 64);
            array::init(g.edges, g.alloc);
            array::init(g.nodes, g.alloc);
            array::init(g.subgraphs, g.alloc);
            attr_set::init(g.attrs, component_type_t::graph, g.alloc);
            return status_t::ok;
        }

        node_t* make_node(graph_t& g) {
            auto node = &array::append(g.nodes);
            {
                str::reset(g.scratch);
                str_buf_t buf(&g.scratch);
                format::format_to(buf, "node_{}_{}", g.name, g.nodes.size);
            }
            node::init(*node, g.nodes.size, slice::make(g.scratch), g.alloc);
            return node;
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
            attr_set::set(g.attrs,
                          attr_type_t::style,
                          u32(v),
                          attr_value_type_t::graph_style);
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

        u0 splines(graph_t& g, spline_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::splines, u32(v));
        }

        u0 pack_mode(graph_t& g, pack_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::pack_mode, u32(v));
        }

        u0 font_path(graph_t& g, const path_t& v) {
            UNUSED(g);
            UNUSED(v);
        }

        status_t serialize(graph_t& g, mem_buf_t& mb, u32 indent) {
            auto graph_name = string::interned::get(g.name);
            if (!OK(graph_name.status))
                return status_t::intern_failure;

            format::format_to(mb, "{:<{}}", " ", indent);
            if (g.subgraph) {
                auto leader = g.cluster ?
                              "subgraph cluster_{} {{\n" :
                              "subgraph {} {{\n";
                format::format_to(mb, leader, graph_name.slice);
            } else {
                auto leader = g.type == graph_type_t::directed ?
                              "digraph \"{}\" {{\n" :
                              "graph \"{}\" {{\n";
                format::format_to(mb, leader, graph_name.slice);
            }

            for (u32 i = 0; i < g.attrs.values.size; ++i) {
                if (i > 0) format::format_to(mb, "\n");
                format::format_to(mb, "{:<{}}", " ", indent + 3);
                attr::serialize(g, g.attrs.values[i], mb);
                format::format_to(mb, ";");
            }

            if ((g.nodes.size > 0 && g.attrs.values.size > 0)
            ||  g.subgraphs.size > 0) {
                format::format_to(mb, "\n\n");
            }

            for (auto sg : g.subgraphs) {
                auto status = serialize(*sg, mb, indent + 3);
                if (!OK(status))
                    return status;
            }

            if (g.nodes.size > 0 && g.attrs.values.size > 0)
                format::format_to(mb, "\n\n");

            for (u32 i = 0; i < g.nodes.size; ++i) {
                if (i > 0) format::format_to(mb, "\n");
                format::format_to(mb, "{:<{}}", " ", indent + 3);
                node::serialize(g, g.nodes[i], mb);
            }

            if (g.edges.size > 0 && g.nodes.size > 0)
                format::format_to(mb, "\n\n");

            for (u32 i = 0; i < g.edges.size; ++i) {
                if (i > 0) format::format_to(mb, "\n");
                format::format_to(mb, "{:<{}}", " ", indent + 3);
                edge::serialize(g, g.edges[i], mb);
            }

            format::format_to(mb, "\n{:<{}}}}\n", " ", indent);
            return status_t::ok;
        }

        status_t serialize(graph_t& g, buf_t& buf) {
            mem_buf_t mb{&buf};
            return serialize(g, mb, 0);
        }

        u0 image_path(graph_t& g, const path_t& v) {
            UNUSED(g);
            UNUSED(v);
        }

        u0 orientation(graph_t& g, orientation_t v) {
            attr_set::set(g.attrs, attr_type_t::orientation, u32(v));
        }

        u0 output_order(graph_t& g, output_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::output_order, u32(v));
        }

        u0 color_scheme(graph_t& g, color_scheme_t v) {
            attr_set::set(g.attrs, attr_type_t::color_scheme, u32(v));
        }

        u0 label_loc(graph_t& g, graph_label_loc_t v) {
            attr_set::set(g.attrs, attr_type_t::label_loc, u32(v));
        }

        u0 cluster_rank(graph_t& g, cluster_mode_t v) {
            attr_set::set(g.attrs, attr_type_t::cluster_rank, u32(v));
        }

        u0 add_subgraph(graph_t& g, graph_t* subgraph) {
            subgraph->parent   = &g;
            subgraph->cluster  = false;
            subgraph->subgraph = true;
            array::append(g.subgraphs, subgraph);
        }

        u0 label_justification(graph_t& g, justification_t v) {
            attr_set::set(g.attrs, attr_type_t::label_just, u32(v));
        }

        u0 add_cluster_subgraph(graph_t& g, graph_t* subgraph) {
            subgraph->parent   = &g;
            subgraph->cluster  = true;
            subgraph->subgraph = true;
            array::append(g.subgraphs, subgraph);
        }
    }

    namespace attr_set {
        u0 free(attr_set_t& set) {
            array::free(set.values);
        }

        u0 set(attr_set_t& set,
               attr_type_t type,
               u32 value,
               attr_value_type_t value_type) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.dw   = value;
            attr->value_type = value_type;
        }

        u0 set(attr_set_t& set, attr_type_t type, b8 flag) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.f    = flag;
            attr->value_type = attr_value_type_t::boolean;
        }

        u0 set(attr_set_t& set, attr_type_t type, f64 value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.fqw  = value;
            attr->value_type = attr_value_type_t::floating_point;
        }

        attr_value_t* get(attr_set_t& set, attr_type_t type) {
            for (u32 i = 0; i < set.values.size; ++i) {
                if (set.values[i].type == type)
                    return &set.values[i];
            }
            return nullptr;
        }

        u0 set(attr_set_t& set, attr_type_t type, hsv_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.hsv  = value;
            attr->value_type = attr_value_type_t::hsv;
        }

        u0 set(attr_set_t& set, attr_type_t type, rgb_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.rgb  = value;
            attr->value_type = attr_value_type_t::rgb;
        }

        u0 set(attr_set_t& set, attr_type_t type, rgba_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.rgba = value;
            attr->value_type = attr_value_type_t::rgba;
        }

        u0 set(attr_set_t& set, attr_type_t type, rect_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.rect = value;
            attr->value_type = attr_value_type_t::rect;
        }

        u0 set(attr_set_t& set, attr_type_t type, point_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type          = type;
            attr->value.point   = value;
            attr->value_type    = attr_value_type_t::point;
        }

        u0 set(attr_set_t& set, attr_type_t type, color_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type          = type;
            attr->value.color   = value;
            attr->value_type    = attr_value_type_t::color;
        }

        u0 set(attr_set_t& set, attr_type_t type, viewport_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            attr = &array::append(set.values);
            attr->type              = type;
            attr->value.viewport    = value;
            attr->value_type        = attr_value_type_t::viewport;
        }

        u0 set(attr_set_t& set, attr_type_t type, str::slice_t value) {
            auto attr = get(set, type);
            if (attr)
                return;
            auto r = string::interned::fold_for_result(value);
            attr = &array::append(set.values);
            attr->type       = type;
            attr->value.dw   = r.id;
            attr->value_type = attr_value_type_t::string;
        }

        status_t init(attr_set_t& set, component_type_t type, alloc_t* alloc) {
            set.alloc = alloc;
            set.type  = type;
            array::init(set.values, set.alloc);
            return status_t::ok;
        }
    }
}
