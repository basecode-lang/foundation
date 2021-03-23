// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/buf.h>
#include <basecode/core/path.h>
#include <basecode/core/array.h>
#include <basecode/core/hashtab.h>

namespace basecode {
    struct wasm_t;
    struct module_t;
    struct limits_t;
    struct section_t;

    using valtype_t             = u8;
    using reftype_t             = u8;
    using module_table_t        = hashtab_t<str::slice_t, module_t>;
    using valtype_array_t       = array_t<valtype_t>;
    using section_array_t       = array_t<section_t>;

    struct limits_t final {
        u32                     min;
        u32                     max;
    };

    struct extern_type_t final {
    };

    struct global_type_t final {
        valtype_t               valtype;
        b8                      mut;
    };

    struct table_type_t final {
        limits_t                limits;
        reftype_t               reftype;
    };

    struct func_type_t final {
        valtype_array_t         params;
        valtype_array_t         results;
        u32                     index;
    };

    struct memory_type_t final {
        limits_t                limits;
    };

    struct section_t final {
        module_t*               module;
        u8*                     data;
        u32                     size;
        u8                      type;
    };

    struct module_t final {
        wasm_t*                 wasm;
        buf_t                   buf;
        buf_crsr_t              crsr;
        str::slice_t            path;
        section_array_t         sections;
        u32                     magic;
        u32                     version;
    };

    struct wasm_t final {
        alloc_t*                alloc;
        module_table_t          modules;
    };

    namespace wasm {
        enum class status_t {
            ok,
            error,
            read_error,
            write_error,
        };

        namespace types {
            constexpr u8 i32                    = 0x7f;
            constexpr u8 i64                    = 0x7e;
            constexpr u8 f32                    = 0x7d;
            constexpr u8 f64                    = 0x7c;
            constexpr u8 func_ref               = 0x70;
            constexpr u8 extern_ref             = 0x6f;
            constexpr u8 exn_ref                = 0x18;
            constexpr u8 func                   = 0x20;
            constexpr u8 empty_block            = 0x40;

            str::slice_t name(u8 type);
        }

        namespace sections {
            constexpr u8 custom                 = 0;
            constexpr u8 type                   = 1;
            constexpr u8 import                 = 2;
            constexpr u8 function               = 3;
            constexpr u8 table                  = 4;
            constexpr u8 memory                 = 5;
            constexpr u8 global                 = 6;
            constexpr u8 export_                = 7;
            constexpr u8 start                  = 8;
            constexpr u8 element                = 9;
            constexpr u8 code                   = 10;
            constexpr u8 data                   = 11;
            constexpr u8 data_count             = 12;

            str::slice_t name(u8 type);
        }

