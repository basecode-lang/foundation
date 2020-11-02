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

            pe_opts_t opts{};
            opts.file          = file;
            opts.alloc         = file->alloc;
            opts.base_addr     = s.opts.base_addr;
            opts.heap_reserve  = s.opts.heap_reserve;
            opts.stack_reserve = s.opts.stack_reserve;

            pe_t pe{};
            pe::init(pe, opts);
            defer(pe::free(pe));

            status_t status;
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

            pe.opts.include_symbol_table = true;    // XXX: temporary for testing

            status = pe::build_sections(s, pe);
            if (!OK(status))
                return status;

            pe::write_dos_header(s, pe);
            pe::write_pe_header(s, pe);
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
        namespace dir_entry {
            u0 free(pe_t& pe, dir_type_t type) {
                auto& entry = pe.dirs[u32(type)];
                if (!entry.init)
                    return;
                switch (type) {
                    case tls_table:break;
                    case com_header:break;
                    case debug_table:
                        break;
                    case export_table:
                        break;
                    case import_table: {
                        auto& import = entry.subclass.import;
                        array::free(import.modules);
                        array::free(import.name_hints.list);
                        break;
                    }
                    case global_table:break;
                    case resource_table:break;
                    case exception_table:break;
                    case relocation_table:break;
                    case load_config_table:break;
                    case certificate_table:break;
                    case architecture_table:break;
                    case bound_import_table:break;
                    case import_address_table:break;
                    case delay_import_descriptor:break;
                    default:
                        break;
                }
            }

            status_t init(pe_t& pe, dir_type_t type) {
                auto& entry = pe.dirs[u32(type)];
                if (entry.init)
                    return status_t::ok;
                switch (type) {
                    case tls_table:break;
                    case com_header:break;
                    case debug_table:break;
                    case export_table:break;
                    case import_table: {
                        auto& import = entry.subclass.import;
                        array::init(import.modules, pe.coff.alloc);
                        array::init(import.name_hints.list, pe.coff.alloc);
                        break;
                    }
                    case global_table:break;
                    case resource_table:break;
                    case exception_table:break;
                    case certificate_table:break;
                    case relocation_table:break;
                    case architecture_table:break;
                    case load_config_table:break;
                    case bound_import_table:break;
                    case import_address_table:break;
                    case delay_import_descriptor:break;
                    default:
                        break;
                }
                entry.init = true;
                return status_t::ok;
            }
        }

        u0 free(pe_t& pe) {
            for (u32 dir_type = 0; dir_type < max_dir_entry_count; ++dir_type)
                dir_entry::free(pe, dir_type_t(dir_type));
            coff::free(pe.coff);
        }

        system_t* system() {
            return &internal::g_pe_sys;
        }

        status_t init(pe_t& pe, const pe_opts_t& opts) {
            auto status = coff::init(pe.coff, opts.file, opts.alloc);
            if (!OK(status))
                return status;

            pe.opts          = {};
            pe.flags         = {};
            pe.base_addr     = opts.base_addr == 0 ? 0x140000000 : opts.base_addr;
            pe.reserve.heap  = opts.heap_reserve == 0 ? 0x100000 : opts.heap_reserve;
            pe.reserve.stack = opts.stack_reserve == 0 ? 0x100000 : opts.stack_reserve;
            pe.size.dos_stub = dos_header_size + sizeof(s_dos_stub);
            pe.coff.offset   = pe.start_offset = align(pe.size.dos_stub, 8);

            // XXX: is this size constant for x64 images?
            pe.size.opt_hdr  = 0xf0;

            for (u32 dir_type = 0; dir_type < max_dir_entry_count; ++dir_type) {
                auto& dir_entry = pe.dirs[dir_type];
                dir_entry.rva  = {};
                dir_entry.init = false;
                dir_entry.type = dir_type_t(dir_type);
            }

            return status_t::ok;
        }

        u0 write_pe_header(session_t& s, pe_t& pe) {
            session::seek(s, pe.start_offset);
            session::write_u8(s, 'P');
            session::write_u8(s, 'E');
            session::write_u8(s, 0);
            session::write_u8(s, 0);
        }

        u0 write_dos_header(session_t& s, pe_t& pe) {
            session::write_u8(s, 'M');
            session::write_u8(s, 'Z');
            session::write_u16(s, pe.size.dos_stub % 512);
            session::write_u16(s, align(pe.size.dos_stub, 512) / 512);
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
            session::write_u32(s, pe.start_offset);
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
                session::write_u32(s, dir.rva.base);
                session::write_u32(s, dir.rva.size);
            }
        }

        status_t build_sections(session_t& s, pe_t& pe) {
            auto& coff = pe.coff;

            coff.size.headers = coff.offset
                                + (pe_sig_size + coff::header_size + pe.size.opt_hdr)
                                + (coff.headers.size * coff::section::header_size);
            coff.offset = align(coff.size.headers, coff.align.file);
            coff.rva    = align(coff.size.headers, coff.align.section);

            for (auto& hdr : coff.headers) {
                auto status = build_section(s, pe, hdr);
                if (!OK(status))
                    return status;
            }

            coff::update_symbol_table(coff);

            return status_t::ok;
        }

        status_t write_sections_data(session_t& s, pe_t& pe) {
            auto& coff = pe.coff;

            for (auto& hdr : coff.headers) {
                auto status = write_section_data(s, pe, hdr);
                if (!OK(status))
                    return status;
            }

            return status_t::ok;
        }

        //
        // 0x....3000: import directory table
        //      import lookup table rva:    0x.....301c
        //      time/date stamp             (time_t)
        //      forwarder chain             (module name rva + name rva)
        //      module name rva             0x.....3014
        //      import address table rva    0x.....3055
        //
        //          ...
        //
        //      20-byte null sentinel value
        //
        // 0x.....3014: kernel32.dll\0
        // 0x.....301c: import directory table:
        //      1...n 8-byte thunk values
        //      0     8-byte null marker
        //
        //
        // 0x.....3055: import address table
        //      1...n 8-byte rva
        //      0     8-byte null marker
        status_t build_section(session_t& s, pe_t& pe, coff_section_hdr_t& hdr) {
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
                    status_t status;

                    status = dir_entry::init(pe, dir_type_t::import_table);
                    if (!OK(status))
                        return status;

                    status = dir_entry::init(pe, dir_type_t::import_address_table);
                    if (!OK(status))
                        return status;

                    const auto& imports = hdr.section->subclass.imports;

                    auto& iat_entry          = pe.dirs[dir_type_t::import_address_table];
                    auto& import_table_entry = pe.dirs[dir_type_t::import_table];
                    auto& import_table       = import_table_entry.subclass.import;
                    import_table_entry.rva.base = coff.rva;
                    import_table_entry.rva.size = (imports.size + 1) * import_dir_table::entry_size;

                    u32 name_hint_offset{};
                    u32 size = import_dir_table::entry_size;
                    u32 lookup_rva = import_table_entry.rva.base + import_table_entry.rva.size;
                    for (const auto& import : imports) {
                        auto& module = array::append(import_table.modules);
                        module.iat_rva = module.fwd_chain = module.time_stamp = {};
                        {
                            const auto symbol    = file::get_symbol(*s.file, import.module);
                            const auto intern_rc = string::interned::get(symbol->name);
                            if (!OK(intern_rc.status))
                                return status_t::symbol_not_found;
                            module.name.slice = intern_rc.slice;
                            module.name.rva   = lookup_rva;
                            lookup_rva += module.name.slice.length + 1;
                            size += import_dir_table::entry_size
                                    + (intern_rc.slice.length + 1)
                                    + ((import.symbols.size + 1) * import_lookup_table::entry_size);
                        }
                        module.lookup_rva    = lookup_rva;
                        module.symbols.start = import_table.name_hints.list.size;
                        module.symbols.size  = import.symbols.size;
                        lookup_rva += (import.symbols.size + 1) * import_lookup_table::entry_size;
                        for (auto symbol_id : import.symbols) {
                            const auto symbol    = file::get_symbol(*s.file, symbol_id);
                            const auto intern_rc = string::interned::get(symbol->name);
                            if (!OK(intern_rc.status))
                                return status_t::symbol_not_found;
                            auto& name_hint = array::append(import_table.name_hints.list);
                            name_hint.hint     = 0;
                            name_hint.name     = intern_rc.slice;
                            name_hint.rva.base = name_hint_offset;
                            name_hint.pad      = (name_hint_offset % 2) != 0;
                            name_hint.rva.size = 2 + name_hint.name.length + 1 + (name_hint.pad ? 1 : 0);
                            name_hint_offset += name_hint.rva.size;
                        }
                    }

                    iat_entry.rva.base = import_table_entry.rva.base + size;
                    iat_entry.rva.size = (import_table.name_hints.list.size + 1) * import_addr_table::entry_size;

                    import_table.name_hints.rva.base = iat_entry.rva.base + iat_entry.rva.size;
                    import_table.name_hints.rva.size = name_hint_offset;

                    for (auto& module : import_table.modules)
                        module.iat_rva = iat_entry.rva.base;

                    for (auto& name_hint : import_table.name_hints.list)
                        name_hint.rva.base += import_table.name_hints.rva.base;

                    hdr.size = (import_table.name_hints.rva.base + import_table.name_hints.rva.size)
                        - import_table_entry.rva.base;

                    coff.init_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);

                    break;
                }
                default: {
                    auto status = coff::build_section(s, coff, hdr);
                    if (!OK(status))
                        return status;
                    return status_t::ok;
                }
            }

            auto& section_rec = hdr.symbol->aux_records[0];
            section_rec.section.len = hdr.size;

            return status_t::ok;
        }

        status_t write_section_data(session_t& s, pe_t& pe, coff_section_hdr_t& hdr) {
            const auto type = hdr.section->type;
            if (type == section::type_t::data && !hdr.section->flags.init)
                return status_t::ok;

            session::seek(s, hdr.offset);

            switch (type) {
                case section::type_t::reloc: {
                    return status_t::not_implemented;
                }
                case section::type_t::import: {
                    const auto& iat_entry = pe.dirs[dir_type_t::import_address_table];
                    const auto& import_table_entry = pe.dirs[dir_type_t::import_table];
                    const auto& import_table  = import_table_entry.subclass.import;
                    for (const auto& module : import_table.modules) {
                        session::write_u32(s, module.lookup_rva);
                        session::write_u32(s, 0);
                        session::write_u32(s, 0);
                        session::write_u32(s, module.name.rva);
                        session::write_u32(s, iat_entry.rva.base);
                    }
                    session::write_u32(s, 0);
                    session::write_u32(s, 0);
                    session::write_u32(s, 0);
                    session::write_u32(s, 0);
                    session::write_u32(s, 0);
                    for (const auto& module : import_table.modules) {
                        session::write_cstr(s, module.name.slice);
                        for (const auto& name_hint : import_table.name_hints.list) {
                            pe_thunk_t data{};
                            data.thunk.by_ordinal = false;
                            data.thunk.value      = name_hint.rva.base;
                            session::write_u64(s, data.bits);
                        }
                        session::write_u64(s, 0);
                    }
                    for (const auto& name_hint : import_table.name_hints.list) {
                        pe_thunk_t data{};
                        data.thunk.by_ordinal = false;
                        data.thunk.value      = name_hint.rva.base;
                        session::write_u64(s, data.bits);
                    }
                    session::write_u64(s, 0);
                    for (const auto& name_hint : import_table.name_hints.list) {
                        session::write_u16(s, name_hint.hint);
                        session::write_cstr(s, name_hint.name);
                        if (name_hint.pad)
                            session::write_u8(s, 0);
                    }
                    break;
                }
                case section::type_t::export_: {
                    return status_t::not_implemented;
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
