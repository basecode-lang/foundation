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

#include <basecode/objfmt/coff.h>

namespace basecode::objfmt::container {
    struct pe_thunk_t;
    struct pe_name_hint_t;
    struct pe_import_module_t;

    using pe_name_hint_list_t           = array_t<pe_name_hint_t>;
    using pe_import_module_list_t       = array_t<pe_import_module_t>;

    enum dir_type_t : u8 {
        reserved                    = 15,
        tls_table                   = 9,
        com_header                  = 14,
        debug_table                 = 6,
        export_table                = 0,
        import_table                = 1,
        global_table                = 8,
        resource_table              = 2,
        exception_table             = 3,
        relocation_table            = 5,
        certificate_table           = 4,
        load_config_table           = 10,
        bound_import_table          = 11,
        architecture_table          = 7,
        import_address_table        = 12,
        delay_import_descriptor     = 13,
    };
    constexpr u32 max_dir_entry_count   = 16;

    struct pe_thunk_t final {
        union {
            struct {
                u64             value:      63;
                u64             by_ordinal: 1;
            }                   thunk;
            u64                 bits;
        };
    };

    struct pe_name_hint_t final {
        str::slice_t            name;
        rva_t                   rva;
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
        pe_import_module_list_t modules;
        struct {
            pe_name_hint_list_t list;
            rva_t               rva;
        }                       name_hints;
    };

    struct pe_dir_t final {
        union {
            pe_import_t         import;
        }                       subclass;
        rva_t                   rva;
        dir_type_t              type;
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
        pe_dir_t                dirs[max_dir_entry_count];
    };

    struct pe_opts_t final {
        alloc_t*                alloc;
        const file_t*           file;
        u64                     base_addr;
        u64                     heap_reserve;
        u64                     stack_reserve;
    };

    namespace pe {
        namespace dir_entry {
            u0 free(pe_t& pe, dir_type_t type);

            status_t init(pe_t& pe, dir_type_t type);
        }

        u0 free(pe_t& pe);

        system_t* system();

        u0 write_pe_header(session_t& s, pe_t& pe);

        u0 write_dos_header(session_t& s, pe_t& pe);

        status_t init(pe_t& pe, const pe_opts_t& opts);

        status_t build_sections(session_t& s, pe_t& pe);

        u0 write_optional_header(session_t& s, pe_t& pe);

        status_t write_sections_data(session_t& s, pe_t& pe);

        status_t build_section(session_t& s, pe_t& pe, coff_section_hdr_t& hdr);

        status_t write_section_data(session_t& s, pe_t& pe, coff_section_hdr_t& hdr);
    }
}
