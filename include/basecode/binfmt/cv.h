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

#include <basecode/core/uuid.h>
#include <basecode/binfmt/io.h>

#define LF_PAD(n) (0xf0 + (n))

namespace basecode::binfmt::cv {
    using type_t                = u32;
    using item_id_t             = u32;

    constexpr u32 CV_SIGNATURE_C6         = 0L;  // Actual signature is >64K
    constexpr u32 CV_SIGNATURE_C7         = 1L;  // First explicit signature
    constexpr u32 CV_SIGNATURE_C11        = 2L;  // C11 (vc5.x) 32-bit types
    constexpr u32 CV_SIGNATURE_C13        = 4L;  // C13 (vc7.x) zero terminated names
    constexpr u32 CV_SIGNATURE_RESERVED   = 5L;  // All signatures from 5 to 64K are reserved

    enum class debug_subsection_type_t : u32 {
        ignore                  = 0x80000000,
        symbols                 = 0xf1,
        lines,
        string_table,
        file_chksms,
        frame_data,
        inlinee_lines,
        cross_scope_imports,
        cross_scope_exports,
        il_lines,
        func_mdtoken_map,
        type_mdtoken_map,
        merged_assembly_input,
        coff_symbol_rva,
    };

    enum class ptr_kind_t : u8 {
        near_16                 = 0x00,
        far_16                  = 0x01,
        huge_16                 = 0x02,
        based_on_segment        = 0x03,
        based_on_value          = 0x04,
        based_on_segment_value  = 0x05,
        based_on_address        = 0x06,
        based_on_segment_addr   = 0x07,
        based_on_type           = 0x08,
        based_on_self           = 0x09,
        near_32                 = 0x0a,
        far_32                  = 0x0b,
        near_64                 = 0x0c
    };

    enum class ptr_mode_t : u8 {
        pointer                 = 0x00,
        lvalue_reference        = 0x01,
        ptr_to_data_member      = 0x02,
        ptr_to_member_function  = 0x03,
        rvalue_reference        = 0x04
    };

    enum class ptr_mods_t : u8 {
        none                    = 0x00,
        flat_32                 = 0x01,
        volatile_               = 0x02,
        const_                  = 0x04,
        unaligned               = 0x08,
        restrict                = 0x10,
    };

    enum class ptr_flags_t : u8 {
        winrt_smart_pointer     = 0x01,
        lvalue_ref_this_pointer = 0x02,
        rvalue_ref_this_pointer = 0x04
    };

    enum class build_info_t : u8 {
        current_directory       = 0,
        build_tool              = 1,
        source_file             = 2,
        pdb_file                = 3,
        command_args            = 4,
    };

    enum class tramp_type_t : u16 {
        incremental,
        branch_island
    };

    enum class cookie_type_t {
        copy,
        xor_sp,
        xor_bp,
        xor_r13
    };

    struct subsection_header_t final {
        debug_subsection_type_t type;
        s32                     len;
    };

    constexpr u32 CV_LINES_HAVE_COLUMNS = 0x0001;

    struct line_t final {
        u32                     offset;
        u32                     line_num_start:     24;
        u32                     delta_line_end:     7;
        u32                     is_statement:       1;
    };

    struct column_t final {
        u16                     offset_col_start;
        u16                     offset_col_end;
    };

    struct c13_lines_hdr_t final {
        u32                     offset;
        u16                     seg;
        u16                     flags;
        u32                     len;
    };

    struct c13_file_block_t final {
        u32                     file_id;
        u32                     num_lines;
        u32                     len;
        line_t                  lines[1];
        column_t                columns[0];
    };

    struct rec_header_t final {
        u16                     len;
        u16                     kind;
        u8                      data[0];
    };

    struct prop_t final {
        u16                     packed:         1;
        u16                     ctor:           1;
        u16                     ovlops:         1;
        u16                     is_nested:      1;
        u16                     cnested:        1;
        u16                     opassign:       1;
        u16                     opcast:         1;
        u16                     fwd_ref:        1;
        u16                     scoped:         1;
        u16                     has_unique_name:1;
        u16                     sealed:         1;
        u16                     hfa:            1;
        u16                     intrinsic:      1;
        u16                     mo_com:         1;
    };

