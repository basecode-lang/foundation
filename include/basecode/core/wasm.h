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
    struct name_t;
    struct module_t;
    struct section_t;
    struct valtype_t;
    struct br_table_t;
    struct func_body_t;
    struct func_type_t;
    struct global_var_t;
    struct table_type_t;
    struct local_entry_t;
    struct instruction_t;
    struct global_type_t;
    struct import_type_t;
    struct export_type_t;
    struct elem_segment_t;
    struct data_segment_t;
    struct resizable_limits_t;

    using name_array_t          = array_t<name_t>;
    using module_table_t        = hashtab_t<str::slice_t, module_t>;
    using index_array_t         = array_t<u32>;
    using target_array_t        = array_t<u32>;
    using limits_array_t        = array_t<resizable_limits_t>;
    using valtype_array_t       = array_t<valtype_t>;
    using section_array_t       = array_t<section_t>;
    using fun_type_array_t      = array_t<func_type_t>;
    using fun_body_array_t      = array_t<func_body_t>;
    using tab_type_array_t      = array_t<table_type_t>;
    using imp_type_array_t      = array_t<import_type_t>;
    using exp_type_array_t      = array_t<export_type_t>;
    using global_var_array_t    = array_t<global_var_t>;
    using local_entry_array_t   = array_t<local_entry_t>;
    using instruction_array_t   = array_t<instruction_t>;
    using global_type_array_t   = array_t<global_type_t>;
    using dat_segment_array_t   = array_t<data_segment_t>;
    using elem_segment_array_t  = array_t<elem_segment_t>;

    enum class op_code_t : u16 {
        unreachable             = 0x0000,
        nop                     = 0x0001,
        block                   = 0x0002,
        loop                    = 0x0003,
        if_                     = 0x0004,
        else_                   = 0x0005,
        try_                    = 0x0006,
        catch_                  = 0x0007,
        throw_                  = 0x0008,
        rethrow                 = 0x0009,
        unwind                  = 0x000a,
        end                     = 0x000b,
        br                      = 0x000c,
        br_if                   = 0x000d,
        br_table                = 0x000e,
        return_                 = 0x000f,
        call                    = 0x0010,
        calli                   = 0x0011,
        return_call             = 0x0012,
        return_calli            = 0x0013,
        delegate                = 0x0018,
        catch_all               = 0x0019,
        drop                    = 0x001a,
        select                  = 0x001b,
        typed_select            = 0x001c,
        local_get               = 0x0020,
        local_set               = 0x0021,
        local_tee               = 0x0022,
        global_get              = 0x0023,
        global_set              = 0x0024,
        table_get               = 0x0025,
        table_set               = 0x0026,
        i32_load                = 0x0028,
        i64_load                = 0x0029,
        f32_load                = 0x002a,
        f64_load                = 0x002b,
        i32_load8_s             = 0x002c,
        i32_load8_u             = 0x002d,
        i32_load16_s            = 0x002e,
        i32_load16_u            = 0x002f,
        i64_load8_s             = 0x0030,
        i64_load8_u             = 0x0031,
        i64_load16_s            = 0x0032,
        i64_load16_u            = 0x0033,
        i64_load32_s            = 0x0034,
        i64_load32_u            = 0x0035,
        i32_store               = 0x0036,
        i64_store               = 0x0037,
        f32_store               = 0x0038,
        f64_store               = 0x0039,
        i32_store8              = 0x003a,
        i64_store8              = 0x003b,
        i32_store16             = 0x003c,
        i64_store16             = 0x003d,
        i64_store32             = 0x003e,
        memory_size             = 0x003f,
        memory_grow             = 0x0040,
        i32_const               = 0x0041,
        i64_const               = 0x0042,
        f32_const               = 0x0043,
        f64_const               = 0x0044,
        i32_eqz                 = 0x0045,
        i32_eq                  = 0x0046,
        i32_ne                  = 0x0047,
        i32_lt_s                = 0x0048,
        i32_lt_u                = 0x0049,
        i32_gt_s                = 0x004a,
        i32_gt_u                = 0x004b,
        i32_le_s                = 0x004c,
        i32_le_u                = 0x004d,
        i32_ge_s                = 0x004e,
        i32_ge_u                = 0x004f,
        i64_eqz                 = 0x0050,
        i64_eq                  = 0x0051,
        i64_ne                  = 0x0052,
        i64_lt_s                = 0x0053,
        i64_lt_u                = 0x0054,
        i64_gt_s                = 0x0055,
        i64_gt_u                = 0x0056,
        i64_le_s                = 0x0057,
        i64_le_u                = 0x0058,
        i64_ge_s                = 0x0059,
        i64_ge_u                = 0x005a,
        f32_eq                  = 0x005b,
        f32_ne                  = 0x005c,
        f32_lt                  = 0x005d,
        f32_gt                  = 0x005e,
        f32_le                  = 0x005f,
        f32_ge                  = 0x0060,
        f64_eq                  = 0x0061,
        f64_ne                  = 0x0062,
        f64_lt                  = 0x0063,
        f64_gt                  = 0x0064,
        f64_le                  = 0x0065,
        f64_ge                  = 0x0066,
        i32_clz                 = 0x0067,
        i32_ctz                 = 0x0068,
        i32_popcnt              = 0x0069,
        i32_add                 = 0x006a,
        i32_sub                 = 0x006b,
        i32_mul                 = 0x006c,
        i32_div_s               = 0x006d,
        i32_div_u               = 0x006e,
        i32_rem_s               = 0x006f,
        i32_rem_u               = 0x0070,
        i32_and                 = 0x0071,
        i32_or                  = 0x0072,
        i32_xor                 = 0x0073,
        i32_shl                 = 0x0074,
        i32_shr_s               = 0x0075,
        i32_shr_u               = 0x0076,
        i32_rotl                = 0x0077,
        i32_rotr                = 0x0078,
        i64_clz                 = 0x0079,
        i64_ctz                 = 0x007a,
        i64_popcnt              = 0x007b,
        i64_add                 = 0x007c,
        i64_sub                 = 0x007d,
        i64_mul                 = 0x007e,
        i64_div_s               = 0x007f,
        i64_div_u               = 0x0080,
        i64_rem_s               = 0x0081,
        i64_rem_u               = 0x0082,
        i64_and                 = 0x0083,
        i64_or                  = 0x0084,
        i64_xor                 = 0x0085,
        i64_shl                 = 0x0086,
        i64_shr_s               = 0x0087,
        i64_shr_u               = 0x0088,
        i64_rotl                = 0x0089,
        i64_rotr                = 0x008a,
        f32_abs                 = 0x008b,
        f32_neg                 = 0x008c,
        f32_ceil                = 0x008d,
        f32_floor               = 0x008e,
        f32_trunc               = 0x008f,
        f32_nearest             = 0x0090,
        f32_sqrt                = 0x0091,
        f32_add                 = 0x0092,
        f32_sub                 = 0x0093,
        f32_mul                 = 0x0094,
        f32_div                 = 0x0095,
        f32_min                 = 0x0096,
        f32_max                 = 0x0097,
        f32_copysign            = 0x0098,
        f64_abs                 = 0x0099,
        f64_neg                 = 0x009a,
        f64_ceil                = 0x009b,
        f64_floor               = 0x009c,
        f64_trunc               = 0x009d,
        f64_nearest             = 0x009e,
        f64_sqrt                = 0x009f,
        f64_add                 = 0x00a0,
        f64_sub                 = 0x00a1,
        f64_mul                 = 0x00a2,
        f64_div                 = 0x00a3,
        f64_min                 = 0x00a4,
        f64_max                 = 0x00a5,
        f64_copysign            = 0x00a6,
        i32_wrap_i64            = 0x00a7,
        i32_trunc_f32_s         = 0x00a8,
        i32_trunc_f32_u         = 0x00a9,
        i32_trunc_f64_s         = 0x00aa,
        i32_trunc_f64_u         = 0x00ab,
        i64_extend_i32_s        = 0x00ac,
        i64_extend_i32_u        = 0x00ad,
        i64_trunc_f32_s         = 0x00ae,
        i64_trunc_f32_u         = 0x00af,
        i64_trunc_f64_s         = 0x00b0,
        i64_trunc_f64_u         = 0x00b1,
        f32_convert_i32_s       = 0x00b2,
        f32_convert_i32_u       = 0x00b3,
        f32_convert_i64_s       = 0x00b4,
        f32_convert_i64_u       = 0x00b5,
        f32_demote_f64          = 0x00b6,
        f64_convert_i32_s       = 0x00b7,
        f64_convert_i32_u       = 0x00b8,
        f64_convert_i64_s       = 0x00b9,
        f64_convert_i64_u       = 0x00ba,
        f64_promote_f32         = 0x00bb,
        i32_reinterpret_f32     = 0x00bc,
        i64_reinterpret_f64     = 0x00bd,
        f32_reinterpret_i32     = 0x00be,
        f64_reinterpret_i64     = 0x00bf,
        i32_extend8_s           = 0x00c0,
        i32_extend16_s          = 0x00c1,
        i64_extend8_s           = 0x00c2,
        i64_extend16_s          = 0x00c3,
        i64_extend32_s          = 0x00c4,
        null                    = 0x00d0,
        is_null                 = 0x00d1,
        ref_func                = 0x00d2,
        i32_trunc_sat_f32_s     = 0xfc00,
        i32_trunc_sat_f32_u     = 0xfc01,
        i32_trunc_sat_f64_s     = 0xfc02,
        i32_trunc_sat_f64_u     = 0xfc03,
        i64_trunc_sat_f32_s     = 0xfc04,
        i64_trunc_sat_f32_u     = 0xfc05,
        i64_trunc_sat_f64_s     = 0xfc06,
        i64_trunc_sat_f64_u     = 0xfc07,
        memory_init             = 0xfc08,
        data_drop               = 0xfc09,
        memory_copy             = 0xfc0a,
        memory_fill             = 0xfc0b,
        table_init              = 0xfc0c,
        elem_drop               = 0xfc0d,
        table_copy              = 0xfc0e,
        table_grow              = 0xfc0f,
        table_size              = 0xfc10,
        table_fill              = 0xfc11,
    };

    enum class op_prefix_t : u8 {
        reserved                = 0xff,
        threads                 = 0xfe,
        simd                    = 0xfd,
        misc                    = 0xfc,
    };

    enum class name_type_t : u8 {
        module                  = 0x00,
        function                = 0x01,
        local                   = 0x02,
    };

    enum class elem_type_t : u8 {
        func_ref                = 0x70,
    };

    enum class core_type_t : u8 {
        none                    = 0x00,
        i32                     = 0x7f,
        i64                     = 0x7e,
        f32                     = 0x7d,
        f64                     = 0x7c,
        func                    = 0x20,
        exn_ref                 = 0x18,
        func_ref                = 0x70,
        extern_ref              = 0x6f,
        empty_block             = 0x40,
    };

    enum class type_group_t : u8 {
        function                = 0x60,
        module                  = 0x61,
        instance                = 0x62,
    };

    enum class valtype_kind_t : u8 {
        none,
        num,
        ref,
    };

    enum class section_type_t : u8 {
        custom                  = 0,
        type                    = 1,
        import                  = 2,
        function                = 3,
        table                   = 4,
        memory                  = 5,
        global                  = 6,
        export_                 = 7,
        start                   = 8,
        element                 = 9,
        code                    = 10,
        data                    = 11,
        data_count              = 12,
    };

    enum class external_kind_t : u8 {
        function                = 0,
        table                   = 1,
        memory                  = 2,
        global                  = 3,
    };

    struct name_t final {
        str::slice_t            payload;
    };

    struct br_table_t final {
        target_array_t          targets_table;
        u32                     default_target;
    };

    struct valtype_t final {
        core_type_t             code;
        valtype_kind_t          category;
    };

    struct func_type_t final {
        valtype_array_t         params;
        valtype_array_t         returns;
        u32                     index;
    };

    struct local_entry_t final {
        u32                     count;
        valtype_t               value_type;
    };

    union instruction_subclass_t final {
        br_table_t              br_table;
        struct {
            u32                 memory;
            u32                 offset;
            u8                  align;
        }                       mem_imm;
        struct {
            u32                 memory;
            u8                  byte;
        }                       alloc_imm;
        struct {
            u32                 type_idx;
            u8                  reserved;
        }                       calli_imm;
        struct {
            u32                 index;
            u32                 table_index;
        }                       ret_calli_imm;
        struct {
            u32                 index;
            u32                 segment;
        }                       seg_idx_imm;
        struct {
            u32                 dst;
            u32                 src;
        }                       copy_imm;
        struct {
            u32                 results;
            u8                  type;
        }                       typed_select;
        u32                     dw;
        u64                     qw;
        u8                      type;
    };

    struct instruction_t final {
        u8*                     data;
        instruction_subclass_t  subclass;
        u32                     size;
        op_code_t               op_code;
    };

    struct func_body_t final {
        local_entry_array_t     locals;
        instruction_array_t     instructions;
    };

    struct elem_segment_t final {
        index_array_t           funcs;
        instruction_array_t     offset;
        u32                     tab_index;
    };

    struct data_segment_t final {
        u8*                     data;
        instruction_array_t     offset;
        u32                     size;
        u32                     index;
    };

    struct resizable_limits_t final {
        u32                     initial;
        u32                     maximum;
        u8                      flags;
    };

    struct table_type_t final {
        resizable_limits_t      limits;
        elem_type_t             type;
    };

    struct global_type_t final {
        valtype_t               content_type;
        b8                      mutability;
    };

    struct global_var_t final {
        instruction_array_t     init;
        global_type_t           type;
    };

    struct import_type_t final {
        str::slice_t            field_name;
        str::slice_t            module_name;
        union {
            u32                 index;
            table_type_t        table;
            global_type_t       global;
            resizable_limits_t  memory;
        }                       subclass;
        external_kind_t         kind;
    };

    struct export_type_t final {
        str::slice_t            field_name;
        u32                     index;
        external_kind_t         kind;
    };

    union section_subclass_t final {
        struct {
            fun_type_array_t    funcs;
            type_group_t        group;
        }                       type;
        struct {
            fun_body_array_t    bodies;
        }                       code;
        struct {
            dat_segment_array_t segments;
        }                       data;
        struct {
            tab_type_array_t    elements;
        }                       table;
        struct {
            imp_type_array_t    entries;
        }                       import;
        struct {
            exp_type_array_t    entries;
        }                       export_;
        struct {
            name_array_t        names;
            u32                 size;
            name_type_t         type;
        }                       custom;
        struct {
            limits_array_t      limits;
        }                       memory;
        struct {
            global_var_array_t  vars;
        }                       global;
        struct {
            elem_segment_array_t entries;
        }                       element;
        index_array_t           function;
        u32                     ds_count;
        u32                     start_func_idx;
    };

    struct section_t final {
        module_t*               module;
        u8*                     payload;
        section_subclass_t      subclass;
        str::slice_t            name;
        u32                     size;
        section_type_t          type;
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
            invalid_section,
            unsupported_op_prefix,
        };

        namespace type {
            str::slice_t name(core_type_t type);

            valtype_t make_valtype(core_type_t type);
        }

        namespace module {
            u0 free(module_t& module);

            status_t decode(module_t& module);

            status_t init(module_t& module, wasm_t* wasm);

            status_t make_section(module_t& module, section_t** section);
        }

        namespace section {
            namespace type_group {
                str::slice_t name(type_group_t group);
            }

            u0 free(section_t& section);

            str::slice_t name(section_type_t type);
        }

        namespace name_type {
            str::slice_t name(name_type_t type);
        }

        namespace instruction {
            str::slice_t name(op_code_t op);

            u0 format_body(const func_body_t& body, u32 addr, u32 idx);

            status_t read_body(module_t& module, instruction_array_t& list);

            u0 format(const instruction_t& inst, u32 addr, u32& block_depth);
        }

        namespace external_kind {
            str::slice_t name(external_kind_t kind);
        }

        u0 free(wasm_t& wasm);

        module_t* load_module(wasm_t& wasm, const path_t& path);

        status_t init(wasm_t& wasm, alloc_t* alloc = context::top()->alloc);

        module_t* load_module(wasm_t& wasm, str::slice_t name, const u8* data, u32 size);
    }
}
