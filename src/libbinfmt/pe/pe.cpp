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

#include <basecode/core/utf.h>
#include <basecode/core/bits.h>
#include <basecode/binfmt/pe.h>
#include <basecode/core/string.h>
#include <basecode/binfmt/binfmt.h>

namespace basecode::binfmt::io::pe {
    [[maybe_unused]] static u8 s_dos_stub[64] = {
        0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c,
        0xcd, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
        0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65,
        0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
        0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    [[maybe_unused]] constexpr u32 dos_header_size          = 0x40;
    [[maybe_unused]] constexpr u32 pe_sig_size              = 0x04;

    [[maybe_unused]] constexpr u16 pe32                     = 0x10b;
    [[maybe_unused]] constexpr u16 pe64                     = 0x20b;
    [[maybe_unused]] constexpr u16 rom                      = 0x107;

    namespace name_table {
        [[maybe_unused]] constexpr u32 entry_size           = 0x12;
    }

    namespace import_dir_table {
        [[maybe_unused]] constexpr u32 entry_size           = 0x14;
    }

    namespace import_addr_table {
        [[maybe_unused]] constexpr u32 entry_size           = 0x08;
    }

    namespace import_lookup_table {
        [[maybe_unused]] constexpr u32 entry_size           = 0x08;
    }

    namespace subsystem {
        [[maybe_unused]] constexpr u16 unknown              = 0;
        [[maybe_unused]] constexpr u16 native               = 1;
        [[maybe_unused]] constexpr u16 win_gui              = 2;
        [[maybe_unused]] constexpr u16 win_cui              = 3;
        [[maybe_unused]] constexpr u16 os2_cui              = 5;
        [[maybe_unused]] constexpr u16 posix_cui            = 7;
        [[maybe_unused]] constexpr u16 win_native           = 8;
        [[maybe_unused]] constexpr u16 win_ce_gui           = 9;
        [[maybe_unused]] constexpr u16 efi_app              = 10;
        [[maybe_unused]] constexpr u16 efi_boot_svc_driver  = 11;
        [[maybe_unused]] constexpr u16 efi_runtime_driver   = 12;
        [[maybe_unused]] constexpr u16 efi_rom              = 13;
        [[maybe_unused]] constexpr u16 xbox                 = 14;
        [[maybe_unused]] constexpr u16 win_boot_app         = 16;
        [[maybe_unused]] constexpr u16 xbox_code_catalog    = 17;
    }

    namespace dir_entry {
        u0 free(pe_t& pe, dir_type_t type) {
            auto& entry = pe.dirs[u32(type)];
            if (!entry.init)
                return;
            switch (type) {
                case tls_table: {
                    auto& tls = entry.subclass.tls;
                    array::free(tls.callbacks);
                    break;
                }
                case export_table: {
                    auto& export_ = entry.subclass.export_;
                    array::free(export_.exports);
                    break;
                }
                case import_table: {
                    auto& import = entry.subclass.import;
                    array::free(import.modules);
                    array::free(import.name_hints.list);
                    break;
                }
                case resource_table: {
                    auto& resources = entry.subclass.resources;
                    array::free(resources.entries);
                    array::free(resources.blocks);
                    break;
                }
                case base_reloc_table: {
                    auto& base_reloc = entry.subclass.base_reloc;
                    array::free(base_reloc.relocs);
                    break;
                }
                default:break;
            }
        }

        status_t init(pe_t& pe, dir_type_t type) {
            auto& entry = pe.dirs[u32(type)];
            if (entry.init)
                return status_t::ok;
            switch (type) {
                case tls_table: {
                    auto& tls = entry.subclass.tls;
                    array::init(tls.callbacks, pe.coff.alloc);
                    break;
                }
                case export_table: {
                    auto& export_ = entry.subclass.export_;
                    array::init(export_.exports, pe.coff.alloc);
                    break;
                }
                case import_table: {
                    auto& import = entry.subclass.import;
                    array::init(import.modules, pe.coff.alloc);
                    array::init(import.name_hints.list, pe.coff.alloc);
                    break;
                }
                case resource_table: {
                    auto& resources = entry.subclass.resources;
                    array::init(resources.entries, pe.coff.alloc);
                    array::init(resources.blocks, pe.coff.alloc);
                    break;
                }
                case base_reloc_table: {
                    auto& base_reloc = entry.subclass.base_reloc;
                    array::init(base_reloc.relocs);
                    break;
                }
                default:break;
            }
            entry.init = true;
            return status_t::ok;
        }
    }

    u0 free(pe_t& pe) {
        for (u32 dir_type = 0; dir_type < max_dir_entry_count; ++dir_type) {
            dir_entry::free(pe, dir_type_t(dir_type));
        }
        coff::free(pe.coff);
    }

    status_t init(pe_t& pe, const opts_t& opts) {
        auto status = coff::init(pe.coff, *opts.file, opts.alloc);
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
        pe.coff.size.opt_hdr = 0xf0;

        for (u32 dir_type = 0; dir_type < max_dir_entry_count; ++dir_type) {
            auto& dir_entry = pe.dirs[dir_type];
            dir_entry.rva  = {};
            dir_entry.init = false;
            dir_entry.type = dir_type_t(dir_type);
        }

        return status_t::ok;
    }

    status_t write_pe_header(file_t& file, pe_t& pe) {
        FILE_SEEK(pe.start_offset);
        FILE_WRITE(s8, 'P');
        FILE_WRITE(s8, 'E');
        FILE_WRITE0(u16);
        return status_t::ok;
    }

    status_t write_dos_header(file_t& file, pe_t& pe) {
        FILE_WRITE(s8, 'M');
        FILE_WRITE(s8, 'Z');
        FILE_WRITE(u16, u16(pe.size.dos_stub % 512));
        FILE_WRITE(u16, u16(align(pe.size.dos_stub, 512) / 512));
        FILE_WRITE0(u16);
        FILE_WRITE(u16, u16(dos_header_size / 16));
        FILE_WRITE0(u16);
        FILE_WRITE(u16, u16(1));
        FILE_WRITE_PAD(sizeof(u16) * 5);
        FILE_WRITE(u16, u16(dos_header_size));
        FILE_WRITE_PAD(sizeof(u16) * 7);
        FILE_WRITE_PAD(sizeof(u16) * 10);
        FILE_WRITE(u32, pe.start_offset);
        FILE_WRITE_STR(slice::make(s_dos_stub, sizeof(s_dos_stub)));
        return status_t::ok;
    }

    status_t build_sections(file_t& file, pe_t& pe) {
        auto& coff = pe.coff;

        coff.size.headers = coff.offset
                            + (pe_sig_size + coff::header_size + pe.coff.size.opt_hdr)
                            + (coff.headers.size * coff::section::header_size);
        coff.offset = align(coff.size.headers, coff.align.file);
        coff.rva    = align(coff.size.headers, coff.align.section);

        for (auto& hdr : coff.headers) {
            auto status = build_section(file, pe, hdr);
            if (!OK(status))
                return status;
        }

        coff::update_symbol_table(coff);

        return status_t::ok;
    }

    status_t write_sections_data(file_t& file, pe_t& pe) {
        auto& coff = pe.coff;

        for (auto& hdr : coff.headers) {
            auto status = write_section_data(file, pe, hdr);
            if (!OK(status))
                return status;
        }

        return status_t::ok;
    }

    status_t write_optional_header(file_t& file, pe_t& pe) {
        const auto& coff = pe.coff;
        FILE_WRITE(const u16, pe64);
        FILE_WRITE(u8, u8(file.versions.linker.major));
        FILE_WRITE(u8, u8(file.versions.linker.minor));
        FILE_WRITE(u32, coff.code.size);
        FILE_WRITE(u32, coff.init_data.size);
        FILE_WRITE(u32, coff.uninit_data.size);
        FILE_WRITE(u32, coff.code.base);
        FILE_WRITE(u32, coff.code.base);
        FILE_WRITE(u64, pe.base_addr);
        FILE_WRITE(u32, coff.align.section);
        FILE_WRITE(u32, coff.align.file);
        FILE_WRITE(u16, file.versions.min_os.major);
        FILE_WRITE(u16, file.versions.min_os.minor);
        FILE_WRITE0(u32);
        FILE_WRITE(u16, file.versions.min_os.major);
        FILE_WRITE(u16, file.versions.min_os.minor);
        FILE_WRITE0(u32);
        FILE_WRITE(u32, u32(align(coff.rva + coff.size.image, coff.align.section)));
        FILE_WRITE(u32, u32(align(coff.size.headers, pe.coff.align.file)));
        FILE_WRITE0(u32);
        FILE_WRITE(u16, u16(file.flags.console ? subsystem::win_cui : subsystem::win_gui));
        FILE_WRITE(u16, pe.flags.dll);
        FILE_WRITE(u64, pe.reserve.stack);
        FILE_WRITE(u64, 0x1000UL);
        FILE_WRITE(u64, pe.reserve.heap);
        FILE_WRITE(u64, 0x1000UL);
        FILE_WRITE(u32, pe.flags.load);
        FILE_WRITE(const u32, max_dir_entry_count);

        for (const auto& dir : pe.dirs) {
            FILE_WRITE(u32, dir.rva.base);
            FILE_WRITE(u32, dir.rva.size);
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
    status_t build_section(file_t& file, pe_t& pe, coff::header_t& hdr) {
        auto& coff = pe.coff;

        hdr.file.offset = coff.offset;
        hdr.rva.base    = coff.rva;

        switch (hdr.section->type) {
            case section::type_t::reloc:
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
                u32 size                    = import_dir_table::entry_size;
                u32 lookup_rva              = import_table_entry.rva.base + import_table_entry.rva.size;
                for (const auto& import : imports) {
                    auto& module = array::append(import_table.modules);
                    module.iat_rva = module.fwd_chain = module.time_stamp = {};
                    {
                        // XXX: FIXME!
//                        const auto symbol    = module::get_symbol(*file.module, import.module_symbol);
                        const auto intern_rc = string::interned::get(0); // XXX: FIXME!
                        if (!OK(intern_rc.status))
                            return status_t::symbol_not_found;
                        module.name.slice    = intern_rc.slice;
                        module.name.rva      = lookup_rva;
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
                        // XXX: FIXME!
//                        const auto symbol    = module::get_symbol(*file.module, symbol_id);
                        const auto intern_rc = string::interned::get(0);    // XXX: FIXME!
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

                for (auto& module : import_table.modules) {
                    module.iat_rva = iat_entry.rva.base;
                }

                for (auto& name_hint : import_table.name_hints.list) {
                    name_hint.rva.base += import_table.name_hints.rva.base;
                }

                hdr.rva.size = (import_table.name_hints.rva.base + import_table.name_hints.rva.size)
                           - import_table_entry.rva.base;
                hdr.file.size = align(hdr.rva.size, coff.align.file);

                coff.init_data.size += hdr.rva.size;
                coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                coff.offset     = align(coff.offset + hdr.rva.size, coff.align.file);
                coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);

                break;
            }
            default: {
                auto status = coff::build_section(file, coff, hdr);
                if (!OK(status))
                    return status;
                return status_t::ok;
            }
        }

        coff::set_section_flags(file, hdr);

        auto sym = coff::section::get_symbol(coff, hdr);
        auto aux_sec = coff::symtab::get_aux(coff, sym, 0);
        if (aux_sec)
            aux_sec->subclass.aux_section.len = hdr.rva.size;

        return status_t::ok;
    }

    status_t write_section_data(file_t& file, pe_t& pe, coff::header_t& hdr) {
        const auto type = hdr.section->type;
        if (type == section::type_t::bss)
            return status_t::ok;

        FILE_SEEK(hdr.file.offset);

        switch (type) {
            case section::type_t::reloc: {
                return status_t::not_implemented;
            }
            case section::type_t::import: {
                const auto     & iat_entry          = pe.dirs[dir_type_t::import_address_table];
                const auto     & import_table_entry = pe.dirs[dir_type_t::import_table];
                const auto     & import_table       = import_table_entry.subclass.import;
                for (const auto& module : import_table.modules) {
                    FILE_WRITE(u32, module.lookup_rva);
                    FILE_WRITE0(u64);
                    FILE_WRITE(u32, module.name.rva);
                    FILE_WRITE(u32, iat_entry.rva.base);
                }
                FILE_WRITE_PAD(sizeof(u32) * 5);
                for (const auto& module : import_table.modules) {
                    FILE_WRITE_CSTR(module.name.slice);
                    for (const auto& name_hint : import_table.name_hints.list) {
                        thunk_t data{};
                        data.thunk.by_ordinal = false;
                        data.thunk.value      = name_hint.rva.base;
                        FILE_WRITE(u64, data.bits);
                    }
                    FILE_WRITE0(u64);
                }
                for (const auto& name_hint : import_table.name_hints.list) {
                    thunk_t data{};
                    data.thunk.by_ordinal = false;
                    data.thunk.value      = name_hint.rva.base;
                    FILE_WRITE(u64, data.bits);
                }
                FILE_WRITE0(u64);
                for (const auto& name_hint : import_table.name_hints.list) {
                    FILE_WRITE(u16, name_hint.hint);
                    FILE_WRITE_CSTR(name_hint.name);
                    if (name_hint.pad)
                        FILE_WRITE0(u8);
                }
                break;
            }
            case section::type_t::export_: {
                return status_t::not_implemented;
            }
            default: {
                auto status = coff::write_section_data(file, pe.coff, hdr);
                if (!OK(status))
                    return status;
            }
        }

        return status_t::ok;
    }
}