    struct modifier_t final {
        u16                     is_const:       1;
        u16                     is_volatile:    1;
        u16                     is_unaligned:   1;
        u16                     pad:            13;
    };

    struct func_attr_t final {
        u8                      cxx_return_udt: 1;
        u8                      ctor:           1;
        u8                      ctor_vbase:     1;
        u8                      unused:         5;
    };

    struct field_attr_t final {
        u16                     access:         2;
        u16                     mprop:          3;
        u16                     pseudo:         1;
        u16                     no_inherit:     1;
        u16                     no_construct:   1;
        u16                     comp_genx:      1;
        u16                     sealed:         1;
        u16                     unused:         6;
    };

    struct matrix_attr_t final {
        u8                      row_major:      1;
        u8                      unused:         7;
    };

    struct lvar_flags_t final {
        u16                     is_param:       1;
        u16                     addr_taken:     1;
        u16                     comp_genx:      1;
        u16                     is_aggregate:   1;
        u16                     is_aggregated:  1;
        u16                     is_aliased:     1;
        u16                     is_alias:       1;
        u16                     is_ret_value:   1;
        u16                     is_optimized:   1;
        u16                     is_enreg_glob:  1;
        u16                     is_enreg_stat:  1;
        u16                     unused:         5;
    };

    struct lvar_attr_t final {
        u32                     offset;
        u16                     seg;
        lvar_flags_t            flags;
    };

    struct lvar_addr_range_t final {
        u32                     offset_start;
        u16                     sect_start;
        u16                     length;
    };

    struct lvar_addr_gap_t final {
        u16                     start_offset;
        u16                     length;
    };

    struct generic_flag_t final {
        u16                     cstyle:         1;
        u16                     rsclean:        1;
        u16                     unused:         14;
    };

    struct range_attr_t final {
        u16                     maybe:      1;
        u16                     pad:        15;
    };

    union pub_flags_t final {
        u32                     flags;
        struct {
            u32                 code:           1;
            u32                 function:       1;
            u32                 managed:        1;
            u32                 msil:           1;
            u32                 pad:            28;
        };
    };

    struct proc_flags_t final {
        union {
            u8                  all;
            struct {
                u8              no_fpo:         1;
                u8              interrupt_re:   1;
                u8              far_ret:        1;
                u8              never_ret:      1;
                u8              not_reached:    1;
                u8              custom_call:    1;
                u8              no_inline:      1;
                u8              opt_dbg_info:   1;
            };
        };
    };

    constexpr u32 LF_ALIAS              = 0x150a;   // ok
    constexpr u32 LF_BCLASS             = 0x1400;   // ok
    constexpr u32 LF_BINTERFACE         = 0x151a;   // ok
    constexpr u32 LF_VBCLASS            = 0x1401;   // ok
    constexpr u32 LF_IVBCLASS           = 0x1402;   // ok
    constexpr u32 LF_VFUNCTAB           = 0x1409;   // ok
    constexpr u32 LF_STMEMBER           = 0x150e;   // ok
    constexpr u32 LF_METHOD             = 0x150f;   // ok
    constexpr u32 LF_MEMBER             = 0x150d;   // ok
    constexpr u32 LF_NESTTYPE           = 0x1510;   // ok
    constexpr u32 LF_ONEMETHOD          = 0x1511;   // ok
    constexpr u32 LF_ENUMERATE          = 0x1502;   // ok
    constexpr u32 LF_INDEX              = 0x1404;   // ok
    constexpr u32 LF_POINTER            = 0x1002;   // ok
    constexpr u32 LF_MODIFIER           = 0x1001;   // ok
    constexpr u32 LF_PROCEDURE          = 0x1008;   // ok
    constexpr u32 LF_MFUNCTION          = 0x1009;   // ok
    constexpr u32 LF_LABEL              = 0x000e;   // ok
    constexpr u32 LF_ARGLIST            = 0x1201;   // ok
    constexpr u32 LF_FIELDLIST          = 0x1203;   // ok
    constexpr u32 LF_ARRAY              = 0x1503;   // ok
    constexpr u32 LF_CLASS              = 0x1504;   // ok
    constexpr u32 LF_STRUCTURE          = 0x1505;   // ok
    constexpr u32 LF_INTERFACE          = 0x1519;   // ok
    constexpr u32 LF_UNION              = 0x1506;   // ok
    constexpr u32 LF_ENUM               = 0x1507;   // ok
    constexpr u32 LF_TYPESERVER2        = 0x1515;   // ok
    constexpr u32 LF_VFTABLE            = 0x151d;   // ok
    constexpr u32 LF_VTSHAPE            = 0x000a;   // ok
    constexpr u32 LF_BITFIELD           = 0x1205;   // ok
    constexpr u32 LF_FUNC_ID            = 0x1601;   // ok
    constexpr u32 LF_MFUNC_ID           = 0x1602;   // ok
    constexpr u32 LF_BUILDINFO          = 0x1603;   // ok
    constexpr u32 LF_SUBSTR_LIST        = 0x1604;   // ok
    constexpr u32 LF_STRING_ID          = 0x1605;   // ok
    constexpr u32 LF_UDT_SRC_LINE       = 0x1606;   // ok
    constexpr u32 LF_UDT_MOD_SRC_LINE   = 0x1607;   // ok
    constexpr u32 LF_METHODLIST         = 0x1206;   // ok
    constexpr u32 LF_PRECOMP            = 0x1509;   // ok
    constexpr u32 LF_ENDPRECOMP         = 0x0014;   // ok

