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

#include <basecode/binfmt/types.h>

namespace basecode::binfmt::coff {
    namespace unwind {
        status_t get(const coff_t& coff,
                     const coff_header_t& hdr,
                     u32 idx,
                     coff_unwind_t& u);
    }

    namespace reloc {
        namespace type::x86_64 {
            str::slice_t name(u16 type);
        }

        namespace type::aarch64 {
            str::slice_t name(u16 type);
        }

        coff_reloc_t get(const coff_t& coff, const coff_header_t& hdr, u32 idx);
    }

    namespace section {
        coff_sym_t* get_symbol(coff_t& coff, coff_header_t& hdr);
    }

    namespace flags {
        str::slice_t name(u32 flag);
    }

    namespace comdat {
        str::slice_t name(u32 sel);
    }

    namespace strtab {
        u0 free(coff_t& coff);

        u0 init(coff_t& coff, alloc_t* alloc);

        u32 add(coff_t& coff, str::slice_t str);

        const s8* get(coff_t& coff, u64 offset);
    }

    namespace symtab {
        namespace sclass {
            str::slice_t name(u8 sclass);
        }

        u0 free(coff_t& sym);

        u0 init(coff_t& sym, alloc_t* alloc);

        coff_sym_t* make_symbol(coff_t& coff);

        coff_sym_t* find_symbol(coff_t& coff, u64 name);

        coff_sym_t* get_aux(coff_t& coff, coff_sym_t* sym, u32 idx);

        coff_sym_t* make_symbol(coff_t& coff, str::slice_t name);

        coff_sym_t* make_symbol(coff_t& coff, u64 name, u32 offset);

        coff_sym_t* make_aux(coff_t& coff, coff_sym_t* sym, coff_sym_type_t type);
    }

    namespace line_num {
        coff_line_num_t get(const coff_t& coff, const coff_header_t& hdr, u32 idx);
    }

    u0 free(coff_t& coff);

    u0 update_symbol_table(coff_t& coff);

    status_t get_section_name(const binfmt::section_t* section,
                              str::slice_t& name);

    status_t read_header(file_t& file, coff_t& coff);

    status_t write_header(file_t& file, coff_t& coff);

    u0 set_section_flags(file_t& file, coff_header_t& hdr);

    status_t build_sections(file_t& file, coff_t& coff);

    status_t read_symbol_table(file_t& file, coff_t& coff);

    status_t write_string_table(file_t& file, coff_t& coff);

    status_t write_symbol_table(file_t& file, coff_t& coff);

    status_t write_sections_data(file_t& file, coff_t& coff);

    status_t init(coff_t& coff, file_t& file, alloc_t* alloc);

    status_t read_section_headers(file_t& file, coff_t& coff);

    status_t write_section_headers(file_t& file, coff_t& coff);

    status_t build_section(file_t& file, coff_t& coff, coff_header_t& hdr);

    status_t write_section_data(file_t& file, coff_t& coff, coff_header_t& hdr);

    status_t write_section_header(file_t& file, coff_t& coff, coff_header_t& hdr);
}
