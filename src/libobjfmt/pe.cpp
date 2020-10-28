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

    namespace internal {
        static u0 fini() {
        }

        static status_t init(alloc_t* alloc) {
            UNUSED(alloc);
            return status_t::ok;
        }

        static status_t read(session_t& s) {
            UNUSED(s);
            return status_t::ok;
        }

        static status_t write(session_t& s) {
            section_hdr_t hdrs[s.file->sections.size];
            for (u32 i = 0; i < s.file->sections.size; ++i) {
                auto& hdr = hdrs[i];
                hdr.section = &s.file->sections[i];
                hdr.rva     = hdr.offset = hdr.size = {};
                hdr.number  = i + 1;
            }

            pe_t pe{};
            pe::init(pe, hdrs, s.file->sections.size, s.file->alloc);
            defer(pe::free(pe));
            auto& coff = pe.coff;

            // XXX: shouldn't be hard coded
            coff.size.opt_hdr = 0xf0;
            coff.flags.image  = coff::flags::relocs_stripped
                                | coff::flags::executable_type
                                | coff::flags::large_address_aware;

            pe::write_dos_header(s, pe);
            pe::write_pe_header(s, pe);
            coff::write_header(s, coff);
            pe::prepare_sections(s, pe);
            pe::write_optional_header(s, pe);
            coff::write_section_headers(s, coff);
            pe::write_section_data(s, pe);

            auto status = session::save(s);
            if (!OK(status))
                return status_t::write_error;

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

    [[maybe_unused]] constexpr u32 dos_header_size              = 0x40;
    [[maybe_unused]] constexpr u32 pe_sig_size                  = 0x04;
    [[maybe_unused]] constexpr u32 imp_addr_entry_size          = 0x08;
    [[maybe_unused]] constexpr u32 imp_dir_entry_size           = 0x14;
    [[maybe_unused]] constexpr u32 imp_lookup_entry_size        = imp_addr_entry_size;
    [[maybe_unused]] constexpr u32 name_table_entry_size        = 0x12;

    [[maybe_unused]] constexpr u16 pe32                         = 0x10b;
    [[maybe_unused]] constexpr u16 pe64                         = 0x20b;
    [[maybe_unused]] constexpr u16 rom                          = 0x107;

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

    namespace pe {
        u0 free(pe_t& pe) {
            coff::free(pe.coff);
        }

        system_t* system() {
            return &internal::g_pe_sys;
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
            session::write_u32(s, pe.code.size);
            session::write_u32(s, pe.init_data.size);
            session::write_u32(s, pe.uninit_data.size);
            session::write_u32(s, pe.code.base);
            session::write_u32(s, pe.code.base);
            session::write_u64(s, pe.base_addr);
            session::write_u32(s, coff.align.section);
            session::write_u32(s, coff.align.file);
            session::write_u16(s, s.versions.min_os.major);
            session::write_u16(s, s.versions.min_os.minor);
            session::write_u16(s, 0);            // XXX: image ver major
            session::write_u16(s, 0);            // XXX: image ver minor
            session::write_u16(s, s.versions.min_os.major);
            session::write_u16(s, s.versions.min_os.minor);
            session::write_u32(s, 0);
            session::write_u32(s, align(coff.rva + pe.size.image, coff.align.section));
            session::write_u32(s, align(pe.size.headers, pe.coff.align.file));
            session::write_u32(s, 0);
            if (s.flags.console) {
                session::write_u16(s, subsystem::win_cui);
            } else {
                session::write_u16(s, subsystem::win_gui);
            }
            session::write_u16(s, 0);            // XXX: dll flags
            session::write_u64(s, 0x100000);     // XXX: size of stack reserve
            session::write_u64(s, 0x1000);       // XXX: size of stack commit
            session::write_u64(s, 0x100000);     // XXX: size of heap reserve
            session::write_u64(s, 0x1000);       // XXX: size of heap commit
            session::write_u32(s, 0);            // XXX: load flags
            session::write_u32(s, max_dir_entry_count);

            for (const auto& dir : pe.dirs) {
                session::write_u32(s, dir.base);
                session::write_u32(s, dir.size);
            }
        }

        status_t prepare_sections(session_t& s, pe_t& pe) {
            auto& coff = pe.coff;
            pe.size.headers = coff.offset
                              + (pe_sig_size + coff::header_size + coff.size.opt_hdr)
                              + (coff.num_hdrs * coff::section::header_size);
            coff.offset = pe.size.headers;
            coff.rva    = pe.size.headers;

            for (u32 i = 0; i < pe.coff.num_hdrs; ++i) {
                auto& hdr = pe.coff.hdrs[i];
                coff.offset = hdr.offset = align(coff.offset + pe.size.image, coff.align.file);
                coff.rva    = hdr.rva    = align(coff.rva + pe.size.image, coff.align.section);
                switch (hdr.section->type) {
                    case section::type_t::code:
                        if (!pe.code.base)
                            pe.code.base = hdr.rva;
                        pe.size.image = hdr.size = hdr.section->subclass.data.length;
                        pe.code.size += pe.size.image;
                        break;
                    case section::type_t::data:
                    case section::type_t::custom:
                        if (!pe.init_data.base)
                            pe.init_data.base = hdr.rva;
                        pe.size.image = hdr.size = hdr.section->subclass.data.length;
                        pe.init_data.size += pe.size.image;
                        break;
                    case section::type_t::uninit:
                        pe.size.image = hdr.size = hdr.section->subclass.size;
                        pe.uninit_data.size += pe.size.image;
                        break;
                    case section::type_t::import: {
                        auto& import_table = pe.dirs[dir_type_t::import_table];
                        auto& import_address_table = pe.dirs[dir_type_t::import_address_table];
                        const auto& imports = hdr.section->subclass.imports;
                        import_address_table.base = coff.rva;
                        u32 imported_count{};
                        for (const auto& import : imports)
                            imported_count += import.symbols.size;
                        import_address_table.size = (imported_count + 1) * imp_addr_entry_size;
                        coff.rva += import_address_table.size;

                        import_table.base = coff.rva;
                        import_table.size = imp_dir_entry_size * 2;
                        coff.rva += import_table.size;

                        pe.import_lookup_table.base = coff.rva;
                        coff.rva += import_address_table.size;

                        pe.name_table.base = coff.rva;
                        pe.name_table.size = (imported_count + 1) * name_table_entry_size;
                        coff.rva += pe.name_table.size;

                        pe.size.image = hdr.size = coff.rva - import_address_table.base;
                        pe.init_data.size += pe.size.image;
                        break;
                    }
                    case section::type_t::debug:
                    case section::type_t::export_:
                    case section::type_t::resource:
                        return status_t::not_implemented;
                }
            }

            return status_t::ok;
        }

        u0 write_section_data(session_t& s, pe_t& pe) {
            for (u32 i = 0; i < pe.coff.num_hdrs; ++i) {
                const auto& hdr = pe.coff.hdrs[i];
                const auto type = hdr.section->type;
                const auto& sc = hdr.section->subclass;
                if (type == section::type_t::uninit)
                    continue;
                session::seek(s, hdr.offset);
                switch (type) {
                    case section::type_t::code:
                    case section::type_t::data:
                        session::write_str(s, sc.data);
                        break;
                    case section::type_t::import: {
                        const auto& imports = sc.imports;
                        for (const auto& import : imports) {
                            const auto module_symbol = file::get_symbol(*s.file, import.module);
                            const auto module_intern = string::interned::get(module_symbol->name);

                            // import address table
                            u32 name_table_ptr = pe.name_table.base + name_table_entry_size;
                            for (u32 i = 0; i < import.symbols.size; ++i) {
                                session::write_u64(s, name_table_ptr);
                                name_table_ptr += name_table_entry_size;
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
                            name_table_ptr = pe.name_table.base + name_table_entry_size;
                            for (u32 i = 0; i < import.symbols.size; ++i) {
                                session::write_u64(s, name_table_ptr);
                                name_table_ptr += name_table_entry_size;
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
                    default:
                        break;
                }
            }
        }

        status_t init(pe_t& pe, section_hdr_t* hdrs, u32 num_hdrs, alloc_t* alloc) {
            auto status = coff::init(pe.coff, hdrs, num_hdrs, alloc);
            if (!OK(status))
                return status;
            auto& coff = pe.coff;
            coff.rva              = coff.offset  = {};
            pe.size               = {};
            pe.base_addr          = 0x140000000;
            coff.align.file       = 0x200;
            coff.align.section    = 0x1000;
            pe.code               = pe.init_data = pe.uninit_data = {};
            ZERO_MEM(pe.dirs, pe.dirs);
            return status_t::ok;
        }
    }
}