    constexpr u32 S_PUB32                                = 0x110e;      // ok
    constexpr u32 S_GDATA32                              = 0x110d;      // ok
    constexpr u32 S_GTHREAD32                            = 0x1113;      // ok
    constexpr u32 S_PROCREF                              = 0x1125;      // ok
    constexpr u32 S_LPROCREF                             = 0x1127;      // ok
    constexpr u32 S_GMANDATA                             = 0x111d;      // ok
    constexpr u32 S_END                                  = 0x0006;      // ok
    constexpr u32 S_FRAMEPROC                            = 0x1012;      // ok
    constexpr u32 S_OBJNAME                              = 0x1101;      // ok
    constexpr u32 S_THUNK32                              = 0x1102;      // ok
    constexpr u32 S_BLOCK32                              = 0x1103;      // ok
    constexpr u32 S_LABEL32                              = 0x1105;      // ok
    constexpr u32 S_REGISTER                             = 0x1106;      // ok
    constexpr u32 S_BPREL32                              = 0x110b;      // ok
    constexpr u32 S_LPROC32                              = 0x110f;      // ok
    constexpr u32 S_GPROC32                              = 0x1110;      // ok
    constexpr u32 S_REGREL32                             = 0x1111;      // ok
    constexpr u32 S_COMPILE2                             = 0x1116;      // ok
    constexpr u32 S_UNAMESPACE                           = 0x1124;      // ok
    constexpr u32 S_TRAMPOLINE                           = 0x112c;      // ok
    constexpr u32 S_SECTION                              = 0x1136;      // ok
    constexpr u32 S_COFFGROUP                            = 0x1137;      // ok
    constexpr u32 S_EXPORT                               = 0x1138;      // ok
    constexpr u32 S_CALLSITEINFO                         = 0x1139;      // ok
    constexpr u32 S_FRAMECOOKIE                          = 0x113a;      // ok
    constexpr u32 S_COMPILE3                             = 0x113c;      // ok
    constexpr u32 S_ENVBLOCK                             = 0x113d;      // ok
    constexpr u32 S_LOCAL                                = 0x113e;      // ok
    constexpr u32 S_DEFRANGE                             = 0x113f;      // ok
    constexpr u32 S_DEFRANGE_SUBFIELD                    = 0x1140;      // ok
    constexpr u32 S_DEFRANGE_REGISTER                    = 0x1141;      // ok
    constexpr u32 S_DEFRANGE_FRAMEPOINTER_REL            = 0x1142;      // ok
    constexpr u32 S_DEFRANGE_SUBFIELD_REGISTER           = 0x1143;      // ok
    constexpr u32 S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE = 0x1144;      // ok
    constexpr u32 S_DEFRANGE_REGISTER_REL                = 0x1145;      // ok
    constexpr u32 S_LPROC32_ID                           = 0x1146;      // ok
    constexpr u32 S_GPROC32_ID                           = 0x1147;      // ok
    constexpr u32 S_BUILDINFO                            = 0x114c;      // ok
    constexpr u32 S_INLINESITE                           = 0x114d;      // ok
    constexpr u32 S_INLINESITE_END                       = 0x114e;      // ok
    constexpr u32 S_PROC_ID_END                          = 0x114f;      // ok
    constexpr u32 S_FILESTATIC                           = 0x1153;      // ok
    constexpr u32 S_LPROC32_DPC                          = 0x1155;      // ok
    constexpr u32 S_LPROC32_DPC_ID                       = 0x1156;      // ok
    constexpr u32 S_CALLEES                              = 0x115a;      // ok
    constexpr u32 S_CALLERS                              = 0x115b;      // ok
    constexpr u32 S_HEAPALLOCSITE                        = 0x115e;      // ok
    constexpr u32 S_FASTLINK                             = 0x1167;
    constexpr u32 S_INLINEES                             = 0x1168;
    constexpr u32 S_CONSTANT                             = 0x1107;      // ok
    constexpr u32 S_UDT                                  = 0x1108;      // ok
    constexpr u32 S_LDATA32                              = 0x110c;      // ok
    constexpr u32 S_LTHREAD32                            = 0x1112;      // ok
    constexpr u32 S_LMANDATA                             = 0x111c;      // ok
    constexpr u32 S_MANCONSTANT                          = 0x112d;      // ok