        namespace instructions {
            constexpr u16 unreachable           = 0x0000;
            constexpr u16 nop                   = 0x0001;
            constexpr u16 block                 = 0x0002;
            constexpr u16 loop                  = 0x0003;
            constexpr u16 if_                   = 0x0004;
            constexpr u16 else_                 = 0x0005;
            constexpr u16 try_                  = 0x0006;
            constexpr u16 catch_                = 0x0007;
            constexpr u16 throw_                = 0x0008;
            constexpr u16 rethrow               = 0x0009;
            constexpr u16 unwind                = 0x000a;
            constexpr u16 end                   = 0x000b;
            constexpr u16 br                    = 0x000c;
            constexpr u16 br_if                 = 0x000d;
            constexpr u16 br_table              = 0x000e;
            constexpr u16 return_               = 0x000f;
            constexpr u16 call                  = 0x0010;
            constexpr u16 calli                 = 0x0011;
            constexpr u16 return_call           = 0x0012;
            constexpr u16 return_calli          = 0x0013;
            constexpr u16 delegate              = 0x0018;
            constexpr u16 catch_all             = 0x0019;
            constexpr u16 drop                  = 0x001a;
            constexpr u16 select                = 0x001b;
            constexpr u16 typed_select          = 0x001c;
            constexpr u16 local_get             = 0x0020;
            constexpr u16 local_set             = 0x0021;
            constexpr u16 local_tee             = 0x0022;
            constexpr u16 global_get            = 0x0023;
            constexpr u16 global_set            = 0x0024;
            constexpr u16 table_get             = 0x0025;
            constexpr u16 table_set             = 0x0026;
            constexpr u16 table_init            = 0xfc0c;
            constexpr u16 elem_drop             = 0xfc0d;
            constexpr u16 table_copy            = 0xfc0e;
            constexpr u16 table_grow            = 0xfc0f;
            constexpr u16 table_size            = 0xfc10;
            constexpr u16 table_fill            = 0xfc11;
            constexpr u16 i32_load              = 0x0028;
            constexpr u16 i64_load              = 0x0029;
            constexpr u16 f32_load              = 0x002a;
            constexpr u16 f64_load              = 0x002b;
            constexpr u16 i32_load8_s           = 0x002c;
            constexpr u16 i32_load8_u           = 0x002d;
            constexpr u16 i32_load16_s          = 0x002e;
            constexpr u16 i32_load16_u          = 0x002f;
            constexpr u16 i64_load8_s           = 0x0030;
            constexpr u16 i64_load8_u           = 0x0031;
            constexpr u16 i64_load16_s          = 0x0032;
            constexpr u16 i64_load16_u          = 0x0033;
            constexpr u16 i64_load32_s          = 0x0034;
            constexpr u16 i64_load32_u          = 0x0035;
            constexpr u16 i32_store             = 0x0036;
            constexpr u16 i64_store             = 0x0037;
            constexpr u16 f32_store             = 0x0038;
            constexpr u16 f64_store             = 0x0039;
            constexpr u16 i32_store8            = 0x003a;
            constexpr u16 i64_store8            = 0x003b;
            constexpr u16 i32_store16           = 0x003c;
            constexpr u16 i64_store16           = 0x003d;
            constexpr u16 i64_store32           = 0x003e;
            constexpr u16 memory_size           = 0x3f00;
            constexpr u16 memory_grow           = 0x4000;
            constexpr u16 memory_init           = 0xfc08;
            constexpr u16 data_drop             = 0xfc09;
            constexpr u16 memory_copy           = 0xfc0a;
            constexpr u16 memory_fill           = 0xfc0b;
            constexpr u16 i32_const             = 0x0041;
            constexpr u16 i64_const             = 0x0042;
            constexpr u16 f32_const             = 0x0043;
            constexpr u16 f64_const             = 0x0044;
            constexpr u16 i32_eqz               = 0x0045;
            constexpr u16 i32_eq                = 0x0046;
            constexpr u16 i32_ne                = 0x0047;
            constexpr u16 i32_lt_s              = 0x0048;
            constexpr u16 i32_lt_u              = 0x0049;
            constexpr u16 i32_gt_s              = 0x004a;
            constexpr u16 i32_gt_u              = 0x004b;
            constexpr u16 i32_le_s              = 0x004c;
            constexpr u16 i32_le_u              = 0x004d;
            constexpr u16 i32_ge_s              = 0x004e;
            constexpr u16 i32_ge_u              = 0x004f;
            constexpr u16 i64_eqz               = 0x0050;
            constexpr u16 i64_eq                = 0x0051;
            constexpr u16 i64_ne                = 0x0052;
            constexpr u16 i64_lt_s              = 0x0053;
            constexpr u16 i64_lt_u              = 0x0054;
            constexpr u16 i64_gt_s              = 0x0055;
            constexpr u16 i64_gt_u              = 0x0056;
            constexpr u16 i64_le_s              = 0x0057;
            constexpr u16 i64_le_u              = 0x0058;
            constexpr u16 i64_ge_s              = 0x0059;
            constexpr u16 i64_ge_u              = 0x005a;
            constexpr u16 f32_eq                = 0x005b;
            constexpr u16 f32_ne                = 0x005c;
            constexpr u16 f32_lt                = 0x005d;
            constexpr u16 f32_gt                = 0x005e;
            constexpr u16 f32_le                = 0x005f;
            constexpr u16 f32_ge                = 0x0060;
            constexpr u16 f64_eq                = 0x0061;
            constexpr u16 f64_ne                = 0x0062;
            constexpr u16 f64_lt                = 0x0063;
            constexpr u16 f64_gt                = 0x0064;
            constexpr u16 f64_le                = 0x0065;
            constexpr u16 f64_ge                = 0x0066;
            constexpr u16 i32_clz               = 0x0067;
            constexpr u16 i32_ctz               = 0x0068;
            constexpr u16 i32_popcnt            = 0x0069;
            constexpr u16 i32_add               = 0x006a;
            constexpr u16 i32_sub               = 0x006b;
            constexpr u16 i32_mul               = 0x006c;
            constexpr u16 i32_div_s             = 0x006d;
            constexpr u16 i32_div_u             = 0x006e;
            constexpr u16 i32_rem_s             = 0x006f;
            constexpr u16 i32_rem_u             = 0x0070;
            constexpr u16 i32_and               = 0x0071;
            constexpr u16 i32_or                = 0x0072;
            constexpr u16 i32_xor               = 0x0073;
            constexpr u16 i32_shl               = 0x0074;
            constexpr u16 i32_shr_s             = 0x0075;
            constexpr u16 i32_shr_u             = 0x0076;
            constexpr u16 i32_rotl              = 0x0077;
            constexpr u16 i32_rotr              = 0x0078;
            constexpr u16 i64_clz               = 0x0079;
            constexpr u16 i64_ctz               = 0x007a;
            constexpr u16 i64_popcnt            = 0x007b;
            constexpr u16 i64_add               = 0x007c;
            constexpr u16 i64_sub               = 0x007d;
            constexpr u16 i64_mul               = 0x007e;
            constexpr u16 i64_div_s             = 0x007f;
            constexpr u16 i64_div_u             = 0x0080;
            constexpr u16 i64_rem_s             = 0x0081;
            constexpr u16 i64_rem_u             = 0x0082;
            constexpr u16 i64_and               = 0x0083;
            constexpr u16 i64_or                = 0x0084;
            constexpr u16 i64_xor               = 0x0085;
            constexpr u16 i64_shl               = 0x0086;
            constexpr u16 i64_shr_s             = 0x0087;
            constexpr u16 i64_shr_u             = 0x0088;
            constexpr u16 i64_rotl              = 0x0089;
            constexpr u16 i64_rotr              = 0x008a;
            constexpr u16 f32_abs               = 0x008b;
            constexpr u16 f32_neg               = 0x008c;
            constexpr u16 f32_ceil              = 0x008d;
            constexpr u16 f32_floor             = 0x008e;
            constexpr u16 f32_trunc             = 0x008f;
            constexpr u16 f32_nearest           = 0x0090;
            constexpr u16 f32_sqrt              = 0x0091;
            constexpr u16 f32_add               = 0x0092;
            constexpr u16 f32_sub               = 0x0093;
            constexpr u16 f32_mul               = 0x0094;
            constexpr u16 f32_div               = 0x0095;
            constexpr u16 f32_min               = 0x0096;
            constexpr u16 f32_max               = 0x0097;
            constexpr u16 f32_copysign          = 0x0098;
            constexpr u16 f64_abs               = 0x0099;
            constexpr u16 f64_neg               = 0x009a;
            constexpr u16 f64_ceil              = 0x009b;
            constexpr u16 f64_floor             = 0x009c;
            constexpr u16 f64_trunc             = 0x009d;
            constexpr u16 f64_nearest           = 0x009e;
            constexpr u16 f64_sqrt              = 0x009f;
            constexpr u16 f64_add               = 0x00a0;
            constexpr u16 f64_sub               = 0x00a1;
            constexpr u16 f64_mul               = 0x00a2;
            constexpr u16 f64_div               = 0x00a3;
            constexpr u16 f64_min               = 0x00a4;
            constexpr u16 f64_max               = 0x00a5;
            constexpr u16 f64_copysign          = 0x00a6;
            constexpr u16 i32_wrap_i64          = 0x00a7;
            constexpr u16 i32_trunc_f32_s       = 0x00a8;
            constexpr u16 i32_trunc_f32_u       = 0x00a9;
            constexpr u16 i32_trunc_f64_s       = 0x00aa;
            constexpr u16 i32_trunc_f64_u       = 0x00ab;
            constexpr u16 i64_extend_i32_s      = 0x00ac;
            constexpr u16 i64_extend_i32_u      = 0x00ad;
            constexpr u16 i64_trunc_f32_s       = 0x00ae;
            constexpr u16 i64_trunc_f32_u       = 0x00af;
            constexpr u16 i64_trunc_f64_s       = 0x00b0;
            constexpr u16 i64_trunc_f64_u       = 0x00b1;
            constexpr u16 f32_convert_i32_s     = 0x00b2;
            constexpr u16 f32_convert_i32_u     = 0x00b3;
            constexpr u16 f32_convert_i64_s     = 0x00b4;
            constexpr u16 f32_convert_i64_u     = 0x00b5;
            constexpr u16 f32_demote_f64        = 0x00b6;
            constexpr u16 f64_convert_i32_s     = 0x00b7;
            constexpr u16 f64_convert_i32_u     = 0x00b8;
            constexpr u16 f64_convert_i64_s     = 0x00b9;
            constexpr u16 f64_convert_i64_u     = 0x00ba;
            constexpr u16 f64_promote_f32       = 0x00bb;
            constexpr u16 i32_reinterpret_f32   = 0x00bc;
            constexpr u16 i64_reinterpret_f64   = 0x00bd;
            constexpr u16 f32_reinterpret_i32   = 0x00be;
            constexpr u16 f64_reinterpret_i64   = 0x00bf;
            constexpr u16 i32_extend8_s         = 0x00c0;
            constexpr u16 i32_extend16_s        = 0x00c1;
            constexpr u16 i64_extend8_s         = 0x00c2;
            constexpr u16 i64_extend16_s        = 0x00c3;
            constexpr u16 i64_extend32_s        = 0x00c4;
            constexpr u16 null                  = 0x00d0;
            constexpr u16 is_null               = 0x00d1;
            constexpr u16 ref_func              = 0x00d2;
            constexpr u16 i32_trunc_sat_f32_s   = 0xfc00;
            constexpr u16 i32_trunc_sat_f32_u   = 0xfc01;
            constexpr u16 i32_trunc_sat_f64_s   = 0xfc02;
            constexpr u16 i32_trunc_sat_f64_u   = 0xfc03;
            constexpr u16 i64_trunc_sat_f32_s   = 0xfc04;
            constexpr u16 i64_trunc_sat_f32_u   = 0xfc05;
            constexpr u16 i64_trunc_sat_f64_s   = 0xfc06;
            constexpr u16 i64_trunc_sat_f64_u   = 0xfc07;

            str::slice_t name(u16 op);
        }

        namespace module {
            u0 free(module_t& module);

            status_t decode(module_t& module);

            status_t init(module_t& module, wasm_t* wasm);
        }

        u0 free(wasm_t& wasm);

        module_t* load_module(wasm_t& wasm, const path_t& path);

        status_t init(wasm_t& wasm, alloc_t* alloc = context::top()->alloc);

        module_t* load_module(wasm_t& wasm, str::slice_t name, const u8* data, u32 size);
    }
}
