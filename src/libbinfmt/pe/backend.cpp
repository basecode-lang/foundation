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

#include <basecode/binfmt/pe.h>

namespace basecode::binfmt::io::pe {
    namespace internal {
        static u0 fini() {
        }

        static status_t read(file_t& file) {
            UNUSED(file);
            return status_t::ok;
        }

        static status_t write(file_t& file) {
            const auto module = file.module;

            opts_t opts{};
            opts.alloc         = module->alloc;
            opts.file          = &file;
            opts.base_addr     = file.opts.base_addr;
            opts.heap_reserve  = file.opts.heap_reserve;
            opts.stack_reserve = file.opts.stack_reserve;

            pe_t pe{};
            pe::init(pe, opts);
            defer(pe::free(pe));

            status_t status;
            auto& coff = pe.coff;

            switch (file.output_type) {
                case output_type_t::obj:
                case output_type_t::lib:
                    return status_t::invalid_output_type;
                case output_type_t::exe:
                    // XXX: relocs_stripped can be determined from file once we have field in the struct
                    // XXX: large_address_aware is (probably?) a fixed requirement for x64
                    coff.flags.image = coff::flags::relocs_stripped
                                       | coff::flags::executable_type
                                       | coff::flags::large_address_aware;
                    break;
                case output_type_t::dll:
                    pe.flags.dll = 0; // XXX: FIXME!
                    coff.flags.image |= coff::flags::dll_type;
                    break;
            }

            pe.opts.include_symbol_table = true;    // XXX: temporary for testing

            status = pe::build_sections(file, pe);
            if (!OK(status))
                return status;

            pe::write_dos_header(*file.session, pe);
            pe::write_pe_header(*file.session, pe);
            coff::write_header(*file.session, coff, pe.size.opt_hdr);
            pe::write_optional_header(file, pe);
            coff::write_section_headers(*file.session, coff);
            status = pe::write_sections_data(*file.session, pe);
            if (!OK(status))
                return status;
            if (pe.opts.include_symbol_table) {
                coff::write_symbol_table(*file.session, coff);
            }

            status = file::save(file);
            if (!OK(status))
                return status;

            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            UNUSED(alloc);
            return status_t::ok;
        }

        system_t                    g_pe_sys {
            .init   = init,
            .fini   = fini,
            .read   = read,
            .write  = write,
            .type   = type_t::pe
        };
    }

    system_t* system() {
        return &internal::g_pe_sys;
    }
}