    struct ml_method_t final {
        field_attr_t            attr;
        u16                     pad0;
        type_t                  index;
        u32                     vbase_offset[0];
    };

    namespace machine {
        constexpr u32 x86_64        = 0xd0;
    }

    namespace leaf {
        struct modifier_t final {
            u16                     leaf;
            cv::type_t              type;
            cv::modifier_t          attr;
        };

        struct pointer_t final {
            u16                     leaf;
            type_t                  utype;
            struct {
                u32                 ptr_type:       5;
                u32                 ptr_mode:       3;
                u32                 is_flat_32:     1;
                u32                 is_volatile:    1;
                u32                 is_const:       1;
                u32                 is_unaligned:   1;
                u32                 is_restrict:    1;
                u32                 size:           6;
                u32                 is_mo_com:      1;
                u32                 is_lref:        1;
                u32                 is_rref:        1;
                u32                 unused:         10;
            }                       attr;
            union {
                struct {
                    type_t          pm_class;
                    u16             pm_enum;
                }                   pm;
                u16                 bseg;
                u8                  sym[1];
                struct {
                    type_t          index;
                    u8              name[1];
                }                   btype;
            };
        };

        struct array_t final {
            u16                     leaf;
            cv::type_t              elem_type;
            cv::type_t              idx_type;
            u8                      data[0];
        };

        struct strided_array_t final {
            u16                     leaf;
            cv::type_t              elem_type;
            cv::type_t              idx_type;
            u32                     stride;
            u8                      data[0];
        };

        struct vector_t final {
            u16                     leaf;
            cv::type_t              elem_type;
            u32                     count;
            u8                      data[0];
        };

        struct matrix_t final {
            u16                     leaf;
            cv::type_t              elem_type;
            u32                     rows;
            u32                     cols;
            u32                     major_stride;
            cv::matrix_attr_t       mat_attr;
            u8                      data[0];
        };

        struct class_t final {
            u16                     leaf;
            u16                     count;
            cv::prop_t              property;
            cv::type_t              field;
            cv::type_t              derived;
            cv::type_t              vshape;
            u8                      data[0];
        };
        using struct_t           = class_t;
        using interface_t        = class_t;

        struct union_t final {
            u16                     leaf;
            u16                     count;
            cv::prop_t              property;
            cv::type_t              field;
            u8                      data[0];
        };

        struct alias_t final {
            u16                     leaf;
            cv::type_t              utype;
            u8                      name[1];
        };

        struct func_id_t final {
            u16                     leaf;
            cv::item_id_t           scope_id;
            cv::type_t              type;
            u8                      name[0];
        };

        struct mfunc_id_t final {
            u16                     leaf;
            cv::type_t              parent_type;
            cv::type_t              type;
            u8                      name[0];
        };

