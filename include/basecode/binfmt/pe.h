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

#include <basecode/binfmt/coff.h>

namespace basecode::binfmt::io::pe {
    struct thunk_t;
    struct reloc_t;
    struct res_data_t;
    struct name_hint_t;
    struct res_entry_t;
    struct export_addr_t;
    struct import_module_t;

    using reloc_list_t               = array_t<reloc_t>;
    using res_data_list_t            = array_t<res_data_t>;
    using res_entry_list_t           = array_t<res_entry_t>;
    using name_hint_list_t           = array_t<name_hint_t>;
    using export_addr_list_t         = array_t<export_addr_t>;
    using tls_callback_list_t        = array_t<u64>;
    using import_module_list_t       = array_t<import_module_t>;

    enum dir_type_t : u8 {
        reserved                        = 15,
        tls_table                       = 9,
        com_header                      = 14,
        debug_table                     = 6,
        gp_register                     = 8,
        export_table                    = 0,
        import_table                    = 1,
        resource_table                  = 2,
        exception_table                 = 3,
        base_reloc_table                = 5,
        certificate_table               = 4,
        load_config_table               = 10,
        bound_import_table              = 11,
        architecture_table              = 7,
        import_address_table            = 12,
        delay_import_descriptor         = 13,
    };
    constexpr u32 max_dir_entry_count   = 16;

    struct res_data_t final {
        str::slice_t            data;
        coff::rva_t             rva;
        u32                     code_page;
    };

    struct res_entry_t final {
        u32                     id;
        u32                     offset;
        b8                      is_name;
        b8                      is_subdir;
    };

    struct res_t final {
        res_entry_list_t        entries;
        res_data_list_t         blocks;
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
    };

    struct tls_t final {
        tls_callback_list_t     callbacks;
        struct {
            u64                 start_va;
            u64                 end_va;
        }                       raw_data;
        u64                     index_addr;
        u32                     size_zero_fill;
        u32                     flags;
    };

    struct thunk_t final {
        union {
            struct {
                u64             value:      63;
                u64             by_ordinal: 1;
            }                   thunk;
            u64                 bits;
        };
    };

    struct debug_t final {
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
        u32                     type;
        coff::rva_t             mem;
        coff::raw_t             file;
    };

    struct load_cfg_t final {
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

    struct name_hint_t final {
        str::slice_t            name;
        coff::rva_t             rva;
        u16                     hint;
        b8                      pad;
    };

    struct import_module_t final {
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

    struct import_t final {
        import_module_list_t    modules;
        struct {
            name_hint_list_t    list;
            coff::rva_t         rva;
        }                       name_hints;
    };

    struct export_addr_t final {
        struct {
            str::slice_t        slice;
            coff::rva_t         rva;
        }                       name;
        u32                     rva;
        u16                     ordinal;
    };

    struct export_t final {
        export_addr_list_t      exports;
        u32                     flags;
        u32                     time_stamp;
        version_t               version;
        struct {
            str::slice_t        name;
            coff::rva_t         rva;
        }                       module;
        struct {
            coff::rva_t         table;
            u32                 base;
        }                       ordinal;
        struct {
            coff::rva_t         table;
            coff::rva_t         pointers;
        }                       name;
        coff::rva_t             table;
    };

    struct reloc_t final {
        u16                     offset: 12;
        u16                     type:   4;
    };

    struct base_reloc_t final {
        reloc_list_t            relocs;
        coff::rva_t             page;
    };

    struct dir_t final {
        union {
            tls_t               tls;
            res_t               resources;
            import_t            import;
            export_t            export_;
            load_cfg_t          load_cfg;
            base_reloc_t        base_reloc;
        }                       subclass;
        coff::rva_t             rva;
        dir_type_t              type;
        b8                      init;
    };

    struct pe_t final {
        coff::coff_t            coff;
        u64                     base_addr;
        struct {
            u64                 heap;
            u64                 stack;
        }                       reserve;
        struct {
            u32                 opt_hdr;
            u32                 dos_stub;
        }                       size;
        struct {
            u32                 dll;
            u32                 load;
        }                       flags;
        struct {
            u32                 include_symbol_table:   1;
            u32                 pad:                    31;
        }                       opts;
        u32                     start_offset;
        dir_t                   dirs[max_dir_entry_count];
    };

    struct opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     base_addr;
        u64                     heap_reserve;
        u64                     stack_reserve;
    };

    namespace base_reloc {
        namespace type {
            [[maybe_unused]] constexpr u8 unknown  = 0;
        }
    }

    namespace dir_entry {
        u0 free(pe_t& pe, dir_type_t type);

        status_t init(pe_t& pe, dir_type_t type);
    }

    u0 free(pe_t& pe);

    u0 write_pe_header(file_t& file, pe_t& pe);

    u0 write_dos_header(file_t& file, pe_t& pe);

    status_t init(pe_t& pe, const opts_t& opts);

    status_t build_sections(file_t& file, pe_t& pe);

    u0 write_optional_header(file_t& file, pe_t& pe);

    status_t write_sections_data(file_t& file, pe_t& pe);

    status_t build_section(file_t& file, pe_t& pe, coff::section_hdr_t& hdr);

    status_t write_section_data(file_t& file, pe_t& pe, coff::section_hdr_t& hdr);
}
