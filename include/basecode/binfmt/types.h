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

#include <basecode/core/buf.h>
#include <basecode/core/intern.h>
#include <basecode/core/bitset.h>
#include <basecode/core/hashtab.h>
#include <basecode/binfmt/configure.h>
#include <basecode/core/memory/system/slab.h>

#define FLAG_CHK(v, f)              (((v) & u32((f))) == f)

#define ELF64_R_SYM(i)              ((i) >> u32(32))
#define ELF64_R_TYPE(i)             ((i) & u32(0xffffffff))
#define ELF64_R_INFO(sym, type)     ((((u64) (sym)) << u32(32)) + (type))
#define ELF64_ST_BIND(val)          (((u8) (val)) >> u32(4))
#define ELF64_ST_TYPE(val)          ((val) & u32(0xf))
#define ELF64_ST_VISIBILITY(o)      ((o) & u32(0x03))
#define ELF64_ST_INFO(bind, type)   (((bind) << u32(4)) + ((type) & u32(0xf)))

namespace basecode::binfmt {
    struct reloc_t;
    struct group_t;
    struct module_t;
    struct symbol_t;
    struct import_t;
    struct member_t;
    struct section_t;
    struct version_t;
    enum class status_t : u32;

    struct file_t;
    struct session_t;
    struct name_map_t;

    struct coff_sym_t;
    struct coff_reloc_t;
    struct coff_unwind_t;
    struct coff_header_t;
    struct coff_line_num_t;

    struct pe_thunk_t;
    struct pe_reloc_t;
    struct pe_res_data_t;
    struct pe_name_hint_t;
    struct pe_res_entry_t;
    struct pe_export_addr_t;
    struct pe_import_module_t;

    using module_id             = u32;
    using member_id             = u32;
    using id_array_t            = array_t<u32>;
    using reloc_array_t         = array_t<reloc_t>;
    using group_array_t         = array_t<group_t>;
    using sym_hashtab_t         = hashtab_t<u32, symbol_t*>;
    using import_array_t        = array_t<import_t>;
    using member_array_t        = array_t<member_t>;
    using sym_ptr_array_t       = array_t<symbol_t*>;
    using ar_index_table_t      = hashtab_t<str::slice_t, u32>;
    using member_ptr_array_t    = array_t<member_t*>;
    using section_ptr_array_t   = array_t<section_t*>;

    using coff_sym_array_t      = array_t<coff_sym_t>;
    using coff_str_array_t      = array_t<str::slice_t>;
    using coff_header_array_t   = array_t<coff_header_t>;

    using name_array_t          = array_t<name_map_t>;
    using file_array_t          = array_t<file_t>;
    using fini_callback_t       = u0 (*)();
    using init_callback_t       = status_t (*)(alloc_t*);
    using read_callback_t       = status_t (*)(file_t&);
    using write_callback_t      = status_t (*)(file_t&);

    using pe_reloc_array_t         = array_t<pe_reloc_t>;
    using pe_res_data_array_t      = array_t<pe_res_data_t>;
    using pe_res_entry_array_t     = array_t<pe_res_entry_t>;
    using pe_name_hint_array_t     = array_t<pe_name_hint_t>;
    using pe_export_addr_array_t   = array_t<pe_export_addr_t>;
    using pe_tls_callback_array_t  = array_t<u64>;
    using pe_import_module_array_t = array_t<pe_import_module_t>;

    enum class status_t : u32 {
        ok                      = 0,
        read_error              = 20000,
        write_error,
        import_failure,
        duplicate_import,
        duplicate_symbol,
        symbol_not_found,
        config_eval_error,
        invalid_section_type,
        container_init_error,
        invalid_container_type,
        spec_section_custom_name,
        not_implemented,
        invalid_machine_type,
        invalid_output_type,
        cannot_map_section_name,
        section_not_found,
        invalid_input_type,
        not_ar_long_name,
        section_entry_out_of_bounds,
        bad_cv_signature,
        missing_linked_section,
        custom_section_missing_symbol,
        invalid_file_type,
        elf_unsupported_section,
        invalid_relocation_type,
        cannot_add_member_to_object     // FIXME
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

    namespace section {
        enum class type_t : u8 {
            none,
            bss,
            text,
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
            u32                 tls:        1;
            u32                 exec:       1;
            u32                 write:      1;
            u32                 alloc:      1;
            u32                 group:      1;
            u32                 merge:      1;
            u32                 strings:    1;
            u32                 exclude:    1;
            u32                 dynamic:    1;
            u32                 pad:        23;
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

    enum class format_type_t : u8 {
        none,
        ar,
        pe,
        elf,
        coff,
        macho
    };

    enum pe_dir_type_t : u8 {
        reserved                = 15,
        tls_table               = 9,
        com_header              = 14,
        debug_table             = 6,
        gp_register             = 8,
        export_table            = 0,
        import_table            = 1,
        resource_table          = 2,
        exception_table         = 3,
        base_reloc_table        = 5,
        certificate_table       = 4,
        load_config_table       = 10,
        bound_import_table      = 11,
        architecture_table      = 7,
        import_address_table    = 12,
        delay_import_descriptor = 13,
    };

    enum class file_type_t : u8 {
        none,
        obj,
        exe,
        dll
    };

    enum class coff_sym_type_t : u8 {
        sym,
        aux_xf,
        aux_file,
        aux_section,
        aux_func_def,
        aux_token_def,
        aux_weak_extern,
    };

    enum class module_type_t : u8 {
        archive,
        object,
    };

    enum class string_table_mode_t : u8 {
        shared,
        exclusive,
    };

    constexpr u32 max_type_count        = 6;
    constexpr u32 max_dir_entry_count   = 16;

    struct coff_rva_t final {
        u32                     base;
        u32                     size;
    };

    struct coff_raw_t final {
        u32                     offset;
        u32                     size;
    };

    // .debug$S (symbolic info)
    // .debug$T (type info)
    struct coff_debug_t final {
    };

    struct coff_reloc_t final {
        u32                     rva;
        u32                     symtab_idx;
        u16                     type;
    };

    struct coff_unwind_t final {
        u32                     begin_rva;
        u32                     end_rva;
        u32                     info;
    };

    struct coff_line_num_t final {
        union {
            u32                 rva;
            u32                 symtab_idx;
        };
        u16                     number;
    };

    struct name_flags_t final {
        u32                     code:       1;
        u32                     init:       1;
        u32                     exec:       1;
        u32                     write:      1;
        u32                     pad:        28;
    };

    struct name_map_t final {
        str::slice_t            name;
        name_flags_t            flags;
        section::type_t         type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    struct file_t final {
        session_t*              session;
        module_t*               module;
        path_t                  path;
        buf_t                   buf;
        buf_crsr_t              crsr;
        machine::type_t         machine;
        struct {
            u32                 gui:        1;
            u32                 console:    1;
            u32                 pad:        30;
        }                       flags;
        struct {
            u64                 base_addr;
            u64                 heap_reserve;
            u64                 stack_reserve;
        }                       opts;
        struct {
            version_t           linker;
            version_t           min_os;
        }                       versions;
        format_type_t           bin_type;
        file_type_t             file_type;
    };

    struct session_t final {
        alloc_t*                alloc;
        file_array_t            files;
    };