        struct string_id_t final {
            u16                     leaf;
            cv::item_id_t           id;
            u8                      name[0];
        };

        struct udt_src_line_t final {
            u16                     leaf;
            cv::type_t              type;
            cv::item_id_t           src;
            u32                     line;
        };

        struct udt_mod_src_line_t final {
            u16                     leaf;
            cv::type_t              type;
            cv::item_id_t           src;
            u32                     line;
            u16                     imod;
        };

        struct build_info_t final {
            u16                     leaf;
            u16                     count;
            cv::item_id_t           arg[sizeof(cv::build_info_t)];
        };

        struct managed_t final {
            u16                     leaf;
            u8                      name[1];
        };

        struct enum_t final {
            u16                     leaf;
            u16                     count;
            cv::prop_t              property;
            cv::type_t              utype;
            cv::type_t              field;
            u8                      name[1];
        };

        struct proc_t final {
            u16                     leaf;
            cv::type_t              rv_type;
            u8                      call_type;
            cv::func_attr_t         func_attr;
            u16                     param_count;
            cv::type_t              arg_list;
        };

        struct mfunc_t final {
            u16                     leaf;
            cv::type_t              rv_type;
            cv::type_t              class_type;
            cv::type_t              this_type;
            u8                      call_type;
            cv::func_attr_t         func_attr;
            u16                     param_count;
            cv::type_t              arg_list;
            s32                     this_adjust;
        };

        struct vt_shape_t final {
            u16                     leaf;
            u16                     count;
            u8                      desc[0];
        };

        struct vf_table_t final {
            u16                     leaf;
            cv::type_t              type;
            cv::type_t              base_vf_table;
            u32                     offset_in_object_layout;
            u32                     len;
            u8                      names[1];
        };

        struct label_t final {
            u16                     leaf;
            u16                     mode;
        };

        struct vft_path final {
            u16                     leaf;
            u32                     count;
            cv::type_t              base[1];
        };

        struct pre_comp_t final {
            u16                     leaf;
            u32                     start;
            u32                     count;
            u32                     signature;
            u8                      name[0];
        };

        struct end_pre_comp_t final {
            u16                     leaf;
            u32                     signature;
        };

        struct skip_t final {
            u16                     leaf;
            cv::type_t              type;
            u8                      data[0];
        };

        struct arg_list_t final {
            u16                     leaf;
            u32                     count;
            cv::type_t              arg[0];
        };
        using substr_list_t         = arg_list_t;

        struct derived_t final {
            u16                     leaf;
            u32                     count;
            cv::type_t              indices[0];
        };

        struct default_arg_t final {
            u16                     leaf;
            cv::type_t              type;
            u8                      expr[0];
        };

        struct field_list_t final {
            u16                     leaf;
            u8                      data[0];
        };

        struct method_list_t final {
            u16                     leaf;
            u8                      ml_method[0];
        };

        struct bit_field_t final {
            u16                     leaf;
            cv::type_t              type;
            u8                      length;
            u8                      position;
        };

        struct ref_sym_t final {
            u16                     leaf;
            u8                      sym[1];
        };

        struct hlsl_t final {
            u16                     leaf;
            cv::type_t              sub_type;
            u16                     kind;
            u16                     num_props:      4;
            u16                     unused:         12;
            u8                      data[0];
        };

        struct modifier_ex_t final {
            u16                     leaf;
            cv::type_t              type;
            u16                     count;
            u16                     mods[0];
        };

        struct s8_t final {
            u16                     leaf;
            s8                      val;
        };

        struct s16_t final {
            u16                     leaf;
            s16                     val;
        };

        struct u16_t final {
            u16                     leaf;
            u16                     val;
        };

        struct s32_t final {
            u16                     leaf;
            s32                     val;
        };

        struct u32_t final {
            u16                     leaf;
            u32                     val;
        };

        struct s64_t final {
            u16                     leaf;
            s64                     val;
        };

        struct u64_t final {
            u16                     leaf;
            u64                     val;
        };

        struct s128_t final {
            u16                     leaf;
            s128                    val;
        };

        struct u128_t final {
            u16                     leaf;
            u128                    val;
        };

        struct f32_t final {
            u16                     leaf;
            f32                     val;
        };

