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

#include <basecode/binfmt/coff.h>

namespace basecode::binfmt::pe {
    namespace base_reloc {
        namespace type {
            constexpr u8 unknown  = 0;
        }
    }

    namespace dir_entry {
        u0 free(pe_t& pe, pe_dir_type_t type);

        status_t init(pe_t& pe, pe_dir_type_t type);
    }

    u0 free(pe_t& pe);

    status_t init(pe_t& pe, const pe_opts_t& opts);

    status_t build_sections(file_t& file, pe_t& pe);

    status_t write_pe_header(file_t& file, pe_t& pe);

    status_t write_dos_header(file_t& file, pe_t& pe);

    status_t write_sections_data(file_t& file, pe_t& pe);

    status_t write_optional_header(file_t& file, pe_t& pe);

    status_t build_section(file_t& file, pe_t& pe, coff_header_t& hdr);

    status_t write_section_data(file_t& file, pe_t& pe, coff_header_t& hdr);
}