    struct io_system_t final {
        init_callback_t         init;
        fini_callback_t         fini;
        read_callback_t         read;
        write_callback_t        write;
        format_type_t           type;
    };

    struct coff_sym_t final {
        union {
            struct {
                s8              bytes[18];
            }                   aux_file;
            struct {
                u32             ptr_next_func;
                u16             line_num;
            }                   aux_xf;
            struct {
                u32             tag_idx;
                u32             flags;
            }                   aux_weak_extern;
            struct {
                u32             tag_idx;
                u32             total_size;
                u32             ptr_line_num;
                u32             ptr_next_func;
            }                   aux_func_def;
            struct {
                u32             len;
                u32             check_sum;
                u16             num_relocs;
                u16             num_lines;
                u16             sect_num;
                u8              comdat_sel;
            }                   aux_section;
            struct {
                u32             symtab_idx;
                u8              aux_type;
            }                   aux_token_def;
            struct {
                u64             strtab_offset;
                str::slice_t    name;
                struct {
                    u32         idx;
                    u32         len;
                }               aux;
                u32             value;
                u16             type;
                s16             section;
                u8              sclass;
            }                   sym;
        }                       subclass;
        u32                     id;
        coff_sym_type_t         type;
    };

    struct coff_header_t final {
        const section_t*        section;
        u64                     short_name;
        str::slice_t            name;
        u32                     symbol;
        coff_rva_t              rva;
        coff_raw_t              file;
        struct {
            coff_raw_t          file;
        }                       relocs;
        struct {
            coff_raw_t          file;
        }                       line_nums;
        u32                     number;
        u32                     flags;
    };

    struct coff_t final {
        alloc_t*                alloc;
        u8*                     buf;
        coff_header_array_t     headers;
        u32                     rva;
        u32                     offset;
        u32                     timestamp;
        struct {
            coff_raw_t          file;
            coff_str_array_t    list;
        }                       strtab;
        struct {
            coff_raw_t          file;
            coff_sym_array_t    list;
            u32                 num_symbols;
        }                       symtab;
        struct {
            u32                 image;
            u16                 headers;
            u16                 opt_hdr;
        }                       size;
        struct {
            u16                 image;
        }                       flags;
        struct {
            u32                 file;
            u32                 section;
        }                       align;
        coff_rva_t              code;
        coff_rva_t              relocs;
        coff_rva_t              init_data;
        coff_rva_t              uninit_data;
        u16                     machine;
    };

    struct pe_res_data_t final {
        str::slice_t            data;
        coff_rva_t              rva;
        u32                     code_page;
    };

    struct pe_res_entry_t final {
        u32                     id;
        u32                     offset;
        b8                      is_name;
        b8                      is_subdir;
    };

    struct pe_res_t final {
        pe_res_entry_array_t    entries;
        pe_res_data_array_t     blocks;
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
    };

    struct pe_tls_t final {
        pe_tls_callback_array_t callbacks;
        struct {
            u64                 start_va;
            u64                 end_va;
        }                       raw_data;
        u64                     index_addr;
        u32                     size_zero_fill;
        u32                     flags;
    };

    struct pe_thunk_t final {
        union {
            struct {
                u64             value:      63;
                u64             by_ordinal: 1;
            }                   thunk;
            u64                 bits;
        };
    };

    struct pe_debug_t final {
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
        u32                     type;
        coff_rva_t              mem;
        coff_raw_t              file;
    };

    struct pe_load_cfg_t final {
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
        struct {
            u32                 clear;
            u32                 set;
        }                       global_flags;
        u32                     critical_section_default_timeout;
        struct {
            u64                 free;
            u64                 total;
        }                       de_commit_threshold;
        // x86 only
        //u64                     lock_prefix_table;
        u64                     maximum_allocation_size;
        u64                     virtual_memory_threshold;
        u64                     processor_affinity_mask;
        u32                     process_heap_flags;
        u16                     csd_version;
        // write only
        //u16                     reserved;
        //u64                     edit_list;
        u64                     security_cookie;
        // x86 only
        //u64                     se_handler_table;
        //u64                     se_handler_count;
        u64                     guard_cf_check_function_ptr;
        u64                     guard_cf_dispatch_function_ptr;
        struct {
            u64                 addr;
            u64                 size;
        }                       guard_cf_function;
        u32                     guard_flags;
        u8                      code_integrity[12];
        struct {
            u64                 addr;
            u64                 size;
        }                       guard_address_taken;
        struct {
            u64                 addr;
            u64                 size;
        }                       guard_long_jump_target;
    };

    struct pe_name_hint_t final {
        str::slice_t            name;
        coff_rva_t              rva;
        u16                     hint;
        b8                      pad;
    };

    struct pe_import_module_t final {
        u32                     lookup_rva;
        u32                     time_stamp;
        u32                     fwd_chain;
        struct {
            str::slice_t        slice;
            u32                 rva;
        }                       name;
        struct {
            u32                 start;
            u32                 size;
        }                       symbols;
        u32                     iat_rva;
    };

    struct pe_import_t final {
        pe_import_module_array_t   modules;
        struct {
            pe_name_hint_array_t   list;
            coff_rva_t          rva;
        }                       name_hints;
    };

    struct pe_export_addr_t final {
        struct {
            str::slice_t        slice;
            coff_rva_t          rva;
        }                       name;
        u32                     rva;
        u16                     ordinal;
    };

    struct pe_export_t final {
        pe_export_addr_array_t  exports;
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
        struct {
            str::slice_t        name;
            coff_rva_t          rva;
        }                       module;
        struct {
            coff_rva_t          table;
            u32                 base;
        }                       ordinal;
        struct {
            coff_rva_t          table;
            coff_rva_t          pointers;
        }                       name;
        coff_rva_t              table;
    };

    struct pe_reloc_t final {
        u16                     offset: 12;
        u16                     type:   4;
    };

    struct pe_base_reloc_t final {
        pe_reloc_array_t        relocs;
        coff_rva_t              page;
    };

    struct pe_dir_t final {
        union {
            pe_tls_t            tls;
            pe_res_t            resources;
            pe_import_t         import;
            pe_export_t         export_;
            pe_load_cfg_t       load_cfg;
            pe_base_reloc_t     base_reloc;
        }                       subclass;
        coff_rva_t              rva;
        pe_dir_type_t           type;
        b8                      init;
    };

    struct pe_t final {
        coff_t                  coff;
        u64                     base_addr;
        struct {
            u64                 heap;
            u64                 stack;
        }                       reserve;
        struct {
            u32                 dos_stub;
        }                       size;
        struct {
            u16                 dll;
            u32                 load;
        }                       flags;
        struct {
            u32                 include_symbol_table:   1;
            u32                 pad:                    31;
        }                       opts;
        u32                     start_offset;
        pe_dir_t                dirs[max_dir_entry_count];
    };

    struct pe_opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     base_addr;
        u64                     heap_reserve;
        u64                     stack_reserve;
    };

    struct elf_file_header_t final {
        u8                      magic[16];
        u16                     type;
        u16                     machine;
        u32                     version;
        u64                     entry_point;
        u64                     pgm_hdr_offset;
        u64                     sect_hdr_offset;
        u32                     flags;
        u16                     header_size;
        u16                     pgm_hdr_size;
        u16                     pgm_hdr_count;
        u16                     sect_hdr_size;
        u16                     sect_hdr_count;
        u16                     strtab_ndx;
    };