        struct f64_t final {
            u16                     leaf;
            f64                     val;
        };

        struct index_t final {
            u16                     leaf;
            u16                     pad0;
            cv::type_t              index;
        };

        struct base_class_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u8                      offset[0];
        };
        using base_interface_t      = base_class_t;

        struct vbase_class_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            cv::type_t              vb_ptr;
            u8                      vb_ptr_off[0];
        };
        using vbase_interface_t     = vbase_class_t;

        struct friend_class_t final {
            u16                     leaf;
            u16                     pad0;
            cv::type_t              index;
        };

        struct friend_func_t final {
            u16                     leaf;
            u16                     pad0;
            cv::type_t              index;
            u8                      name[1];
        };

        struct member_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u8                      offset[0];
        };

        struct static_member_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u8                      name[1];
        };

        struct vfunc_table_t final {
            u16                     leaf;
            u16                     pad0;
            cv::type_t              type;
        };

        struct vfunc_offset_t final {
            u16                     leaf;
            u16                     pad0;
            s32                     offset;
        };

        struct method_t final {
            u16                     leaf;
            u16                     count;
            cv::type_t              method_idx;
            u8                      name[1];
        };

        struct one_method_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u32                     vbase_offset[0];
        };

        struct enumerate_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            u8                      value[0];
        };

        struct nest_type_t final {
            u16                     leaf;
            u16                     pad0;
            cv::type_t              index;
            u8                      name[1];
        };

        struct nest_type_ex_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u8                      name[1];
        };

        struct member_modify_t final {
            u16                     leaf;
            cv::field_attr_t        attr;
            cv::type_t              index;
            u8                      name[1];
        };

        struct type_server2_t final {
            u16                     leaf;
            uuid_t                  sig70;
            u32                     age;
            u8                      name[0];
        };
    }

    namespace sym {
        struct reg_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u16                     reg;
            u8                      name[1];
        };

        struct attr_reg_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            lvar_attr_t             attr;
            u16                     reg;
            u8                      name[1];
        };

        struct many_reg_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u8                      count;
            u8                      reg[1];
        };

        struct many_reg2_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u16                     count;
            u16                     reg[1];
        };

        struct attr_many_reg_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            lvar_attr_t             attr;
            u8                      count;
            u8                      reg[1];
            u8                      name[0];
        };

        struct attr_many_reg2_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            lvar_attr_t             attr;
            u16                     count;
            u16                     reg[1];
            u8                      name[0];
        };

        struct const_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u16                     value;
            u8                      name[0];
        };

        struct udt_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u8                      name[1];
        };

        struct type_ref_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
        };

        struct search_t final {
            u16                     len;
            u16                     type;
            u32                     start_sym;
            u16                     seg;
        };

        struct cflag_t final {
            u16                     len;
            u16                     type;
            u8                      machine;
            struct {
                u8                  language:       8;
                u8                  pcode:          1;
                u8                  float_prec:     2;
                u8                  float_pkg:      2;
                u8                  amb_data:       3;
                u8                  amb_code:       3;
                u8                  mode32:         1;
                u8                  pad:            4;
            }                       flags;
            u8                      ver[1];
        };

        struct compile2_t final {
            u16                     len;
            u16                     type;
            struct {
                u32                 language:       8;
                u32                 ec:             1;
                u32                 no_db_info:     1;
                u32                 ltcg:           1;
                u32                 no_data_align:  1;
                u32                 managed_present:1;
                u32                 security_chk:   1;
                u32                 hot_patch:      1;
                u32                 cvtcil:         1;
                u32                 msil_module:    1;
                u32                 pad:            15;
            }                       flags;
            u16                     machine;
            u16                     ver_fe_major;
            u16                     ver_fe_minor;
            u16                     ver_fe_build;
            u16                     ver_major;
            u16                     ver_minor;
            u16                     ver_build;
            u8                      ver_str[1];
        };

        struct compile3_t final {
            u16                     len;
            u16                     type;
            struct {
                u32                 language:       8;
                u32                 ec:             1;
                u32                 no_db_info:     1;
                u32                 ltcg:           1;
                u32                 no_data_align:  1;
                u32                 managed_present:1;
                u32                 security_chk:   1;
                u32                 hot_patch:      1;
                u32                 cvtcil:         1;
                u32                 msil_module:    1;
                u32                 sdl:            1;
                u32                 pgo:            1;
                u32                 exp:            1;
                u32                 pad:            12;
            }                       flags;
            u16                     machine;
            u16                     ver_fe_major;
            u16                     ver_fe_minor;
            u16                     ver_fe_build;
            u16                     ver_major;
            u16                     ver_minor;
            u16                     ver_build;
            u16                     ver_qfe;
            s8                      ver_str[1];
        };

        struct env_block_t final {
            u16                     len;
            u16                     type;
            struct {
                u8                  rev:            1;
                u8                  pad:            7;
            }                       flags;
            u8                      str[1];
        };

        struct obj_name_t final {
            u16                     len;
            u16                     type;
            u32                     signature;
            u8                      name[1];
        };

        struct end_arg_t final {
            u16                     len;
            u16                     type;
        };

        struct return_t final {
            u16                     len;
            u16                     type;
            generic_flag_t          flags;
            u8                      style;
        };

        struct entry_this_t final {
            u16                     len;
            u16                     type;
            u8                      this_sym;
        };

        struct bp_rel_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            type_t                  type_index;
            u8                      name[1];
        };

        struct frame_rel_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            type_t                  type_index;
            lvar_attr_t             attr;
            u8                      name[1];
        };

        struct slot_t final {
            u16                     len;
            u16                     type;
            u32                     slot;
            type_t                  type_index;
            u8                      name[1];
        };

        struct attr_slot_t final {
            u16                     len;
            u16                     type;
            u32                     slot;
            type_t                  type_index;
            lvar_attr_t             attr;
            u8                      name[1];
        };

        struct annotation_t final {
            u16                     len;
            u16                     type;
            u32                     offset;
            u16                     seg;
            u16                     csz;
            u8                      str[1];
        };

        struct data_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct public_t final {
            u16                     len;
            u16                     type;
            pub_flags_t             flags;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct proc_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            u32                     next;
            u32                     proc_len;
            u32                     dbg_start;
            u32                     dbg_end;
            type_t                  type_index;
            u32                     offset;
            u16                     seg;
            proc_flags_t            flags;
            u8                      name[1];
        };

        struct man_proc_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            u32                     next;
            u32                     proc_len;
            u32                     dbg_start;
            u32                     dbg_end;
            u32                     token;
            u32                     offset;
            u16                     seg;
            proc_flags_t            flags;
            u16                     ret_reg;
            u8                      name[1];
        };

        struct thunk_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            u32                     next;
            u32                     offset;
            u16                     reg;
            u16                     thunk_len;
            u8                      ord;
            u8                      name[1];
            u8                      variant[0];
        };

        struct trampoline_t final {
            u16                     len;
            u16                     type;
            u16                     tramp_type;
            u16                     thunk_size;
            u32                     thunk_offset;
            u32                     thunk_target;
            u16                     thunk_sect;
            u16                     target_sect;
        };

        struct label_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            u32                     block_len;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct thread_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct ref2_t final {
            u16                     len;
            u16                     type;
            u32                     sum_name;
            u32                     offset;
            u16                     module_idx;
            u8                      name[1];
        };

        struct frame_proc_t final {
            u16                     len;
            u16                     type;
            u32                     frame_size;
            u32                     pad_size;
            u32                     offset_pad;
            u32                     save_reg_size;
            u32                     except_handler_offset;
            u16                     except_handler_sect;
            struct {
                u32                 has_alloca:     1;
                u32                 has_set_jmp:    1;
                u32                 has_long_jmp:   1;
                u32                 has_inl_asm:    1;
                u32                 has_eh:         1;
                u32                 inl_spec:       1;
                u32                 has_seh:        1;
                u32                 naked:          1;
                u32                 security_checks:1;
                u32                 async_eh:       1;
                u32                 gs_no_stack_ord:1;
                u32                 was_inlined:    1;
                u32                 gs_check:       1;
                u32                 safe_buffers:   1;
                u32                 enc_local_bp:   1;
                u32                 enc_param_bp:   1;
                u32                 pogo_on:        1;
                u32                 valid_counts:   1;
                u32                 opt_speed:      1;
                u32                 guard_cf:       1;
                u32                 guard_cfw:      1;
                u32                 pad:            9;
            }                       flags;
        };

        struct block_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            u32                     block_len;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct export_t final {
            u16                     len;
            u16                     type;
            u16                     ordinal;
            u16                     is_const:       1;
            u16                     data:           1;
            u16                     is_private:     1;
            u16                     no_name:        1;
            u16                     has_ordinal:    1;
            u16                     forwarder:      1;
            u16                     reserved:       10;
            u8                      name[1];
        };

        struct inline_site_t final {
            u16                     len;
            u16                     type;
            u32                     parent;
            u32                     end;
            item_id_t               inlinee;
            u8                      data[0];
        };

        struct frame_cookie_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            u16                     reg;
            cookie_type_t           cookie_type;
            u8                      flags;
        };

        struct heap_alloc_site_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            u16                     sect;
            u16                     size;
            type_t                  type_index;
        };

        struct call_site_info_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            u16                     sect;
            u16                     size;
            type_t                  type_index;
        };

        struct local_t final {
            u16                     len;
            u16                     type;
            type_t                  type_index;
            lvar_flags_t            flags;
            u8                      name[0];
        };

        struct reg_rel_t final {
            u16                     len;
            u16                     type;
            u32                     offset;
            u16                     reg;
            u16                     type_index;
            u8                      name[1];
        };

        struct section_t final {
            u16                     len;
            u16                     type;
            u16                     sect_num;
            u8                      align;
            u8                      reserved;
            u32                     rva;
            u32                     size;
            u32                     characteristics;
            u8                      name[1];
        };

        struct unamespace_t final {
            u16                     len;
            u16                     type;
            u8                      name[1];
        };

        struct coff_group_t final {
            u16                     len;
            u16                     type;
            u32                     size;
            u32                     characteristics;
            u32                     offset;
            u16                     seg;
            u8                      name[1];
        };

        struct file_static_t final {
            u16                     len;
            u16                     type;
            type_t                  index;
            u32                     module_offset;
            lvar_flags_t            flags;
            u8                      name[0];
        };

        struct defrange_t final {
            u16                     len;
            u16                     type;
            u32                     program;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct build_info_t final {
            u16                     len;
            u16                     type;
            item_id_t               id;
        };

        struct defrange_reg_t final {
            u16                     len;
            u16                     type;
            u16                     reg;
            range_attr_t            attr;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct defrange_reg_rel_t final {
            u16                     len;
            u16                     type;
            u16                     base_reg;
            u16                     spilled_udt_member: 1;
            u16                     padding:            3;
            u16                     offset_parent:      12;
            s32                     offset_base_ptr;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct defrange_subfield_t final {
            u16                     len;
            u16                     type;
            u32                     program;
            u32                     offset_parent;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct defrange_frameptr_rel_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct defrange_subfield_reg_t final {
            u16                     len;
            u16                     type;
            u16                     reg;
            range_attr_t            attr;
            u32                     offset_parent:  12;
            u32                     pad:            20;
            lvar_addr_range_t       range;
            lvar_addr_gap_t         gaps[0];
        };

        struct function_list_t final {
            u16                     len;
            u16                     type;
            u32                     count;
            type_t                  funcs[0];
        };

        struct defrange_frameptr_rel_full_scope_t final {
            u16                     len;
            u16                     type;
            s32                     offset;
        };

        str::slice_t name(u16 type);
    }

    struct cv_t final {
        alloc_t*                alloc;
        u32                     signature;
        struct {
            u32                 utf8_symbols:       1;
            u32                 pad:                31;
        }                       flags;
    };

    status_t free(cv_t& cv);

    str::slice_t sig_name(u32 sig);

    str::slice_t machine_name(u16 machine);

    str::slice_t language_name(u8 language);

    status_t init(cv_t& cv, alloc_t* alloc = context::top()->alloc);

    str::slice_t debug_subsection_name(debug_subsection_type_t type);

    status_t read_type_data(cv_t& cv, io::file_t& file, u32 offset, u32 size);

    status_t read_symbol_data(cv_t& cv, io::file_t& file, u32 offset, u32 size);
}
