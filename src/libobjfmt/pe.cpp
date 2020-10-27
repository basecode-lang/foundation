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

#include <basecode/core/buf.h>
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
    [[maybe_unused]] constexpr u64 image_base_addr              = 0x140000000;    //0x00400000;   // XXX: check this
    [[maybe_unused]] constexpr u32 section_align                = 0x1000;       // XXX: this seems massive!
    [[maybe_unused]] constexpr u32 file_align                   = 0x200;

    [[maybe_unused]] constexpr u32 relocations_stripped         = 0x0001;
    [[maybe_unused]] constexpr u32 executable_image             = 0x0002;

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

    static u0 write_pad(buf_crsr_t& crsr) {
        if ((crsr.pos % 2) == 0) return;
        buf::cursor::write_u8(crsr, 0);
    }

    static u0 write_pad8(buf_crsr_t& crsr, str::slice_t slice) {
        for (u32 i = 0; i < 8; ++i)
            buf::cursor::write_u8(crsr, i < slice.length ? slice[i] : 0);
    }

    static u0 write_cstr(buf_crsr_t& crsr, str::slice_t slice) {
        for (u32 i = 0; i < slice.length; ++i)
            buf::cursor::write_u8(crsr, slice[i]);
        buf::cursor::write_u8(crsr, 0);
    }

    static u0 write_pad16(buf_crsr_t& crsr, str::slice_t slice) {
        for (u32 i = 0; i < 16; ++i)
            buf::cursor::write_u8(crsr, i < slice.length ? slice[i] : 0);
    }

    static u0 fini() {
    }

    static status_t read(file_t& file) {
        UNUSED(file);
        return status_t::ok;
    }

    static status_t write(file_t& file) {
        buf_t buf{};
        buf::init(buf, file.alloc);
        defer(buf::free(buf));

        buf_crsr_t crsr{};
        buf::cursor::init(crsr, buf);

        const auto dos_stub_size = dos_header_size + sizeof(s_dos_stub);
        const auto pe_offset     = align(dos_stub_size, 8);

        buf::cursor::write_u8(crsr, 'M');
        buf::cursor::write_u8(crsr, 'Z');
        buf::cursor::write_u16(crsr, dos_stub_size % 512);
        buf::cursor::write_u16(crsr, align(dos_stub_size, 512) / 512);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, dos_header_size / 16);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 1);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, dos_header_size);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u16(crsr, 0);
        for (u32 i = 0; i < 10; ++i)
            buf::cursor::write_u16(crsr, 0);
        buf::cursor::write_u32(crsr, pe_offset);
        for (u8 code_byte : s_dos_stub)
            buf::cursor::write_u8(crsr, code_byte);

        buf::cursor::seek(crsr, pe_offset);
        buf::cursor::write_u8(crsr, 'P');
        buf::cursor::write_u8(crsr, 'E');
        buf::cursor::write_u8(crsr, 0);
        buf::cursor::write_u8(crsr, 0);
        buf::cursor::write_u16(crsr, u16(file.machine));
        buf::cursor::write_u16(crsr, u16(file.sections.size));
        buf::cursor::write_u32(crsr, 0);
        buf::cursor::write_u32(crsr, 0);
        buf::cursor::write_u32(crsr, 0);
        buf::cursor::write_u16(crsr, optional_header_size);
        buf::cursor::write_u16(crsr, 0x022F /*relocations_stripped | executable_image*/);

        const auto headers_size = pe_offset + pe_header_size + (file.sections.size * section_header_size);

        u32 section_num         = 1;
        u64 offset              = headers_size;
        u64 rva                 = headers_size;
        u64 size                = {};
        u32 base_of_code        = {};
        u32 base_of_data        = {};
        u32 size_of_code        = {};
        u32 size_of_init_data   = {};
        u32 size_of_uninit_data = {};
        u32 iat_rva             = {};
        u32 iat_size            = {};
        u32 name_tbl_rva        = {};
        u32 name_tbl_size       = {};
        u32 imp_dir_tbl_rva     = {};
        u32 imp_dir_tbl_size    = {};
        u32 imp_lookup_tbl_rva  = {};

        for (auto& section : file.sections) {
            offset = section.offset           = align(offset + size, file_align);
            rva    = section.address.virtual_ = align(rva + size, section_align);
            switch (section.type) {
                case section::type_t::code:
                    if (!base_of_code)
                        base_of_code = section.address.virtual_;
                    size = section.size = section.subclass.data.length;
                    size_of_code += size;
                    break;
                case section::type_t::data:
                    if (!base_of_data)
                        base_of_data = section.address.virtual_;
                    size = section.size = section.subclass.data.length;
                    size_of_init_data += size;
                    break;
                case section::type_t::uninit:
                    size = section.size = section.subclass.size;
                    size_of_uninit_data += size;
                    break;
                case section::type_t::import: {
                    const auto& imports = section.subclass.imports;
                    iat_rva  = rva;
                    u32 imported_count{};
                    for (const auto& import : imports)
                        imported_count += import.symbols.size;
                    iat_size = (imported_count + 1) * imp_addr_entry_size;
                    rva += iat_size;

                    imp_dir_tbl_rva = rva;
                    imp_dir_tbl_size = imp_dir_entry_size * 2;
                    rva += imp_dir_tbl_size;

                    imp_lookup_tbl_rva = rva;
                    rva += iat_size;

                    name_tbl_rva = rva;
                    name_tbl_size = (imported_count + 1) * name_table_entry_size;
                    rva += name_tbl_size;

                    size = section.size = rva - iat_rva;
                    size_of_init_data += size;
                    break;
                }
            }
            ++section_num;
        }

        buf::cursor::write_u16(crsr, pe64);
        buf::cursor::write_u8(crsr, 6);     // XXX: linker major
        buf::cursor::write_u8(crsr, 0);     // XXX: linker minor
        buf::cursor::write_u32(crsr, size_of_code);
        buf::cursor::write_u32(crsr, size_of_init_data);
        buf::cursor::write_u32(crsr, size_of_uninit_data);
        buf::cursor::write_u32(crsr, base_of_code);
        buf::cursor::write_u32(crsr, base_of_code);
        buf::cursor::write_u64(crsr, image_base_addr);
        buf::cursor::write_u32(crsr, section_align);
        buf::cursor::write_u32(crsr, file_align);
        buf::cursor::write_u16(crsr, 4);            // XXX: OS ver major
        buf::cursor::write_u16(crsr, 0);            // XXX: OS ver minor
        buf::cursor::write_u16(crsr, 0);            // XXX: image ver major
        buf::cursor::write_u16(crsr, 0);            // XXX: image ver minor
        buf::cursor::write_u16(crsr, 4);            // XXX: subsystem ver major
        buf::cursor::write_u16(crsr, 0);            // XXX: subsystem ver minor
        buf::cursor::write_u32(crsr, 0);
        buf::cursor::write_u32(crsr, align(rva + size, section_align));
        buf::cursor::write_u32(crsr, align(headers_size, file_align));
        buf::cursor::write_u32(crsr, 0);
        buf::cursor::write_u16(crsr, subsys_win_cui);     // XXX: subsystem / console
        buf::cursor::write_u16(crsr, 0);            // XXX: dll flags
        buf::cursor::write_u64(crsr, 0x100000);     // XXX: size of stack reserve
        buf::cursor::write_u64(crsr, 0x1000);       // XXX: size of stack commit
        buf::cursor::write_u64(crsr, 0x100000);     // XXX: size of heap reserve
        buf::cursor::write_u64(crsr, 0x1000);       // XXX: size of heap commit
        buf::cursor::write_u32(crsr, 0);            // XXX: load flags
        buf::cursor::write_u32(crsr, 16);           // XXX: number of rva and sizes

        // windows data directories
        buf::cursor::write_u32(crsr, 0);            // XXX: export table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: export table size

        buf::cursor::write_u32(crsr, imp_dir_tbl_rva);    // XXX: import table pointer
        buf::cursor::write_u32(crsr, imp_dir_tbl_size);   // XXX: import table size

        buf::cursor::write_u32(crsr, 0);            // XXX: resource table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: resource table size

        buf::cursor::write_u32(crsr, 0);            // XXX: exception table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: exception table size

        buf::cursor::write_u32(crsr, 0);            // XXX: certificate table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: certificate table size

        buf::cursor::write_u32(crsr, 0);            // XXX: base relocation table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: base relocation table size

        buf::cursor::write_u32(crsr, 0);            // XXX: debug table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: debug table size

        buf::cursor::write_u32(crsr, 0);            // XXX: architecture table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: architecture table size

        buf::cursor::write_u32(crsr, 0);            // XXX: global table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: global table size

        buf::cursor::write_u32(crsr, 0);            // XXX: tls table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: tls table size

        buf::cursor::write_u32(crsr, 0);            // XXX: load config table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: load config table size

        buf::cursor::write_u32(crsr, 0);            // XXX: bound import table pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: bound import table size

        buf::cursor::write_u32(crsr, iat_rva);            // XXX: import address table pointer
        buf::cursor::write_u32(crsr, iat_size);           // XXX: import address table size

        buf::cursor::write_u32(crsr, 0);            // XXX: delay import descriptor pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: delay import descriptor size

        buf::cursor::write_u32(crsr, 0);            // XXX: clr runtime header pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: clr runtime header size

        buf::cursor::write_u32(crsr, 0);            // XXX: reserved pointer
        buf::cursor::write_u32(crsr, 0);            // XXX: reserved size

        // section headers
        for (const auto& section : file.sections) {
            const auto symbol = objfmt::file::get_symbol(file, section.symbol);
            const auto intern_rc = string::interned::get(symbol->name);
            write_pad8(crsr, intern_rc.slice);
            buf::cursor::write_u32(crsr, section.size);
            buf::cursor::write_u32(crsr, section.address.virtual_);
            if (section.type == section::type_t::uninit) {
                buf::cursor::write_u32(crsr, 0);
                buf::cursor::write_u32(crsr, 0);
            } else {
                buf::cursor::write_u32(crsr, align(section.size, file_align));
                buf::cursor::write_u32(crsr, section.offset);
            }
            buf::cursor::write_u32(crsr, 0);         // XXX: pointer to relocs
            buf::cursor::write_u32(crsr, 0);         // XXX: pointer to line numbers
            buf::cursor::write_u16(crsr, 0);         // XXX: number of relocs
            buf::cursor::write_u16(crsr, 0);         // XXX: number of line numbers

            u32 flags{};
            if (section.flags.code)     flags |= section_code;
            if (section.flags.data) {
                if (section.type == section::type_t::data
                ||  section.type == section::type_t::import) {
                    flags |= section_init_data;
                } else if (section.type == section::type_t::uninit) {
                    flags |= section_uninit_data;
                }
            }
            if (section.flags.read)     flags |= memory_read;
            if (section.flags.write)    flags |= memory_write;
            if (section.flags.exec)     flags |= memory_execute;

            buf::cursor::write_u32(crsr, flags);
        }

        // section data
        for (const auto& section : file.sections) {
            if (section.type == section::type_t::uninit)
                continue;
            buf::cursor::seek(crsr, section.offset);
            switch (section.type) {
                case section::type_t::code:
                case section::type_t::data:
                    buf::cursor::write_str(crsr, section.subclass.data);
                    break;
                case section::type_t::import: {
                    const auto& imports = section.subclass.imports;
                    for (const auto& import : imports) {
                        const auto module_symbol = file::get_symbol(file, import.module);
                        const auto module_intern = string::interned::get(module_symbol->name);

                        // import address table
                        format::print("name_tbl_rva = 0x{:08x}\n", name_tbl_rva);
                        u32 name_table_ptr = name_tbl_rva + name_table_entry_size;
                        for (u32 i = 0; i < import.symbols.size; ++i) {
                            format::print("name_table_ptr = 0x{:08x}\n", name_table_ptr);
                            buf::cursor::write_u64(crsr, name_table_ptr);
                            name_table_ptr += name_table_entry_size;
                        }
                        buf::cursor::write_u64(crsr, 0);

                        // import directory table
                        buf::cursor::write_u32(crsr, imp_lookup_tbl_rva);
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, name_tbl_rva);
                        buf::cursor::write_u32(crsr, iat_rva);

                        // null node
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, 0);
                        buf::cursor::write_u32(crsr, 0);

                        // import lookup table
                        name_table_ptr = name_tbl_rva + name_table_entry_size;
                        for (u32 i = 0; i < import.symbols.size; ++i) {
                            format::print("name_table_ptr = 0x{:08x}\n", name_table_ptr);
                            buf::cursor::write_u64(crsr, name_table_ptr);
                            name_table_ptr += name_table_entry_size;
                        }
                        buf::cursor::write_u64(crsr, 0);

                        // name table
                        write_pad16(crsr, module_intern.slice);
                        buf::cursor::write_u8(crsr, 0);
                        buf::cursor::write_u8(crsr, 0);
                        for (auto symbol_id : import.symbols) {
                            const auto symbol = file::get_symbol(file, symbol_id);
                            const auto symbol_intern = string::interned::get(symbol->name);
                            buf::cursor::write_u16(crsr, 0);    // XXX: hint
                            write_pad16(crsr, symbol_intern.slice);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        auto status = buf::save(buf, file.path);
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

    system_t* system() {
        return &g_pe_sys;
    }
}