    struct elf_pgm_header_t final {
        u32                     type;
        u32                     flags;
        u64                     offset;
        u64                     virt_addr;
        u64                     phys_addr;
        u64                     file_size;
        u64                     mem_size;
        u64                     align;
    };

    struct elf_sect_header_t final {
        u32                     name_offset;
        u32                     type;
        u64                     flags;
        u64                     addr;
        u64                     offset;
        u64                     size;
        u32                     link;
        u32                     info;
        u64                     addr_align;
        u64                     entity_size;
    };

    struct elf_ver_t final {
        u16                     version;
        u16                     flags;
        u16                     index;
        u16                     aux_count;
        u32                     hash;
        u32                     aux_offset;
        u32                     next_offset;
    };

    struct elf_var_aux_t final {
        u32                     name;
        u32                     next_offset;
    };

    struct elf_ver_need_t final {
        u16                     version;
        u16                     aux_count;
        u32                     file_offset;
        u32                     aux_offset;
        u32                     next_offset;
    };

    struct elf_ver_need_aux_t final {
        u32                     hash;
        u16                     flags;
        u16                     other;
        u32                     name_offset;
        u32                     next_offset;
    };

    struct elf_sym_t final {
        u32                     name_offset;
        u8                      info;
        u8                      other;
        u16                     section_ndx;
        u64                     value;
        u64                     size;
    };

    struct elf_sym_bind_t final {
        u16                     bound_to;
        u16                     flags;
    };

    struct elf_rela_t final {
        u64                     offset;
        u64                     info;
        s64                     addend;
    };

    struct elf_note_header_t final {
        u32                     name_size;
        u32                     desc_size;
        u32                     type;
        u8                      data[0];
    };

    struct elf_dyn_t final {
        s64                     tag;
        u64                     value;
    };

    struct elf_group_t final {
        u32                     flags;
        u32                     sect_hdr_indexes[0];
    };

    struct elf_opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     entry_point;
        u32                     header_offset;
        u32                     flags;
        u8                      clazz;
        u8                      os_abi;
        u8                      version;
        u8                      endianess;
        u8                      abi_version;
    };

    struct elf_t final {
        alloc_t*                alloc;
        const elf_opts_t*       opts;
        elf_file_header_t*      file_header;
        elf_pgm_header_t*       segments;
        elf_sect_header_t*      sections;
    };

    union macho_lc_str_t final {
        u32                     offset;
#ifndef __LP64__
        s8*                     name;
#endif
    };

    struct macho_fat_header_t final {
        u32                     magic;
        u32                     num_fat_arch;
    };

    struct macho_fat_arch_t final {
        s32                     cpu_type;
        s32                     cpu_sub_type;
        u32                     offset;
        u32                     size;
        u32                     align;
    };

    struct macho_file_header_t final {
        u32                     magic;
        s32                     cpu_type;
        s32                     cpu_sub_type;
        u32                     file_type;
        u32                     num_load_commands;
        u32                     size_of_commands;
        u32                     flags;
        u32                     reserved;
    };

    struct macho_load_cmd_t final {
        u32                     cmd;
        u32                     size;
    };

    struct macho_uuid_cmd_t final {
        macho_load_cmd_t        hdr;
        u8                      uuid[16];
    };

    struct macho_rpath_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          path;
    };

    struct macho_dylib_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          name;
        u32                     timestamp;
        u32                     current_version;
        u32                     compatibility_version;
    };

    struct macho_symtab_cmd_t final {
        macho_load_cmd_t        hdr;
        u32                     sym_offs;
        u32                     num_syms;
        u32                     str_offs;
        u32                     str_size;
    };

    struct macho_segment_cmd_t final {
        macho_load_cmd_t        hdr;
        s8                      name[16];
        u64                     virt_addr;
        u64                     virt_size;
        u64                     file_offs;
        u64                     file_size;
        s32                     max_prot;
        s32                     init_prot;
        u32                     num_sects;
        u32                     flags;
    };

