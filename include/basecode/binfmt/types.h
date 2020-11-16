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
#include <basecode/core/intern.h>
#include <basecode/core/hashtab.h>
#include <basecode/binfmt/configure.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::binfmt {
    struct reloc_t;
    struct group_t;
    struct module_t;
    struct symbol_t;
    struct import_t;
    struct member_t;
    struct section_t;
    struct version_t;
    struct symbol_offs_t;

    using import_id             = u32;
    using symbol_id             = u32;
    using module_id             = u32;
    using member_id             = u32;
    using section_id            = u32;

    using id_list_t             = array_t<u32>;
    using reloc_list_t          = array_t<reloc_t>;
    using group_list_t          = array_t<group_t>;
    using import_list_t         = array_t<import_t>;
    using symbol_list_t         = array_t<symbol_t>;
    using member_list_t         = array_t<member_t>;
    using section_list_t        = array_t<section_t>;
    using section_ptr_list_t    = array_t<section_t*>;
    using symbol_offs_list_t    = array_t<symbol_offs_t>;

    enum class status_t : u32 {
        ok                              = 0,
        read_error                      = 2000,
        write_error                     = 2001,
        import_failure                  = 2002,
        duplicate_import                = 2003,
        duplicate_symbol                = 2004,
        symbol_not_found                = 2005,
        config_eval_error               = 2006,
        invalid_section_type            = 2007,
        container_init_error            = 2008,
        invalid_container_type          = 2009,
        spec_section_custom_name        = 2010,
        not_implemented                 = 2011,
        invalid_machine_type            = 2012,
        invalid_output_type             = 2013,
        cannot_map_section_name         = 2014,
        section_not_found               = 2015,
        invalid_input_type              = 2016,
        not_ar_long_name                = 2017,
        section_entry_out_of_bounds     = 2018,
        bad_cv_signature                = 2019,
        missing_linked_section          = 2020,
        custom_section_missing_symbol   = 2021,
        invalid_file_type               = 2022,
        elf_unsupported_section         = 2023,
        invalid_relocation_type         = 2024,
    };

    enum class string_table_mode_t : u8 {
        shared,
        exclusive,
    };

    enum class module_type_t : u8 {
        archive,
        object,
    };

    namespace symbol {
        enum class type_t : u8 {
            none,
            tls,
            file,
            object,
            common,
            section,
            function,
        };

        enum class scope_t : u8 {
            none,
            local,
            global,
            weak
        };

        enum class visibility_t : u8 {
            default_,
            internal_,
            hidden,
            protected_,
        };
    }

    namespace machine {
        enum class type_t : u32 {
            unknown,
            x86_64,
            aarch64,
        };

        namespace x86_64::reloc {
            enum class type_t : u8 {
                none,                   // COFF: absolute
                direct_64,              // COFF: addr64
                pc_rel_32,              // COFF: rel32 - rel32_5
                got_32,
                plt_32,
                copy,
                glob_dat,
                jump_slot,
                relative,
                got_pc_rel,
                direct_32,
                direct_32_signed,
                direct_16,
                pc_rel_16_signed,
                direct_8_signed,
                pc_rel_8_signed,
                dtp_mod_64,
                dtp_offset_64,
                tp_offset_64,
                tls_gd_signed,
                tls_ld_signed,
                dtp_offset_32,
                got_tp_offset_signed,
                tp_offset_32,
                pc_rel_64,
                got_pc_32,
                got_entry_64,
                got_offset_64,
                got_pc_rel_64,
                got_pc_64,
                got_pc_rel_32_signed,
                got_pc_rel_offset,
                got_plt_64,
                plt_offset_64,
                size_sym_32,
                size_sym_64,
                got_pc_32_tls_desc,
                tls_desc_call,
                tls_desc,
                i_relative,
                relative_64,
                got_pc_relx,
                rex_got_pc_relx,
            };
        }

        namespace aarch64::reloc {
            enum class type_t : u8 {
                none,
                abs_64,
                abs_32,
                abs_16,
                pc_rel_64,
                pc_rel_32,
                pc_rel_16,
                movw_uabs_g0,
                movw_uabs_g0_nc,
                movw_uabs_g1,
                movw_uabs_g1_nc,
                movw_uabs_g2,
                movw_uabs_g2_nc,
                movw_uabs_g3,
                movw_sabs_g0,
                movw_sabs_g1,
                movw_sabs_g2,
                ld_pc_rel_lo_19,
                adr_pc_rel_lo21,
                adr_pc_rel_pg_hi_21,
                adr_pc_rel_pg_hi_21_nc,
                add_abs_lo_12_nc,
                ldst_8_abs_lo_12_nc,
                tst_br_14,
                cond_br_19,
                jump_26,
                call_26,
                ldst_16_abs_lo_12_nc,
                ldst_32_abs_lo_12_nc,
                ldst_64_abs_lo_12_nc,
                movw_pc_rel_g0,
                movw_pc_rel_g0_nc,
                movw_pc_rel_g1,
                movw_pc_rel_g1_nc,
                movw_pc_rel_g2,
                movw_pc_rel_g2_nc,
                movw_pc_rel_g3,
                ldst_128_abs_lo12_nc,
                movw_gotoff_g0,
                movw_gotoff_g0_nc,
                movw_gotoff_g1,
                movw_gotoff_g1_nc,
                movw_gotoff_g2,
                movw_gotoff_g2_nc,
                movw_gotoff_g3,
                got_rel_64,
                got_rel_32,
                got_ld_pc_rel_19,
                ld_64_got_off_lo_15,
                adr_got_page,
                ld_64_got_lo12_nc,
                ld_64_got_page_lo_15,
                tls_gd_adr_pc_rel_21,
                tls_gd_adr_page_21,
                tls_gd_add_lo12_nc,
                tls_gd_movw_g1,
                tls_gd_movw_g0_nc,
                tls_ld_pc_rel_19,
                tls_ld_movw_dtp_rel_g2,
                tls_ld_movw_dtp_rel_g1,
                tls_ld_movw_dtp_rel_g1_nc,
                tls_ld_movw_dtp_rel_g0,
                tls_ld_movw_dtp_rel_g0_nc,
                tls_add_dtp_rel_hi_12,
                tls_add_dtp_rel_lo_12,
                tls_add_dtp_rel_lo_12_nc,
                tls_ldst_8_dtp_rel_lo_12,
                tls_ldst_8_dtp_rel_lo_12_nc,
                tls_ldst_16_dtp_rel_lo_12,
                tls_ldst_16_dtp_rel_lo_12_nc,
                tls_ldst_32_dtp_rel_lo_12,
                tls_ldst_32_dtp_rel_lo_12_nc,
                tls_ldst_64_dtp_rel_lo_12,
                tls_ldst_64_dtp_rel_lo_12_nc,
                tls_ie_movw_got_tp_rel_g1,
                tls_ie_movw_got_tp_rel_g0_nc,
                tls_ie_adr_got_tp_rel_page_21,
                tls_ie_ld_64_got_tp_rel_lo_12_nc,

                // XXX: holy fuck, AArch64 has tons of TLS relocs
                //      finish up to TLS_D_LDST128_DTPREL_LO12_NC

                copy,
                glob_dat,
                jump_slot,
                relative,
                tls_dtp_mod_64,
                tls_dtp_rel64,
                tls_tp_rel_64,
                tls_desc,
                i_relative,
            };
        }
    }

    namespace section {
        enum class type_t : u8 {
            code,
            data,
            rsrc,
            note,
            init,
            fini,
            debug,
            group,
            reloc,
            strtab,
            symtab,
            import,
            unwind,
            custom,
            export_,
            pre_init,
        };

        struct flags_t final {
            u32                 code:       1;
            u32                 init:       1;
            u32                 exec:       1;
            u32                 write:      1;
            u32                 alloc:      1;
            u32                 group:      1;
            u32                 merge:      1;
            u32                 strings:    1;
            u32                 exclude:    1;
            u32                 tls:        1;
            u32                 pad:        22;
        };
    }

    struct symbol_t final {
        u64                     size;
        u64                     value;
        symbol_id               id;
        symbol_id               next;
        section_id              section;
        u32                     name_offset;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;

        b8 operator==(const symbol_t& other) const {
            return id == other.id;
        }
    };

    struct symbol_opts_t final {
        u64                     size;
        u64                     value;
        u32                     name_offset;
        section_id              section;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;
    };

    struct symbol_offs_t final {
        symbol_id               symbol;
        u32                     offset;
    };

    struct import_t final {
        id_list_t               symbols;
        symbol_id               module_symbol;
        section_id              section;
        import_id               id;
        struct {
            u32                 load:   1;
            u32                 pad:    31;
        }                       flags;
    };

    struct group_t final {
        u32                     flags;
        id_list_t               sections;
    };

    struct reloc_t final {
        u64                     offset;
        s64                     addend;
        symbol_id               symbol;
        union {
            machine::x86_64::reloc::type_t  x86_64_type;
            machine::aarch64::reloc::type_t aarch64_type;
        };
    };

    struct symbol_table_t final {
        alloc_t*                alloc;
        hashtab_t<u32, u32>     index;
        symbol_list_t           symbols;
    };

    struct string_table_t final {
        alloc_t*                alloc;
        struct {
            u8*                 data;
            u32                 size;
            u32                 capacity;
        }                       buf;
        struct {
            u32*                data;
            u32                 size;
            u32                 capacity;
        }                       offs;
        string_table_mode_t     mode;

        const s8* operator[](u32 idx) const {
            return (const s8*) (buf.data + offs.data[idx]);
        }
    };

    union section_subclass_t {
        const u8*               data;
        group_t                 group;
        reloc_list_t            relocs;
        string_table_t          strtab;
        symbol_table_t          symtab;
        import_list_t           imports;
    };

    struct section_t final {
        alloc_t*                alloc;
        const module_t*         module;
        section_subclass_t      subclass;
        section_id              id;
        section_id              link;
        u32                     info;
        u32                     size;
        u32                     align;
        u32                     ext_type;
        u32                     name_offset;
        section::flags_t        flags;
        section::type_t         type;
    };

    struct section_opts_t final {
        section_id              link;
        section::flags_t        flags;
        u32                     info;
        u32                     size;
        u32                     align;
        u32                     ext_type;
        u32                     name_offset;
        struct {
            u8*                 buf;
            u32                 size_in_bytes;
        }                       strtab;
    };

    enum class member_type_t : u8 {
        module,
        raw
    };

    union member_subclass_t final {
        str::slice_t            raw;
        module_t*               module;
    };

    struct member_t final {
        str::slice_t            name;
        member_subclass_t       subclass;
        member_id               id;
        member_type_t           type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    union module_subclass_t final {
        struct {
            section_list_t      sections;
            section_id          strtab;
            section_id          symtab;
        }                       object;
        struct {
            member_list_t       members;
            symbol_offs_list_t  offsets;
        }                       archive;
    };

    struct module_t final {
        alloc_t*                alloc;
        module_subclass_t       subclass;
        module_id               id;
        module_type_t           type;
    };

    struct result_t final {
        u32                     id;
        status_t                status;
    };
}
