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

#include <basecode/core/buf.h>
#include <basecode/objfmt/coff.h>

namespace basecode::objfmt::container::pe {
    enum dir_type_t : u8 {
        export_table,
        import_table,
        resource_table,
        exception_table,
        certificate_table,
        relocation_table,
        debug_table,
        architecture_table,
        global_table,
        tls_table,
        load_config_table,
        bound_import_table,
        import_address_table,
        delay_import_descriptor,
        com_header,
        reserved
    };

    constexpr u32 max_dir_entry_count = 16;

    struct data_bound_t final {
        u32                     base;
        u32                     size;
    };

    struct dir_entry_t final {
        u32                     rva;
        u32                     size;
    };

    struct pe_t final {
        coff::section_hdr_t*    hdrs;
        buf_t                   buf;
        u64                     rva;
        u64                     offset;
        u64                     base_addr;
        struct {
            u64                 image;
            u64                 headers;
        }                       size;
        buf_crsr_t              crsr;
        struct {
            u32                 file;
            u32                 section;
        }                       align;
        data_bound_t            code;
        data_bound_t            init_data;
        dir_entry_t             name_table;
        data_bound_t            uninit_data;
        dir_entry_t             import_lookup_table;
        dir_entry_t             dirs[max_dir_entry_count];
    };

    namespace pe {
        u0 free(pe_t& pe);

        u0 write_pad(pe_t& pe);

        u0 write_dos_header(pe_t& pe);

        u0 seek(pe_t& pe, u32 offset);

        u0 write_u8(pe_t& pe, u8 value);

        u0 write_section_data(pe_t& pe);

        u0 write_u16(pe_t& pe, u16 value);

        u0 write_u32(pe_t& pe, u32 value);

        u0 write_u64(pe_t& pe, u64 value);

        status_t save(pe_t& pe, const path_t& path);

        u0 write_pad8(pe_t& pe, str::slice_t slice);

        u0 write_cstr(pe_t& pe, str::slice_t slice);

        u0 write_pad16(pe_t& pe, str::slice_t slice);

        u0 write_str(pe_t& pe, const String_Concept auto& str) {
            buf::cursor::write_str(pe.crsr, str);
        }

        u0 write_section_headers(pe_t& pe, const context_t& ctx);

        u0 write_optional_header(pe_t& pe, const context_t& ctx);

        status_t prepare_sections(pe_t& pe, const context_t& ctx);

        u0 write_pe_and_coff_header(pe_t& pe, const context_t& ctx);

        status_t init(pe_t& pe, coff::section_hdr_t* hdrs, alloc_t* alloc);
    }

    system_t* system();
}
