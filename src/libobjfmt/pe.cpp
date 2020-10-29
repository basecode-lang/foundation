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

#include <basecode/core/bits.h>
#include <basecode/objfmt/pe.h>
#include <basecode/core/string.h>
#include <basecode/objfmt/objfmt.h>

namespace basecode::objfmt::container {
    [[maybe_unused]] static u8 s_dos_stub[64] = {
        0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c,
        0xcd, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
        0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65,
        0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
        0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    [[maybe_unused]] constexpr u32 dos_header_size              = 0x40;
    [[maybe_unused]] constexpr u32 pe_sig_size                  = 0x04;

    [[maybe_unused]] constexpr u16 pe32                         = 0x10b;
    [[maybe_unused]] constexpr u16 pe64                         = 0x20b;
    [[maybe_unused]] constexpr u16 rom                          = 0x107;

    namespace name_table {
        [[maybe_unused]] constexpr u32 entry_size               = 0x12;
    }

    namespace import_dir_table {
        [[maybe_unused]] constexpr u32 entry_size               = 0x14;
    }

    namespace import_addr_table {
        [[maybe_unused]] constexpr u32 entry_size               = 0x08;
    }

    namespace import_lookup_table {
        [[maybe_unused]] constexpr u32 entry_size               = 0x08;
    }

    namespace subsystem {
        [[maybe_unused]] constexpr u16 unknown                  = 0;
        [[maybe_unused]] constexpr u16 native                   = 1;
        [[maybe_unused]] constexpr u16 win_gui                  = 2;
        [[maybe_unused]] constexpr u16 win_cui                  = 3;
        [[maybe_unused]] constexpr u16 os2_cui                  = 5;
        [[maybe_unused]] constexpr u16 posix_cui                = 7;
        [[maybe_unused]] constexpr u16 win_native               = 8;
        [[maybe_unused]] constexpr u16 win_ce_gui               = 9;
        [[maybe_unused]] constexpr u16 efi_app                  = 10;
        [[maybe_unused]] constexpr u16 efi_boot_svc_driver      = 11;
        [[maybe_unused]] constexpr u16 efi_runtime_driver       = 12;
        [[maybe_unused]] constexpr u16 efi_rom                  = 13;
        [[maybe_unused]] constexpr u16 xbox                     = 14;
        [[maybe_unused]] constexpr u16 win_boot_app             = 16;
        [[maybe_unused]] constexpr u16 xbox_code_catalog        = 17;
    }

    namespace internal {
        static u0 fini() {
        }

        static status_t read(session_t& s) {
            UNUSED(s);
            return status_t::ok;
        }

        static status_t write(session_t& s) {
            const auto file = s.file;

            section_hdr_t hdrs[file->sections.size];
            for (u32 i = 0; i < file->sections.size; ++i) {
                auto& hdr = hdrs[i];
                hdr.section = &file->sections[i];
                hdr.name = {};
                auto status = coff::get_section_name(hdr.section, hdr.name);
                if (!OK(status))
                    return status;
                hdr.rva     = hdr.offset    = hdr.size = {};
                hdr.relocs  = hdr.line_nums = {};
                hdr.number  = i + 1;
            }

            pe_opts_t opts{};
            opts.alloc         = file->alloc;
            opts.hdrs          = hdrs;
            opts.num_hdrs      = file->sections.size;
            opts.machine       = file->machine;
            opts.base_addr     = s.opts.base_addr;
            opts.heap_reserve  = s.opts.heap_reserve;
            opts.stack_reserve = s.opts.stack_reserve;

            pe_t pe{};
            pe::init(pe, opts);
            defer(pe::free(pe));

            pe.opts.include_symbol_table = true;    // XXX: temporary for testing

            auto& coff = pe.coff;

            switch (s.output_type) {
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

            status_t status;

            pe::write_dos_header(s, pe);
            pe::write_pe_header(s, pe);
            status = pe::build_sections(s, pe);
            if (!OK(status))
                return status;
            if (pe.opts.include_symbol_table) {
                coff.symbol_table.size = (file->symbols.size + (file->sections.size * 2))
                                         * coff::symbol_table::entry_size;
            }
            coff::write_header(s, coff, pe.size.opt_hdr);
            pe::write_optional_header(s, pe);
            coff::write_section_headers(s, coff);
            status = pe::write_sections_data(s, pe);
            if (!OK(status))
                return status;
            if (pe.opts.include_symbol_table) {
                coff::write_symbol_table(s, coff);
            }

            status = session::save(s);
            if (!OK(status))
                return status_t::write_error;

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

    namespace pe {
        u0 free(pe_t& pe) {
            coff::free(pe.coff);
        }

        system_t* system() {
            return &internal::g_pe_sys;
        }

        status_t init(pe_t& pe, const pe_opts_t& opts) {
            auto status = coff::init(pe.coff,
                                     opts.hdrs,
                                     opts.num_hdrs,
                                     opts.machine,
                                     opts.alloc);
            if (!OK(status))
                return status;

            pe.opts          = {};
            pe.flags         = {};
            pe.base_addr     = opts.base_addr == 0 ? 0x140000000 : opts.base_addr;
            pe.reserve.heap  = opts.heap_reserve == 0 ? 0x100000 : opts.heap_reserve;
            pe.reserve.stack = opts.stack_reserve == 0 ? 0x100000 : opts.stack_reserve;

            // XXX: is this size constant for x64 images?
            pe.size.opt_hdr  = 0xf0;

            ZERO_MEM(pe.dirs, pe.dirs);

            return status_t::ok;
        }

        u0 write_pe_header(session_t& s, pe_t& pe) {
            session::seek(s, pe.coff.offset);
            session::write_u8(s, 'P');
            session::write_u8(s, 'E');
            session::write_u8(s, 0);
            session::write_u8(s, 0);
        }

        u0 write_dos_header(session_t& s, pe_t& pe) {
            const auto dos_stub_size = dos_header_size + sizeof(s_dos_stub);
            pe.coff.offset = align(dos_stub_size, 8);

            session::write_u8(s, 'M');
            session::write_u8(s, 'Z');
            session::write_u16(s, dos_stub_size % 512);
            session::write_u16(s, align(dos_stub_size, 512) / 512);
            session::write_u16(s, 0);
            session::write_u16(s, dos_header_size / 16);
            session::write_u16(s, 0);
            session::write_u16(s, 1);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, dos_header_size);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            for (u32 i = 0; i < 10; ++i)
                session::write_u16(s, 0);
            session::write_u32(s, pe.coff.offset);
            for (u8 code_byte : s_dos_stub)
                session::write_u8(s, code_byte);
        }

        u0 write_optional_header(session_t& s, pe_t& pe) {
            const auto& coff = pe.coff;
            session::write_u16(s, pe64);
            session::write_u8(s, s.versions.linker.major);
            session::write_u8(s, s.versions.linker.minor);
            session::write_u32(s, coff.code.size);
            session::write_u32(s, coff.init_data.size);
            session::write_u32(s, coff.uninit_data.size);
            session::write_u32(s, coff.code.base);
            session::write_u32(s, coff.code.base);
            session::write_u64(s, pe.base_addr);
            session::write_u32(s, coff.align.section);
            session::write_u32(s, coff.align.file);
            session::write_u16(s, s.versions.min_os.major);
            session::write_u16(s, s.versions.min_os.minor);
            session::write_u16(s, 0);
            session::write_u16(s, 0);
            session::write_u16(s, s.versions.min_os.major);
            session::write_u16(s, s.versions.min_os.minor);
            session::write_u32(s, 0);
            session::write_u32(s, align(coff.rva + coff.size.image, coff.align.section));
            session::write_u32(s, align(coff.size.headers, pe.coff.align.file));
            session::write_u32(s, 0);
            if (s.flags.console) {
                session::write_u16(s, subsystem::win_cui);
            } else {
                session::write_u16(s, subsystem::win_gui);
            }
            session::write_u16(s, pe.flags.dll);
            session::write_u64(s, pe.reserve.stack);
            session::write_u64(s, 0x1000);
            session::write_u64(s, pe.reserve.heap);
            session::write_u64(s, 0x1000);
            session::write_u32(s, pe.flags.load);
            session::write_u32(s, max_dir_entry_count);

            for (const auto& dir : pe.dirs) {
                session::write_u32(s, dir.base);
                session::write_u32(s, dir.size);
            }
        }

        status_t build_sections(session_t& s, pe_t& pe) {
            auto& coff = pe.coff;

            coff.size.headers = coff.offset
                                + (pe_sig_size + coff::header_size + pe.size.opt_hdr)
                                + (coff.num_hdrs * coff::section::header_size);
            coff.offset = align(coff.size.headers, coff.align.file);
            coff.rva    = align(coff.size.headers, coff.align.section);

            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                auto status = build_section(s, pe, coff.hdrs[i]);
                if (!OK(status))
                    return status;
            }

            if (pe.opts.include_symbol_table) {
                coff.symbol_table.offset = coff.offset;
            }

            return status_t::ok;
        }

        status_t write_sections_data(session_t& s, pe_t& pe) {
            const auto& coff = pe.coff;

            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                auto status = write_section_data(s, pe, coff.hdrs[i]);
                if (!OK(status))
                    return status;
            }

            return status_t::ok;
        }

        status_t build_section(session_t& s, pe_t& pe, section_hdr_t& hdr) {
            auto& coff = pe.coff;

            hdr.offset = coff.offset;
            hdr.rva    = coff.rva;

            switch (hdr.section->type) {
                case section::type_t::reloc: {
                    return status_t::not_implemented;
                }
                case section::type_t::export_: {
                    return status_t::not_implemented;
                }
                case section::type_t::import: {
                    auto& import_table = pe.dirs[dir_type_t::import_table];
                    auto& import_address_table = pe.dirs[dir_type_t::import_address_table];
                    const auto& imports = hdr.section->subclass.imports;
                    import_address_table.base = coff.rva;
                    u32 imported_count{};
                    for (const auto& import : imports)
                        imported_count += import.symbols.size;
                    import_address_table.size = (imported_count + 1) * import_addr_table::entry_size;
                    coff.rva += import_address_table.size;

                    import_table.base = coff.rva;
                    import_table.size = import_dir_table::entry_size * 2;
                    coff.rva += import_table.size;

                    pe.import_lookup_table.base = coff.rva;
                    coff.rva += import_address_table.size;

                    pe.name_table.base = coff.rva;
                    pe.name_table.size = (imported_count + 1) * name_table::entry_size;
                    coff.rva += pe.name_table.size;

                    hdr.size = coff.rva - import_address_table.base;

                    coff.init_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                    coff.rva        = align(coff.rva, coff.align.section);
                    break;
                }
                default: {
                    auto status = coff::build_section(s, coff, hdr);
                    if (!OK(status))
                        return status;
                }
            }

            return status_t::ok;
        }

        status_t write_section_data(session_t& s, pe_t& pe, section_hdr_t& hdr) {
            const auto type = hdr.section->type;
            if (type == section::type_t::data && !hdr.section->flags.init)
                return status_t::ok;

            const auto& sc = hdr.section->subclass;
            session::seek(s, hdr.offset);

            switch (type) {
                case section::type_t::reloc: {
                    return status_t::not_implemented;
                }
                case section::type_t::export_: {
                    return status_t::not_implemented;
                }
                case section::type_t::import: {
                    const auto& imports = sc.imports;
                    for (const auto& import : imports) {
                        const auto module_symbol = file::get_symbol(*s.file, import.module);
                        const auto module_intern = string::interned::get(module_symbol->name);

                        // import address table
                        u32 name_table_ptr = pe.name_table.base + name_table::entry_size;
                        for (u32 j = 0; j < import.symbols.size; ++j) {
                            session::write_u64(s, name_table_ptr);
                            name_table_ptr += name_table::entry_size;
                        }
                        session::write_u64(s, 0);

                        // import directory table
                        session::write_u32(s, pe.import_lookup_table.base);
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);
                        session::write_u32(s, pe.name_table.base);
                        session::write_u32(s, pe.dirs[dir_type_t::import_address_table].base);

                        // null node
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);

                        // import lookup table
                        name_table_ptr = pe.name_table.base + name_table::entry_size;
                        for (u32 j = 0; j < import.symbols.size; ++j) {
                            session::write_u64(s, name_table_ptr);
                            name_table_ptr += name_table::entry_size;
                        }
                        session::write_u64(s, 0);

                        // name table
                        session::write_pad16(s, module_intern.slice);
                        session::write_u8(s, 0);
                        session::write_u8(s, 0);
                        for (auto symbol_id : import.symbols) {
                            const auto symbol = file::get_symbol(*s.file, symbol_id);
                            const auto symbol_intern = string::interned::get(symbol->name);
                            session::write_u16(s, 0);    // XXX: hint
                            session::write_pad16(s, symbol_intern.slice);
                        }
                    }
                    break;
                }
                default: {
                    auto status = coff::write_section_data(s, pe.coff, hdr);
                    if (!OK(status))
                        return status;
                }
            }

            return status_t::ok;
        }
    }
}
