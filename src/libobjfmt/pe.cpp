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

namespace basecode::objfmt::container::pe {
    [[maybe_unused]] static u8 s_dos_stub[64] = {
        0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c,
        0xcd, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
        0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65,
        0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
        0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    static str::slice_t s_section_names[] = {
        [u32(section::type_t::code)]        = ".text"_ss,
        [u32(section::type_t::data)]        = ".rdata"_ss,
        [u32(section::type_t::debug)]       = ".debug"_ss,
        [u32(section::type_t::uninit)]      = ".bss"_ss,
        [u32(section::type_t::import)]      = ".idata"_ss,
        [u32(section::type_t::export_)]     = ".edata"_ss,
        [u32(section::type_t::resource)]    = ".rsrc"_ss,
    };

    [[maybe_unused]] constexpr u32 dos_header_size              = 0x40;
    [[maybe_unused]] constexpr u32 pe_sig_size                  = 0x04;
    [[maybe_unused]] constexpr u32 coff_header_size             = 0x14;
    [[maybe_unused]] constexpr u32 optional_header_size         = 0xf0;
    [[maybe_unused]] constexpr u32 pe_header_size               = pe_sig_size + coff_header_size + optional_header_size;
    [[maybe_unused]] constexpr u32 section_header_size          = 0x28;
    [[maybe_unused]] constexpr u32 imp_addr_entry_size          = 0x08;
    [[maybe_unused]] constexpr u32 imp_dir_entry_size           = 0x14;
    [[maybe_unused]] constexpr u32 imp_lookup_entry_size        = imp_addr_entry_size;
    [[maybe_unused]] constexpr u32 name_table_entry_size        = 0x12;

    [[maybe_unused]] constexpr u16 pe32                         = 0x10b;
    [[maybe_unused]] constexpr u16 pe64                         = 0x20b;
    [[maybe_unused]] constexpr u16 rom                          = 0x107;

    [[maybe_unused]] constexpr u32 link_reloc_overflow          = 0x01000000;
    [[maybe_unused]] constexpr u32 memory_can_discard           = 0x02000000;
    [[maybe_unused]] constexpr u32 memory_not_cached            = 0x04000000;
    [[maybe_unused]] constexpr u32 memory_not_paged             = 0x08000000;
    [[maybe_unused]] constexpr u32 memory_shared                = 0x10000000;
    [[maybe_unused]] constexpr u32 memory_execute               = 0x20000000;
    [[maybe_unused]] constexpr u32 memory_read                  = 0x40000000;
    [[maybe_unused]] constexpr u32 memory_write                 = 0x80000000;

    [[maybe_unused]] constexpr u32 section_code                 = 0x00000020;
    [[maybe_unused]] constexpr u32 section_init_data            = 0x00000040;
    [[maybe_unused]] constexpr u32 section_uninit_data          = 0x00000080;
    [[maybe_unused]] constexpr u32 section_align_1              = 0x00100000;
    [[maybe_unused]] constexpr u32 section_align_2              = 0x00200000;
    [[maybe_unused]] constexpr u32 section_align_4              = 0x00300000;
    [[maybe_unused]] constexpr u32 section_align_8              = 0x00400000;
    [[maybe_unused]] constexpr u32 section_align_16             = 0x00500000; // default
    [[maybe_unused]] constexpr u32 section_align_32             = 0x00600000;
    [[maybe_unused]] constexpr u32 section_align_64             = 0x00700000;
    [[maybe_unused]] constexpr u32 section_align_128            = 0x00800000;
    [[maybe_unused]] constexpr u32 section_align_256            = 0x00900000;
    [[maybe_unused]] constexpr u32 section_align_512            = 0x00a00000;
    [[maybe_unused]] constexpr u32 section_align_1024           = 0x00b00000;
    [[maybe_unused]] constexpr u32 section_align_2048           = 0x00c00000;
    [[maybe_unused]] constexpr u32 section_align_4096           = 0x00d00000;
    [[maybe_unused]] constexpr u32 section_align_8192           = 0x00e00000;

    [[maybe_unused]] constexpr u16 subsys_unknown               = 0;
    [[maybe_unused]] constexpr u16 subsys_native                = 1;
    [[maybe_unused]] constexpr u16 subsys_win_gui               = 2;
    [[maybe_unused]] constexpr u16 subsys_win_cui               = 3;
    [[maybe_unused]] constexpr u16 subsys_os2_cui               = 5;
    [[maybe_unused]] constexpr u16 subsys_posix_cui             = 7;
    [[maybe_unused]] constexpr u16 subsys_win_native            = 8;
    [[maybe_unused]] constexpr u16 subsys_win_ce_gui            = 9;
    [[maybe_unused]] constexpr u16 subsys_efi_app               = 10;
    [[maybe_unused]] constexpr u16 subsys_efi_boot_svc_driver   = 11;
    [[maybe_unused]] constexpr u16 subsys_efi_runtime_driver    = 12;
    [[maybe_unused]] constexpr u16 subsys_efi_rom               = 13;
    [[maybe_unused]] constexpr u16 subsys_xbox                  = 14;
    [[maybe_unused]] constexpr u16 subsys_win_boot_app          = 16;
    [[maybe_unused]] constexpr u16 subsys_xbox_code_catalog     = 17;

    namespace image {
        namespace flags {
            [[maybe_unused]] constexpr u32 relocs_stripped              = 0x0001;
            [[maybe_unused]] constexpr u32 executable_type              = 0x0002;
            [[maybe_unused]] constexpr u32 line_nums_stripped           = 0x0004;
            [[maybe_unused]] constexpr u32 local_syms_stripped          = 0x0008;
            [[maybe_unused]] constexpr u32 aggressive_ws_trim           = 0x0010;
            [[maybe_unused]] constexpr u32 large_address_aware          = 0x0020;
            [[maybe_unused]] constexpr u32 reserved                     = 0x0040;
            [[maybe_unused]] constexpr u32 bytes_reversed_lo            = 0x0080;
            [[maybe_unused]] constexpr u32 machine_32bit                = 0x0100;
            [[maybe_unused]] constexpr u32 debug_stripped               = 0x0200;
            [[maybe_unused]] constexpr u32 removable_run_from_swap      = 0x0400;
            [[maybe_unused]] constexpr u32 net_run_from_swap            = 0x0800;
            [[maybe_unused]] constexpr u32 system_type                  = 0x1000;
            [[maybe_unused]] constexpr u32 dll_type                     = 0x2000;
            [[maybe_unused]] constexpr u32 up_system_only               = 0x4000;
            [[maybe_unused]] constexpr u32 bytes_reversed_hi            = 0x8000;
        }
    }

    namespace pe {
        u0 free(pe_t& pe) {
            buf::free(pe.buf);
        }

        u0 write_pad(pe_t& pe) {
            if ((pe.crsr.pos % 2) == 0) return;
            buf::cursor::write_u8(pe.crsr, 0);
        }

        u0 seek(pe_t& pe, u32 offset) {
            buf::cursor::seek(pe.crsr, offset);
        }

        u0 write_dos_header(pe_t& pe) {
            const auto dos_stub_size = dos_header_size + sizeof(s_dos_stub);
            pe.offset = align(dos_stub_size, 8);

            pe::write_u8(pe, 'M');
            pe::write_u8(pe, 'Z');
            pe::write_u16(pe, dos_stub_size % 512);
            pe::write_u16(pe, align(dos_stub_size, 512) / 512);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, dos_header_size / 16);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 1);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, dos_header_size);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            pe::write_u16(pe, 0);
            for (u32 i = 0; i < 10; ++i)
                pe::write_u16(pe, 0);
            pe::write_u32(pe, pe.offset);
            for (u8 code_byte : s_dos_stub)
                pe::write_u8(pe, code_byte);
        }

        u0 write_u8(pe_t& pe, u8 value) {
            buf::cursor::write_u8(pe.crsr, value);
        }

        u0 write_u16(pe_t& pe, u16 value) {
            buf::cursor::write_u16(pe.crsr, value);
        }

        u0 write_u32(pe_t& pe, u32 value) {
            buf::cursor::write_u32(pe.crsr, value);
        }

        u0 write_u64(pe_t& pe, u64 value) {
            buf::cursor::write_u64(pe.crsr, value);
        }

        status_t save(pe_t& pe, const path_t& path) {
            auto status = buf::save(pe.buf, path);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        u0 write_pad8(pe_t& pe, str::slice_t slice) {
            for (u32 i = 0; i < 8; ++i)
                buf::cursor::write_u8(pe.crsr, i < slice.length ? slice[i] : 0);
        }

        u0 write_cstr(pe_t& pe, str::slice_t slice) {
            for (u32 i = 0; i < slice.length; ++i)
                buf::cursor::write_u8(pe.crsr, slice[i]);
            buf::cursor::write_u8(pe.crsr, 0);
        }

        u0 write_pad16(pe_t& pe, str::slice_t slice) {
            for (u32 i = 0; i < 16; ++i)
                buf::cursor::write_u8(pe.crsr, i < slice.length ? slice[i] : 0);
        }

        u0 write_optional_header(pe_t& pe, const context_t& ctx) {
            pe::write_u16(pe, pe64);
            pe::write_u8(pe, ctx.versions.linker.major);
            pe::write_u8(pe, ctx.versions.linker.minor);
            pe::write_u32(pe, pe.code.size);
            pe::write_u32(pe, pe.init_data.size);
            pe::write_u32(pe, pe.uninit_data.size);
            pe::write_u32(pe, pe.code.base);
            pe::write_u32(pe, pe.code.base);
            pe::write_u64(pe, pe.base_addr);
            pe::write_u32(pe, pe.align.section);
            pe::write_u32(pe, pe.align.file);
            pe::write_u16(pe, ctx.versions.min_os.major);
            pe::write_u16(pe, ctx.versions.min_os.minor);
            pe::write_u16(pe, 0);            // XXX: image ver major
            pe::write_u16(pe, 0);            // XXX: image ver minor
            pe::write_u16(pe, ctx.versions.min_os.major);
            pe::write_u16(pe, ctx.versions.min_os.minor);
            pe::write_u32(pe, 0);
            pe::write_u32(pe, align(pe.rva + pe.size.image, pe.align.section));
            pe::write_u32(pe, align(pe.size.headers, pe.align.file));
            pe::write_u32(pe, 0);
            if (ctx.flags.console) {
                pe::write_u16(pe, subsys_win_cui);
            } else {
                pe::write_u16(pe, subsys_win_gui);
            }
            pe::write_u16(pe, 0);            // XXX: dll flags
            pe::write_u64(pe, 0x100000);     // XXX: size of stack reserve
            pe::write_u64(pe, 0x1000);       // XXX: size of stack commit
            pe::write_u64(pe, 0x100000);     // XXX: size of heap reserve
            pe::write_u64(pe, 0x1000);       // XXX: size of heap commit
            pe::write_u32(pe, 0);            // XXX: load flags
            pe::write_u32(pe, 16);           // XXX: number of rva and sizes

            for (const auto& dir : pe.dirs) {
                pe::write_u32(pe, dir.rva);
                pe::write_u32(pe, dir.size);
            }
        }

        u0 write_section_headers(pe_t& pe, const context_t& ctx) {
            const auto num_sections = ctx.file->sections.size;
            for (u32 i = 0; i < num_sections; ++i) {
                const auto& hdr = pe.hdrs[i];
                const auto type = hdr.section->type;
                const auto& flags = hdr.section->flags;

                if (type == section::type_t::custom) {
                    const auto symbol = objfmt::file::get_symbol(*ctx.file, hdr.section->symbol);
                    const auto intern_rc = string::interned::get(symbol->name);
                    pe::write_pad8(pe, intern_rc.slice);
                } else {
                    pe::write_pad8(pe, s_section_names[u32(type)]);
                }
                pe::write_u32(pe, hdr.size);
                pe::write_u32(pe, hdr.rva);
                if (type == section::type_t::uninit) {
                    pe::write_u32(pe, 0);
                    pe::write_u32(pe, 0);
                } else {
                    pe::write_u32(pe, align(hdr.size, pe.align.file));
                    pe::write_u32(pe, hdr.offset);
                }
                pe::write_u32(pe, 0);         // XXX: pointer to relocs
                pe::write_u32(pe, 0);         // N.B. always null
                pe::write_u16(pe, 0);         // XXX: number of relocs
                pe::write_u16(pe, 0);         // N.B. always zero

                u32 bitmask{};
                if (flags.code)     bitmask |= section_code;
                if (flags.data) {
                    if (type == section::type_t::data
                        ||  type == section::type_t::import) {
                        bitmask |= section_init_data;
                    } else if (type == section::type_t::uninit) {
                        bitmask |= section_uninit_data;
                    }
                }
                if (flags.read)     bitmask |= memory_read;
                if (flags.write)    bitmask |= memory_write;
                if (flags.exec)     bitmask |= memory_execute;

                pe::write_u32(pe, bitmask);
            }
        }

        status_t prepare_sections(pe_t& pe, const context_t& ctx) {
            pe.size.headers = pe.offset
                              + pe_header_size
                              + (ctx.file->sections.size * section_header_size);
            pe.offset = pe.size.headers;
            pe.rva    = pe.size.headers;

            const auto num_sections = ctx.file->sections.size;
            for (u32 i = 0; i < num_sections; ++i) {
                auto& hdr = pe.hdrs[i];
                pe.offset = hdr.offset = align(pe.offset + pe.size.image, pe.align.file);
                pe.rva    = hdr.rva    = align(pe.rva + pe.size.image, pe.align.section);
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
                        import_address_table.rva = pe.rva;
                        u32 imported_count{};
                        for (const auto& import : imports)
                            imported_count += import.symbols.size;
                        import_address_table.size = (imported_count + 1) * imp_addr_entry_size;
                        pe.rva += import_address_table.size;

                        import_table.rva = pe.rva;
                        import_table.size = imp_dir_entry_size * 2;
                        pe.rva += import_table.size;

                        pe.import_lookup_table.rva = pe.rva;
                        pe.rva += import_address_table.size;

                        pe.name_table.rva = pe.rva;
                        pe.name_table.size = (imported_count + 1) * name_table_entry_size;
                        pe.rva += pe.name_table.size;

                        pe.size.image = hdr.size = pe.rva - import_address_table.rva;
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

        u0 write_section_data(pe_t& pe, const context_t& ctx) {
            const auto num_sections = ctx.file->sections.size;
            for (u32 i = 0; i < num_sections; ++i) {
                const auto& hdr = pe.hdrs[i];
                const auto type = hdr.section->type;
                const auto& sc = hdr.section->subclass;
                if (type == section::type_t::uninit)
                    continue;
                pe::seek(pe, hdr.offset);
                switch (type) {
                    case section::type_t::code:
                    case section::type_t::data:
                        pe::write_str(pe, sc.data);
                        break;
                    case section::type_t::import: {
                        const auto& imports = sc.imports;
                        for (const auto& import : imports) {
                            const auto module_symbol = file::get_symbol(*ctx.file, import.module);
                            const auto module_intern = string::interned::get(module_symbol->name);

                            // import address table
                            u32 name_table_ptr = pe.name_table.rva + name_table_entry_size;
                            for (u32 i = 0; i < import.symbols.size; ++i) {
                                pe::write_u64(pe, name_table_ptr);
                                name_table_ptr += name_table_entry_size;
                            }
                            pe::write_u64(pe, 0);

                            // import directory table
                            pe::write_u32(pe, pe.import_lookup_table.rva);
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, pe.name_table.rva);
                            pe::write_u32(pe, pe.dirs[dir_type_t::import_address_table].rva);

                            // null node
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, 0);
                            pe::write_u32(pe, 0);

                            // import lookup table
                            name_table_ptr = pe.name_table.rva + name_table_entry_size;
                            for (u32 i = 0; i < import.symbols.size; ++i) {
                                pe::write_u64(pe, name_table_ptr);
                                name_table_ptr += name_table_entry_size;
                            }
                            pe::write_u64(pe, 0);

                            // name table
                            pe::write_pad16(pe, module_intern.slice);
                            pe::write_u8(pe, 0);
                            pe::write_u8(pe, 0);
                            for (auto symbol_id : import.symbols) {
                                const auto symbol = file::get_symbol(*ctx.file, symbol_id);
                                const auto symbol_intern = string::interned::get(symbol->name);
                                pe::write_u16(pe, 0);    // XXX: hint
                                pe::write_pad16(pe, symbol_intern.slice);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        u0 write_pe_and_coff_header(pe_t& pe, const context_t& ctx) {
            pe::seek(pe, pe.offset);
            pe::write_u8(pe, 'P');
            pe::write_u8(pe, 'E');
            pe::write_u8(pe, 0);
            pe::write_u8(pe, 0);
            pe::write_u16(pe, u16(ctx.file->machine));
            pe::write_u16(pe, ctx.file->sections.size);
            pe::write_u32(pe, 0);
            pe::write_u32(pe, 0);
            pe::write_u32(pe, 0);
            pe::write_u16(pe, optional_header_size);
            pe::write_u16(pe,
                          image::flags::relocs_stripped
                          | image::flags::executable_type
                          | image::flags::large_address_aware);
        }

        status_t init(pe_t& pe, coff::section_hdr_t* hdrs, alloc_t* alloc) {
            pe.hdrs          = hdrs;
            pe.rva           = pe.offset    = {};
            pe.size          = {};
            pe.base_addr     = 0x140000000;
            pe.align.file    = 0x200;
            pe.align.section = 0x1000;
            pe.code          = pe.init_data = pe.uninit_data = {};
            ZERO_MEM(pe.dirs, pe.dirs);
            buf::init(pe.buf, alloc);
            // XXX: revisit
            buf::reserve(pe.buf, 64 * 1024);
            buf::cursor::init(pe.crsr, pe.buf);
            return status_t::ok;
        }
    }

    static u0 fini() {
    }

    static status_t init(alloc_t* alloc) {
        UNUSED(alloc);
        return status_t::ok;
    }

    static status_t read(const context_t& ctx) {
        UNUSED(ctx);
        return status_t::ok;
    }

    static status_t write(const context_t& ctx) {
        coff::section_hdr_t hdrs[ctx.file->sections.size];
        for (u32 i = 0; i < ctx.file->sections.size; ++i) {
            auto& hdr = hdrs[i];
            hdr.section = &ctx.file->sections[i];
            hdr.rva     = hdr.offset = hdr.size = {};
            hdr.number  = i + 1;
        }

        pe_t pe{};
        pe::init(pe, hdrs, ctx.file->alloc);
        defer(pe::free(pe));

        pe::write_dos_header(pe);
        pe::write_pe_and_coff_header(pe, ctx);
        pe::prepare_sections(pe, ctx);
        pe::write_optional_header(pe, ctx);
        pe::write_section_headers(pe, ctx);
        pe::write_section_data(pe, ctx);

        auto status = pe::save(pe, ctx.file->path);
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

    system_t* system() {
        return &g_pe_sys;
    }
}