    struct macho_sub_library_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          sub_library;
    };

    struct macho_sub_umbrella_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          sub_umbrella;
    };

    struct macho_sub_framework_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          umbrella;
    };

    struct macho_sub_client_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          client;
    };

    struct macho_prebind_cksum_cmd_t final {
        macho_load_cmd_t        hdr;
        u32                     checksum;
    };

    struct macho_link_edit_cmd_t final {
        macho_load_cmd_t        hdr;
        u32                     data_offs;
        u32                     data_size;
    };

    struct macho_dysymtab_cmd_t final {
        macho_load_cmd_t        hdr;
        u32                     idx_local_sym;
        u32                     num_local_sym;
        u32                     idx_ext_def_sym;
        u32                     num_ext_def_sym;
        u32                     idx_undef_sym;
        u32                     num_undef_sym;
        u32                     toc_tab_offs;
        u32                     num_toc_entries;
        u32                     mod_tab_offs;
        u32                     num_mod_tab_entries;
        u32                     ext_ref_sym_offs;
        u32                     num_ext_ref_syms;
        u32                     indirect_sym_offs;
        u32                     num_indirect_syms;
        u32                     ext_reloc_offs;
        u32                     num_ext_reloc;
        u32                     local_reloc_offs;
        u32                     num_local_reloc;
    };

    struct macho_dylinker_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          name;
    };

    struct entry_point_cmd_t final {
        macho_load_cmd_t        hdr;
        u64                     file_offs;
        u64                     stack_size;
    };

    struct macho_prebound_dylib_cmd_t final {
        macho_load_cmd_t        hdr;
        macho_lc_str_t          name;
        u32                     num_modules;
        macho_lc_str_t          linked_modules;
    };

    struct macho_encryption_info_cmd_t final {
        macho_load_cmd_t        hdr;
        u32                     crypt_offs;
        u32                     crypt_size;
        u32                     crypt_id;
    };

    struct macho_section_t final {
        s8                      name[16];
        s8                      segment_name[16];
        u64                     addr;
        u64                     size;
        u32                     offset;
        u32                     align;
        u32                     rel_offs;
        u32                     num_relocs;
        u32                     flags;
        u32                     reserved1;
        u32                     reserved2;
        u32                     reserved3;
    };

    struct macho_nlist_t final {
        u32                     str_offs;
        u8                      type;
        u8                      sect_num;
        u16                     desc;
        u64                     value;
    };

    struct macho_opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     entry_point;
    };

    struct macho_t final {
        alloc_t*                alloc;
        const macho_opts_t*     opts;
        macho_file_header_t*    file_header;
    };

    namespace ar {
        constexpr u32 header_size   = 60;
    }

    namespace elf {
        constexpr u32 class_64         = 2;
        constexpr u32 data_2lsb        = 1;
        constexpr u32 data_2msb        = 2;
        constexpr u32 os_abi_sysv      = 0;
        constexpr u32 os_abi_gnu       = 3;
        constexpr u32 version_current  = 1;

        namespace note {
            constexpr u32 header_size  = sizeof(elf_note_header_t);
            constexpr u32 pr_status    = 1;
            constexpr u32 pr_fp_reg    = 2;
            constexpr u32 pr_rs_info   = 3;
            constexpr u32 task_struct  = 4;
            constexpr u32 gnu_prop     = 5;
            constexpr u32 auxv         = 6;

            constexpr u32 file         = 0x46494c45;
            constexpr u32 sig_info     = 0x53494749;
            constexpr u32 prx_fp_reg   = 0x46e62b7f;

            constexpr u32 i386_tls     = 0x200;
        }

        namespace file {
            constexpr u16 header_size          = sizeof(elf_file_header_t);
            constexpr u16 magic_class          = 4;
            constexpr u16 magic_data           = 5;
            constexpr u16 magic_version        = 6;
            constexpr u16 magic_os_abi         = 7;
            constexpr u16 magic_abi_version    = 8;

            namespace type {
                constexpr u8 none              = 0;
                constexpr u8 rel               = 1;
                constexpr u8 exec              = 2;
                constexpr u8 dyn               = 3;
                constexpr u8 core              = 4;
            }
        }

        namespace group {
            constexpr u32 entity_size          = sizeof(u32);
        }

        namespace relocs {
            constexpr u32 entity_size          = sizeof(elf_rela_t);

            namespace x86_64 {
                constexpr u32 none                 = 0;
                constexpr u32 d64                  = 1;
                constexpr u32 pc32                 = 2;
                constexpr u32 got32                = 3;
                constexpr u32 plt32                = 4;
                constexpr u32 copy                 = 5;
                constexpr u32 glob_dat             = 6;
                constexpr u32 jmp_slot             = 7;
                constexpr u32 relative             = 8;
                constexpr u32 got_pc_rel           = 9;
                constexpr u32 dir32_zx             = 10;
                constexpr u32 dir32_sx             = 11;
                constexpr u32 dir16_zx             = 12;
                constexpr u32 pc_rel_16_sx         = 13;
                constexpr u32 dir8_sx              = 14;
                constexpr u32 pc_rel_8_sx          = 15;
                constexpr u32 dtp_mod_64           = 16;
                constexpr u32 dtp_off_64           = 17;
                constexpr u32 tp_off_64            = 18;
                constexpr u32 tls_gd               = 19;
                constexpr u32 tls_ld               = 20;
                constexpr u32 dtp_off_32           = 21;
                constexpr u32 got_tp_off           = 22;
                constexpr u32 tp_off_32            = 23;
                constexpr u32 pc64                 = 24;
                constexpr u32 got_off_64           = 25;
                constexpr u32 got_pc_32            = 26;
                constexpr u32 got_entry_64         = 27;
                constexpr u32 got_pc_rel_64        = 28;
                constexpr u32 got_pc_64            = 29;
                constexpr u32 got_plt_64           = 30;
                constexpr u32 plt_off_64           = 31;
                constexpr u32 size32               = 32;
                constexpr u32 size64               = 33;
                constexpr u32 got_pc_32_tls_desc   = 34;
                constexpr u32 tls_desc_call        = 35;
                constexpr u32 tls_desc             = 36;
                constexpr u32 i_relative           = 37;
                constexpr u32 relative64           = 38;
                constexpr u32 got_pc_relx          = 41;
                constexpr u32 rex_got_pc_relx      = 42;
            }

            namespace aarch64 {
                constexpr u32 none                         = 0;
                constexpr u32 abs64                        = 257;
                constexpr u32 abs32                        = 258;
                constexpr u32 abs16                        = 259;
                constexpr u32 prel64                       = 260;
                constexpr u32 prel32                       = 261;
                constexpr u32 prel16                       = 262;
                constexpr u32 movw_uabs_g0                 = 263;
                constexpr u32 movw_uabs_g0_nc              = 264;
                constexpr u32 movw_uabs_g1                 = 265;
                constexpr u32 movw_uabs_g1_nc              = 266;
                constexpr u32 movw_uabs_g2                 = 267;
                constexpr u32 movw_uabs_g2_nc              = 268;
                constexpr u32 movw_uabs_g3                 = 269;
                constexpr u32 movw_sabs_g0                 = 270;
                constexpr u32 movw_sabs_g1                 = 271;
                constexpr u32 movw_sabs_g2                 = 272;
                constexpr u32 ld_prel_lo19                 = 273;
                constexpr u32 adr_prel_lo21                = 274;
                constexpr u32 adr_prel_pg_hi21             = 275;
                constexpr u32 adr_prel_pg_hi21_nc          = 276;
                constexpr u32 add_abs_lo12_nc              = 277;
                constexpr u32 ldst8_abs_lo12_nc            = 278;
                constexpr u32 tstbr14                      = 279;
                constexpr u32 condbr19                     = 280;
                constexpr u32 jump26                       = 282;
                constexpr u32 call26                       = 283;
                constexpr u32 ldst16_abs_lo12_nc           = 284;
                constexpr u32 ldst32_abs_lo12_nc           = 285;
                constexpr u32 ldst64_abs_lo12_nc           = 286;
                constexpr u32 movw_prel_g0                 = 287;
                constexpr u32 movw_prel_g0_nc              = 288;
                constexpr u32 movw_prel_g1                 = 289;
                constexpr u32 movw_prel_g1_nc              = 290;
                constexpr u32 movw_prel_g2                 = 291;
                constexpr u32 movw_prel_g2_nc              = 292;
                constexpr u32 movw_prel_g3                 = 293;
                constexpr u32 ldst128_abs_lo12_nc          = 299;
                constexpr u32 movw_gotoff_g0               = 300;
                constexpr u32 movw_gotoff_g0_nc            = 301;
                constexpr u32 movw_gotoff_g1               = 302;
                constexpr u32 movw_gotoff_g1_nc            = 303;
                constexpr u32 movw_gotoff_g2               = 304;
                constexpr u32 movw_gotoff_g2_nc            = 305;
                constexpr u32 movw_gotoff_g3               = 306;
                constexpr u32 gotrel64                     = 307;
                constexpr u32 gotrel32                     = 308;
                constexpr u32 got_ld_prel19                = 309;
                constexpr u32 ld64_gotoff_lo15             = 310;
                constexpr u32 adr_got_page                 = 311;
                constexpr u32 ld64_got_lo12_nc             = 312;
                constexpr u32 ld64_gotpage_lo15            = 313;
                constexpr u32 tlsgd_adr_prel21             = 512;
                constexpr u32 tlsgd_adr_page21             = 513;
                constexpr u32 tlsgd_add_lo12_nc            = 514;
                constexpr u32 tlsgd_movw_g1                = 515;
                constexpr u32 tlsgd_movw_g0_nc             = 516;
                constexpr u32 tlsld_adr_prel21             = 517;
                constexpr u32 tlsld_adr_page21             = 518;
                constexpr u32 tlsld_add_lo12_nc            = 519;
                constexpr u32 tlsld_movw_g1                = 520;
                constexpr u32 tlsld_movw_g0_nc             = 521;
                constexpr u32 tlsld_ld_prel19              = 522;
                constexpr u32 tlsld_movw_dtprel_g2         = 523;
                constexpr u32 tlsld_movw_dtprel_g1         = 524;
                constexpr u32 tlsld_movw_dtprel_g1_nc      = 525;
                constexpr u32 tlsld_movw_dtprel_g0         = 526;
                constexpr u32 tlsld_movw_dtprel_g0_nc      = 527;
                constexpr u32 tlsld_add_dtprel_hi12        = 528;
                constexpr u32 tlsld_add_dtprel_lo12        = 529;
                constexpr u32 tlsld_add_dtprel_lo12_nc     = 530;
                constexpr u32 tlsld_ldst8_dtprel_lo12      = 531;
                constexpr u32 tlsld_ldst8_dtprel_lo12_nc   = 532;
                constexpr u32 tlsld_ldst16_dtprel_lo12     = 533;
                constexpr u32 tlsld_ldst16_dtprel_lo12_nc  = 534;
                constexpr u32 tlsld_ldst32_dtprel_lo12     = 535;
                constexpr u32 tlsld_ldst32_dtprel_lo12_nc  = 536;
                constexpr u32 tlsld_ldst64_dtprel_lo12     = 537;
                constexpr u32 tlsld_ldst64_dtprel_lo12_nc  = 538;
                constexpr u32 tlsie_movw_gottprel_g1       = 539;
                constexpr u32 tlsie_movw_gottprel_g0_nc    = 540;
                constexpr u32 tlsie_adr_gottprel_page21    = 541;
                constexpr u32 tlsie_ld64_gottprel_lo12_nc  = 542;
                constexpr u32 tlsie_ld_gottprel_prel19     = 543;
                constexpr u32 tlsle_movw_tprel_g2          = 544;
                constexpr u32 tlsle_movw_tprel_g1          = 545;
                constexpr u32 tlsle_movw_tprel_g1_nc       = 546;
                constexpr u32 tlsle_movw_tprel_g0          = 547;
                constexpr u32 tlsle_movw_tprel_g0_nc       = 548;
                constexpr u32 tlsle_add_tprel_hi12         = 549;
                constexpr u32 tlsle_add_tprel_lo12         = 550;
                constexpr u32 tlsle_add_tprel_lo12_nc      = 551;
                constexpr u32 tlsle_ldst8_tprel_lo12       = 552;
                constexpr u32 tlsle_ldst8_tprel_lo12_nc    = 553;
                constexpr u32 tlsle_ldst16_tprel_lo12      = 554;
                constexpr u32 tlsle_ldst16_tprel_lo12_nc   = 555;
                constexpr u32 tlsle_ldst32_tprel_lo12      = 556;
                constexpr u32 tlsle_ldst32_tprel_lo12_nc   = 557;
                constexpr u32 tlsle_ldst64_tprel_lo12      = 558;
                constexpr u32 tlsle_ldst64_tprel_lo12_nc   = 559;
                constexpr u32 tlsdesc_ld_prel19            = 560;
                constexpr u32 tlsdesc_adr_prel21           = 561;
                constexpr u32 tlsdesc_adr_page21           = 562;
                constexpr u32 tlsdesc_ld64_lo12            = 563;
                constexpr u32 tlsdesc_add_lo12             = 564;
                constexpr u32 tlsdesc_off_g1               = 565;
                constexpr u32 tlsdesc_off_g0_nc            = 566;
                constexpr u32 tlsdesc_ldr                  = 567;
                constexpr u32 tlsdesc_add                  = 568;
                constexpr u32 tlsdesc_call                 = 569;
                constexpr u32 tlsle_ldst128_tprel_lo12     = 570;
                constexpr u32 tlsle_ldst128_tprel_lo12_nc  = 571;
                constexpr u32 tlsld_ldst128_dtprel_lo12    = 572;
                constexpr u32 tlsld_ldst128_dtprel_lo12_nc = 573;
                constexpr u32 copy                         = 1024;
                constexpr u32 glob_dat                     = 1025;
                constexpr u32 jump_slot                    = 1026;
                constexpr u32 relative                     = 1027;
                constexpr u32 tls_dtpmod64                 = 1028;
                constexpr u32 tls_dtprel64                 = 1029;
                constexpr u32 tls_tprel64                  = 1030;
                constexpr u32 tls_desc                     = 1031;
                constexpr u32 i_relative                   = 1032;
            }
        }

        namespace version {
            namespace def {
                constexpr u32 none     = 0;
                constexpr u32 current  = 1;
                constexpr u32 number   = 2;
            }

            namespace flags {
                constexpr u32 base     = 1;
                constexpr u32 weak     = 2;
            }

            namespace index {
                constexpr u32 local    = 0;
                constexpr u32 global   = 1;
            }
        }

        namespace machine {
            constexpr u16 x86_64            = 62;
            constexpr u16 aarch64           = 183;
            constexpr u16 tilera_tile_pro   = 188;
            constexpr u16 tilera_tile_gx    = 191;
            constexpr u16 riscv             = 243;
        }

        namespace dynamic {
            namespace type {
                constexpr u32 null                  = 0;
                constexpr u32 needed                = 1;
                constexpr u32 plt_rel_size          = 2;
                constexpr u32 plt_got               = 3;
                constexpr u32 hash                  = 4;
                constexpr u32 strtab                = 5;
                constexpr u32 symtab                = 6;
                constexpr u32 rela                  = 7;
                constexpr u32 rela_size             = 8;
                constexpr u32 rela_ent_size         = 9;
                constexpr u32 str_size              = 10;
                constexpr u32 sym_ent_size          = 11;
                constexpr u32 init                  = 12;
                constexpr u32 fini                  = 13;
                constexpr u32 soname                = 14;
                constexpr u32 rpath                 = 15;
                constexpr u32 symbolic              = 16;
                constexpr u32 rel                   = 17;
                constexpr u32 rel_size              = 18;
                constexpr u32 rel_ent_size          = 19;
                constexpr u32 plt_rel               = 20;
                constexpr u32 debug                 = 21;
                constexpr u32 text_rel              = 22;
                constexpr u32 jmp_rel               = 23;
                constexpr u32 bind_now              = 24;
                constexpr u32 init_array            = 25;
                constexpr u32 fini_array            = 26;
                constexpr u32 init_array_size       = 27;
                constexpr u32 fini_array_size       = 28;
                constexpr u32 run_path              = 29;
                constexpr u32 flags                 = 30;
                constexpr u32 pre_init_array        = 32;
                constexpr u32 pre_init_array_size   = 33;
                constexpr u32 low_os                = 0x6000000d;
                constexpr u32 high_os               = 0x6ffff000;
                constexpr u32 low_proc              = 0x70000000;
                constexpr u32 high_proc             = 0x7fffffff;
            }
        }

        namespace section {
            constexpr u16 header_size          = 64;

            namespace type {
                constexpr u32 null             = 0;
                constexpr u32 progbits         = 1;
                constexpr u32 symtab           = 2;
                constexpr u32 strtab           = 3;
                constexpr u32 rela             = 4;
                constexpr u32 hash             = 5;
                constexpr u32 dynamic          = 6;
                constexpr u32 note             = 7;
                constexpr u32 nobits           = 8;
                constexpr u32 rel              = 9;
                constexpr u32 shlib            = 10;
                constexpr u32 dynsym           = 11;
                constexpr u32 init_array       = 14;
                constexpr u32 fini_array       = 15;
                constexpr u32 pre_init_array   = 16;
                constexpr u32 group            = 17;
                constexpr u32 symtab_shndx     = 18;
                constexpr u32 low_os           = 0x60000000;
                constexpr u32 gnu_eh_frame     = 0x6474e550;
                constexpr u32 gnu_stack        = 0x6474e551;
                constexpr u32 gnu_rel_ro       = 0x6474e552;
                constexpr u32 gnu_attributes   = 0x6ffffff5;
                constexpr u32 gnu_hash         = 0x6ffffff6;
                constexpr u32 gnu_lib_list     = 0x6ffffff7;
                constexpr u32 checksum         = 0x6ffffff8;
                constexpr u32 gnu_ver_def      = 0x6ffffffd;
                constexpr u32 gnu_ver_need     = 0x6ffffffe;
                constexpr u32 gnu_ver_sym      = 0x6fffffff;
                constexpr u32 high_os          = 0x6fffffff;
                constexpr u32 low_proc         = 0x70000000;
                constexpr u32 x86_64_unwind    = 0x70000001;
                constexpr u32 high_proc        = 0x7fffffff;
                constexpr u32 low_user         = 0x80000000;
                constexpr u32 high_user        = 0x8fffffff;
            }

            namespace flags {
                constexpr u32 write            = 0x1;
                constexpr u32 alloc            = 0x2;
                constexpr u32 exec_instr       = 0x4;
                constexpr u32 merge            = 0x10;
                constexpr u32 strings          = 0x20;
                constexpr u32 info_link        = 0x40;
                constexpr u32 link_order       = 0x80;
                constexpr u32 os_non_conform   = 0x100;
                constexpr u32 group            = 0x200;
                constexpr u32 tls              = 0x400;
                constexpr u32 compressed       = 0x800;
                constexpr u32 mask_os          = 0x0ff00000;
                constexpr u32 mask_proc        = 0xf0000000;
                constexpr u32 ordered          = 0x40000000;
                constexpr u32 exclude          = 0x80000000;
            }

            namespace info {
            }

            namespace link {
            }

            namespace indexes {
                constexpr u32 undef            = 0;
                constexpr u32 live_abs         = 0xfff1;
                constexpr u32 live_patch       = 0xff20;
                constexpr u32 live_common      = 0xfff2;
            }
        }

        namespace segment {
            constexpr u16 header_size          = 56;

            namespace type {
                constexpr u32 null             = 0x0;
                constexpr u32 load             = 0x1;
                constexpr u32 dynamic          = 0x2;
                constexpr u32 interp           = 0x3;
                constexpr u32 note             = 0x4;
                constexpr u32 shlib            = 0x5;
                constexpr u32 pgm_hdr          = 0x6;
                constexpr u32 tls              = 0x7;
                constexpr u32 gnu_eh_frame     = 0x6474e550;
                constexpr u32 gnu_property     = 0x6474e553;
            }

            namespace flags {
                constexpr u32 read             = 0x4;
                constexpr u32 exec             = 0x1;
                constexpr u32 write            = 0x2;
            }
        }

        namespace symtab {
            constexpr u32 entity_size          = 24;

            namespace type {
                constexpr u8 notype            = 0x0;
                constexpr u8 object            = 0x1;
                constexpr u8 func              = 0x2;
                constexpr u8 section           = 0x3;
                constexpr u8 file              = 0x4;
                constexpr u8 common            = 0x5;
                constexpr u8 tls               = 0x6;
            }

            namespace scope {
                constexpr u8 local             = 0x0;
                constexpr u8 global            = 0x1;
                constexpr u8 weak              = 0x2;
            }

            namespace visibility {
                constexpr u8 default_          = 0x0;
                constexpr u8 internal          = 0x1;
                constexpr u8 hidden            = 0x2;
                constexpr u8 protected_        = 0x3;
            }
        }

        namespace hashtab {
            constexpr u32 entity_size          = sizeof(u32);
        }
    }

    namespace coff {
        constexpr u32 header_size   = 0x14;

        namespace clr {
        }

        namespace debug {
            constexpr u32 unknown   = 0;
            constexpr u32 code_view = 2;
        }

        namespace unwind {
            constexpr u32 entry_size = 12;
        }

        namespace reloc {
            constexpr u32 entry_size = 10;

            namespace type::x86_64 {
                constexpr u16 absolute              = 0;
                constexpr u16 addr64                = 1;
                constexpr u16 addr32                = 2;
                constexpr u16 addr32nb              = 3;
                constexpr u16 rel32                 = 4;
                constexpr u16 rel32_1               = 5;
                constexpr u16 rel32_2               = 6;
                constexpr u16 rel32_3               = 7;
                constexpr u16 rel32_4               = 8;
                constexpr u16 rel32_5               = 9;
                constexpr u16 section               = 10;
                constexpr u16 section_rel           = 11;
                constexpr u16 section_rel_7         = 12;
                constexpr u16 token                 = 13;
                constexpr u16 span_rel32_signed     = 14;
                constexpr u16 pair                  = 15;
                constexpr u16 span32_signed         = 16;
            }

            namespace type::aarch64 {
                constexpr u16 absolute              = 0;
                constexpr u16 addr32                = 1;
                constexpr u16 addr32nb              = 2;
                constexpr u16 branch26              = 3;
                constexpr u16 page_base_rel_21      = 4;
                constexpr u16 rel_21                = 5;
                constexpr u16 page_offset_12a       = 6;
                constexpr u16 page_offset_12l       = 7;
                constexpr u16 section_rel           = 8;
                constexpr u16 section_rel_low12a    = 9;
                constexpr u16 section_rel_high12a   = 10;
                constexpr u16 section_rel_low12l    = 11;
                constexpr u16 token                 = 12;
                constexpr u16 section               = 13;
                constexpr u16 addr64                = 14;
                constexpr u16 branch19              = 15;
                constexpr u16 branch14              = 16;
                constexpr u16 rel_32                = 17;
            }
        }

        namespace machine {
            constexpr u16 amd64                     = 0x8664;
            constexpr u16 arm64                     = 0xaa64;
        }

        namespace section {
            constexpr u32 header_size               = 0x28;
            constexpr u32 no_pad                    = 0x00000008;
            constexpr u32 content_code              = 0x00000020;
            constexpr u32 data_init                 = 0x00000040;
            constexpr u32 data_uninit               = 0x00000080;
            constexpr u32 link_info                 = 0x00000200;
            constexpr u32 link_remove               = 0x00000800;
            constexpr u32 link_comdat               = 0x00001000;
            constexpr u32 gp_relative               = 0x00008000;
            constexpr u32 memory_purgeable          = 0x00020000;
            constexpr u32 memory_locked             = 0x00040000;
            constexpr u32 memory_preload            = 0x00080000;
            constexpr u32 memory_align_1            = 0x00100000;
            constexpr u32 memory_align_2            = 0x00200000;
            constexpr u32 memory_align_4            = 0x00300000;
            constexpr u32 memory_align_8            = 0x00400000;
            constexpr u32 memory_align_16           = 0x00500000; // default
            constexpr u32 memory_align_32           = 0x00600000;
            constexpr u32 memory_align_64           = 0x00700000;
            constexpr u32 memory_align_128          = 0x00800000;
            constexpr u32 memory_align_256          = 0x00900000;
            constexpr u32 memory_align_512          = 0x00a00000;
            constexpr u32 memory_align_1024         = 0x00b00000;
            constexpr u32 memory_align_2048         = 0x00c00000;
            constexpr u32 memory_align_4096         = 0x00d00000;
            constexpr u32 memory_align_8192         = 0x00e00000;
            constexpr u32 link_reloc_overflow       = 0x01000000;
            constexpr u32 memory_discard            = 0x02000000;
            constexpr u32 memory_not_cached         = 0x04000000;
            constexpr u32 memory_not_paged          = 0x08000000;
            constexpr u32 memory_shared             = 0x10000000;
            constexpr u32 memory_execute            = 0x20000000;
            constexpr u32 memory_read               = 0x40000000;
            constexpr u32 memory_write              = 0x80000000;
        }

        namespace flags {
            constexpr u32 relocs_stripped           = 0x0001;
            constexpr u32 executable_type           = 0x0002;
            constexpr u32 line_nums_stripped        = 0x0004;
            constexpr u32 local_syms_stripped       = 0x0008;
            constexpr u32 aggressive_ws_trim        = 0x0010;
            constexpr u32 large_address_aware       = 0x0020;
            constexpr u32 reserved                  = 0x0040;
            constexpr u32 bytes_reversed_lo         = 0x0080;
            constexpr u32 machine_32bit             = 0x0100;
            constexpr u32 debug_stripped            = 0x0200;
            constexpr u32 removable_run_from_swap   = 0x0400;
            constexpr u32 net_run_from_swap         = 0x0800;
            constexpr u32 system_type               = 0x1000;
            constexpr u32 dll_type                  = 0x2000;
            constexpr u32 up_system_only            = 0x4000;
            constexpr u32 bytes_reversed_hi         = 0x8000;
        }

        namespace comdat {
            constexpr u32 select_none               = 0;
            constexpr u32 select_no_duplicates      = 1;
            constexpr u32 select_any                = 2;
            constexpr u32 select_same_as            = 3;
            constexpr u32 select_exact_match        = 4;
            constexpr u32 select_associative        = 5;
            constexpr u32 select_largest            = 6;
        }

        namespace symtab {
            constexpr u32 entry_size                = 0x12;

            namespace aux {
                constexpr u8 clr_token_def          = 1;
            }

            namespace type {
                constexpr u8 none                   = 0;
                constexpr u8 function               = 0x20;
            }

            namespace section {
                constexpr s32 undef                 = 0;
                constexpr s32 absolute              = -1;
                constexpr s32 debug                 = -2;
            }

            namespace sclass {
                constexpr u8 null_                  = 0;
                constexpr u8 auto_                  = 1;
                constexpr u8 external_              = 2;
                constexpr u8 static_                = 3;
                constexpr u8 register_              = 4;
                constexpr u8 extern_def             = 5;
                constexpr u8 label                  = 6;
                constexpr u8 undef_label            = 7;
                constexpr u8 member_of_struct       = 8;
                constexpr u8 argument               = 9;
                constexpr u8 struct_tag             = 10;
                constexpr u8 member_of_union        = 11;
                constexpr u8 union_tag              = 12;
                constexpr u8 type_def               = 13;
                constexpr u8 undef_static           = 14;
                constexpr u8 enum_tag               = 15;
                constexpr u8 member_of_enum         = 16;
                constexpr u8 register_param         = 17;
                constexpr u8 bit_field              = 18;
                constexpr u8 block                  = 100;
                constexpr u8 function               = 101;
                constexpr u8 end_of_struct          = 102;
                constexpr u8 file                   = 103;
                constexpr u8 section                = 104;
                constexpr u8 weak_external          = 105;
                constexpr u8 clr_token              = 107;
                constexpr u8 end_of_function        = 0xff;
            }
        }

        namespace line_num {
            constexpr u32 entry_size                = 6;
        }
    }

    namespace macho {
        namespace fat {
            constexpr u32 magic                         = 0xcafebabe;
            constexpr u32 cigam                         = 0xbebafeca;
            constexpr u32 magic_64                      = 0xcafebabf;
            constexpr u32 cigam_64                      = 0xbfbafeca;
        }

        namespace file {
            constexpr u32 magic                         = 0xfeedface;
            constexpr u32 cigam                         = 0xcefaedfe;
            constexpr u32 magic_64                      = 0xfeedfacf;
            constexpr u32 cigam_64                      = 0xcffaedfe;

            namespace type {
                constexpr u32 object                    = 0x1;
                constexpr u32 execute                   = 0x2;
                constexpr u32 fvm_lib                   = 0x3;
                constexpr u32 core                      = 0x4;
                constexpr u32 preload                   = 0x5;
                constexpr u32 dylib                     = 0x6;
                constexpr u32 dylinker                  = 0x7;
                constexpr u32 bundle                    = 0x8;
                constexpr u32 dylib_stub                = 0x9;
                constexpr u32 dsym                      = 0xa;
                constexpr u32 kext_bundle               = 0xb;
            }

            namespace flags {
                constexpr u32 none                      = 0;
                constexpr u32 no_undefs                 = 0x1;
                constexpr u32 incr_link                 = 0x2;
                constexpr u32 dyld_link                 = 0x4;
                constexpr u32 bin_dat_load              = 0x8;
                constexpr u32 pre_bound                 = 0x10;
                constexpr u32 split_segs                = 0x20;
                constexpr u32 lazy_hint                 = 0x40;
                constexpr u32 two_level                 = 0x80;
                constexpr u32 force_flat                = 0x100;
                constexpr u32 no_multi_defs             = 0x200;
                constexpr u32 no_fix_pre_binding        = 0x400;
                constexpr u32 pre_bindable              = 0x800;
                constexpr u32 all_mods_bound            = 0x1000;
                constexpr u32 subsections_via_symbols   = 0x2000;
                constexpr u32 canonical                 = 0x4000;
                constexpr u32 weak_defines              = 0x8000;
                constexpr u32 binds_to_weak             = 0x10000;
                constexpr u32 allow_stack_exec          = 0x20000;
                constexpr u32 root_safe                 = 0x40000;
                constexpr u32 setuid_safe               = 0x80000;
                constexpr u32 no_reexported_dylibs      = 0x100000;
                constexpr u32 pie                       = 0x200000;
                constexpr u32 dead_strippable_dylib     = 0x400000;
            }
        }

        namespace name {
            constexpr u32 undef                         = 0;
            constexpr u32 abs                           = 2;
            constexpr u32 ext                           = 1;
            constexpr u32 sect                          = 0xe;
            constexpr u32 weak_ref                      = 0x0040;
            constexpr u32 weak_def                      = 0x0080;
        }

        namespace segment {
            namespace type {
                constexpr u32 null                      = 0;    // (no name)
                constexpr u32 page_zero                 = 1;    // __PAGEZERO
                constexpr u32 text                      = 2;    // __TEXT
                //  sections:
                //      __text
                //      __cstring
                //      __picsymbol_stub
                //      __symbol_stub
                //      __const
                //      __literal4
                //      __literal8
                constexpr u32 data                      = 3;    // __DATA
                //  sections:
                //      __data
                //      __la_symbol_ptr
                //      __nl_symbol_ptr
                //      __dyld
                //      __const
                //      __mod_init_func
                //      __mod_term_func
                //      __bss
                //      __common
                constexpr u32 objc                      = 4;    // __OBJC
                constexpr u32 import                    = 5;    // __IMPORT
                //  sections:
                //      __jump_table
                //      __pointers
                constexpr u32 link_edit                 = 6;    // __LINKEDIT
            }
        }

        namespace section {
            namespace type {
                constexpr u32 unknown                   = 0;
                constexpr u32 discard                   = 1;
                constexpr u32 text                      = 2;
                constexpr u32 stubs                     = 3;
                constexpr u32 ro_data                   = 4;
                constexpr u32 uw_info                   = 5;
                constexpr u32 nl_ptr                    = 6;
                constexpr u32 la_ptr                    = 7;
                constexpr u32 init                      = 8;
                constexpr u32 fini                      = 9;
                constexpr u32 rw_data                   = 10;
                constexpr u32 bss                       = 11;
                constexpr u32 link_edit                 = 12;
            }

            namespace flags {
                constexpr u32 regular                   = 0x0;
                constexpr u32 zero_fill                 = 0x1;
                constexpr u32 non_lazy_syms             = 0x06;
                constexpr u32 init_func_ptr             = 0x09;
                constexpr u32 term_func_ptr             = 0x0a;
                constexpr u32 attr_pure_instr           = 0x80000000;
                constexpr u32 attr_some_instr           = 0x00000400;
            }
        }

        namespace machine {
            namespace type {
                constexpr s32 unknown                   = 0;
                constexpr s32 any                       = -1;
                constexpr s32 aarch64                   = 12;
                constexpr s32 x86_64                    = 0x10000007;
            }

            namespace sub_type {
                constexpr u32 unknown                   = 0;
                constexpr u32 lib64                     = 0x80000000;
            }
        }

        namespace load_command {
            constexpr u32 req_dyld                      = 0x80000000;

            namespace type {
                constexpr u32 segment                   = 0x1;
                constexpr u32 symtab                    = 0x2;
                constexpr u32 symseg                    = 0x3;
                constexpr u32 thread                    = 0x4;
                constexpr u32 unix_thread               = 0x5;
                constexpr u32 load_fvm_lib              = 0x6;
                constexpr u32 id_fvm_lib                = 0x7;
                constexpr u32 ident                     = 0x8;
                constexpr u32 fvm_file                  = 0x9;
                constexpr u32 pre_page                  = 0xa;
                constexpr u32 dysymtab                  = 0xb;
                constexpr u32 load_dylib                = 0xc;
                constexpr u32 id_dylib                  = 0xd;
                constexpr u32 load_dylinker             = 0xe;
                constexpr u32 id_dylinker               = 0xf;
                constexpr u32 prebound_dylib            = 0x10;
                constexpr u32 routines                  = 0x11;
                constexpr u32 sub_framework             = 0x12;
                constexpr u32 sub_umbrella              = 0x13;
                constexpr u32 sub_client                = 0x14;
                constexpr u32 sub_library               = 0x15;
                constexpr u32 two_level_hints           = 0x16;
                constexpr u32 prebind_cksum             = 0x17;
                constexpr u32 load_weak_dylib           = (u32(0x18) | req_dyld);
                constexpr u32 segment_64                = 0x19;
                constexpr u32 routines_64               = 0x1a;
                constexpr u32 uuid                      = 0x1b;
                constexpr u32 rpath                     = (u32(0x1c) | req_dyld);
                constexpr u32 code_signature            = 0x1d;
                constexpr u32 segment_split_info        = 0x1e;
                constexpr u32 reexport_dylib            = (u32(0x1f) | req_dyld);
                constexpr u32 lazy_load_dylib           = 0x20;
                constexpr u32 encryption_info           = 0x21;
                constexpr u32 dyld_info                 = 0x22;
                constexpr u32 dyld_info_only            = (u32(0x22) | req_dyld);
                constexpr u32 main                      = (u32(0x28) | req_dyld);
            }
        }

    }

    struct symbol_t final {
        section_t*              section;
        symbol_t*               next;
        u64                     size;
        u64                     value;
        u32                     ndx;
        u32                     name_offset;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;

        b8 operator==(const symbol_t& other) const {
            return this == &other;
        }
    };

    struct symbol_opts_t final {
        section_t*              section;
        u64                     size;
        u64                     value;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;
    };

    struct import_t final {
        sym_ptr_array_t         symbols;
        symbol_t*               module_symbol;
        section_t*              section;
        struct {
            u32                 load:   1;
            u32                 pad:    31;
        }                       flags;
    };

    struct group_t final {
        u32                     flags;
        id_array_t              sections;
    };

    struct reloc_t final {
        symbol_t*               symbol;
        u64                     offset;
        s64                     addend;
        union {
            machine::x86_64::reloc::type_t  x86_64_type;
            machine::aarch64::reloc::type_t aarch64_type;
        };
    };

    struct symbol_table_t final {
        alloc_t*                alloc;
        sym_hashtab_t           index;
        sym_ptr_array_t         symbols;

        symbol_t* operator[](u32 idx) {
            return idx < symbols.size ? symbols[idx] : nullptr;
        }

        const symbol_t* operator[](u32 idx) const {
            return idx < symbols.size ? symbols[idx] : nullptr;
        }
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
        reloc_array_t           relocs;
        string_table_t          strtab;
        symbol_table_t          symtab;
        import_array_t          imports;
    };

    struct section_t final {
        alloc_t*                alloc;
        const module_t*         module;
        section_t*              link;
        section_subclass_t      subclass;
        u32                     info;
        u32                     size;
        u32                     align;
        u32                     ext_type;
        u32                     number;
        u32                     name_offset;
        section::flags_t        flags;
        section::type_t         type;

        inline auto& as_data()      { return subclass.data;     }
        inline auto& as_group()     { return subclass.group;    }
        inline auto& as_relocs()    { return subclass.relocs;   }
        inline auto& as_strtab()    { return subclass.strtab;   }
        inline auto& as_symtab()    { return subclass.symtab;   }
        inline auto& as_imports()   { return subclass.imports;  }
    };

    struct section_opts_t final {
        section_t*              link;
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

    struct member_t final {
        str::slice_t            buf;
        str::slice_t            name;
        module_t*               owner;
        module_t*               module;
        member_id               id;
        u32                     uid;
        u32                     gid;
        u32                     date;
        u32                     mode;
        struct {
            u32                 data;
            u32                 header;
        }                       offset;
        format_type_t           format_type;
    };

    union module_subclass_t final {
        struct {
            section_t*          extended_symtab;
            member_array_t      members;
            ar_index_table_t    index;
            bitset_t            bitmap;
            u32                 long_names;
            u32                 coff_table;
        }                       archive;
    };

    struct module_t final {
        alloc_t*                alloc;
        section_t*              strtab;
        section_t*              symtab;
        module_subclass_t       subclass;
        section_ptr_array_t     sections;
        module_id               id;
        module_type_t           type;

        inline auto       as_archive()          { return &subclass.archive; }
        inline const auto as_archive() const    { return &subclass.archive; }
    };
}
