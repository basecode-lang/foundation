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

    struct pe_t final {
        coff_t                  coff;
        u64                     base_addr;
        struct {
            u64                 image;
            u64                 headers;
        }                       size;
        rva_t                   code;
        rva_t                   init_data;
        rva_t                   name_table;
        rva_t                   uninit_data;
        rva_t                   import_lookup_table;
        rva_t                   dirs[max_dir_entry_count];
    };

    namespace pe {
        u0 free(pe_t& pe);

        system_t* system();

        u0 write_pe_header(session_t& s, pe_t& pe);

        u0 write_dos_header(session_t& s, pe_t& pe);

        u0 write_section_data(session_t& s, pe_t& pe);

        u0 write_optional_header(session_t& s, pe_t& pe);

        status_t prepare_sections(session_t& s, pe_t& pe);

        status_t init(pe_t& pe, section_hdr_t* hdrs, u32 num_hdrs, alloc_t* alloc);
    }
}
