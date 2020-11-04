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
#include <basecode/binfmt/coff.h>
#include <basecode/core/string.h>

namespace basecode::binfmt::io::coff {
    namespace string_table {
        u0 free(coff_t& coff) {
            array::free(coff.string_table.list);
        }

        u0 init(coff_t& coff, alloc_t* alloc) {
            coff.string_table.file.offset = 0;
            coff.string_table.file.size   = sizeof(u32);
            array::init(coff.string_table.list, alloc);
        }

        u32 add(coff_t& coff, str::slice_t str) {
            auto offset = coff.string_table.file.size;
            array::append(coff.string_table.list, str);
            coff.string_table.file.size += str.length + 1;
            return offset;
        }
    }

    namespace symbol_table {
        u0 free(coff_t& coff) {
            for (auto& sym : coff.symbol_table.list) {
                array::free(sym.aux_records);
            }
            array::free(coff.symbol_table.list);
        }

        u0 init(coff_t& coff, alloc_t* alloc) {
            array::init(coff.symbol_table.list, alloc);
        }

        aux_record_t* make_aux_record(coff_t& coff,
                                      symbol_t* sym,
                                      aux_record_type_t type) {
            auto aux = &array::append(sym->aux_records);
            aux->type    = type;
            aux->section = {};
            coff.symbol_table.file.size += entry_size;
            return aux;
        }

        symbol_t* make_symbol(coff_t& coff, str::slice_t name) {
            auto sym = &array::append(coff.symbol_table.list);
            array::init(sym->aux_records, coff.alloc);
            if (name.length > 8) {
                sym->name.offset = string_table::add(coff, name);
                sym->name.offset <<= u32(32);
                sym->inlined     = false;
            } else {
                sym->name.slice = name;
                sym->inlined    = true;
            }
            coff.symbol_table.file.size += entry_size;
            return sym;
        }
    }

    u0 free(coff_t& coff) {
        array::free(coff.headers);
        string_table::free(coff);
        symbol_table::free(coff);
    }

    status_t init(coff_t& coff,
                  file_t& file,
                  alloc_t* alloc) {
        using type_t = binfmt::section::type_t;

        const auto module = file.module;
        coff.alloc = alloc;

        array::init(coff.headers, coff.alloc);
        string_table::init(coff, coff.alloc);
        symbol_table::init(coff, coff.alloc);

        coff.align.file    = 0x200;
        coff.align.section = 0x1000;

        switch (file.machine) {
            case binfmt::machine::type_t::unknown:
                return status_t::invalid_machine_type;
            case binfmt::machine::type_t::x86_64:
                coff.machine = machine::amd64;
                break;
            case binfmt::machine::type_t::aarch64:
                coff.machine = machine::arm64;
                break;
        }

        status_t status{};

        for (u32 i = 0; i < module->sections.size; ++i) {
            auto& hdr = array::append(coff.headers);
            hdr = {};
            hdr.number  = i + 1;
            hdr.section = &module->sections[i];

            str::slice_t name{};
            if (hdr.section->type == type_t::custom) {
                auto intern_rc = string::interned::get(hdr.section->symbol);
                if (!OK(intern_rc.status))
                    return status_t::symbol_not_found;
                name = intern_rc.slice;
            } else {
                status = coff::get_section_name(hdr.section, name);
                if (!OK(status))
                    return status;
            }

            hdr.symbol          = coff::symbol_table::make_symbol(coff, name);
            hdr.symbol->section = hdr.number;
            hdr.symbol->value   = {};
            hdr.symbol->type    = 0;
            hdr.symbol->sclass  = symbol_table::storage_class::static_;

            auto aux = coff::symbol_table::make_aux_record(coff, hdr.symbol,aux_record_type_t::section);
            aux->section.len        = hdr.size;
            aux->section.sect_num   = hdr.number;
            aux->section.check_sum  = 0;
            aux->section.comdat_sel = 0;
            aux->section.num_relocs = hdr.relocs.size;
            aux->section.num_lines  = hdr.line_nums.size;
        }

        auto symbols = hashtab::values(const_cast<symbol_table_t&>(module->symbols));
        defer(array::free(symbols));

        for (auto symbol : symbols) {
            const auto intern_rc = string::interned::get(symbol->name);
            if (!OK(intern_rc.status))
                return status_t::symbol_not_found;
            auto sym = coff::symbol_table::make_symbol(coff, intern_rc.slice);
            sym->value   = symbol->value;
            sym->section = symbol->section;
            switch (symbol->type) {
                case symbol::type_t::none:
                case symbol::type_t::object:
                    sym->type = 0;
                    break;
                case symbol::type_t::file:
                    sym->type = 0;
                    sym->sclass = symbol_table::storage_class::file;
                    break;
                case symbol::type_t::function:
                    sym->type = symbol_table::derived_type::function << u32(8);
                    break;
            }
            if (!sym->sclass) {
                switch (symbol->scope) {
                    case symbol::scope_t::none:
                        sym->sclass = symbol_table::storage_class::null_;
                        break;
                    case symbol::scope_t::local:
                        sym->sclass = symbol_table::storage_class::static_;
                        break;
                    case symbol::scope_t::global:
                        sym->sclass = symbol_table::storage_class::external_;
                        break;
                    case symbol::scope_t::weak:
                        sym->sclass = symbol_table::storage_class::weak_external;
                        break;
                }
            }
        }

        return status;
    }

    u0 update_symbol_table(coff_t& coff) {
        coff.symbol_table.file.offset = coff.offset;
        coff.string_table.file.offset = coff.symbol_table.file.offset + coff.symbol_table.file.size;
    }

    u0 write_string_table(file_t& file, coff_t& coff) {
        file::write_u32(file, coff.string_table.file.size);
        for (auto str : coff.string_table.list) {
            file::write_cstr(file, str);
        }
    }

    u0 write_symbol_table(file_t& file, coff_t& coff) {
        file::seek(file, coff.symbol_table.file.offset);
//            std::sort(
//                std::begin(coff.symbol_table.list),
//                std::end(coff.symbol_table.list),
//                [](auto lhfile, auto rhs) {
//                    return lhs < rhs;
//                });
        for (const auto& symbol : coff.symbol_table.list) {
            if (symbol.inlined) {
                file::write_pad8(file, symbol.name.slice);
            } else {
                file::write_u64(file, symbol.name.offset);
            }
            file::write_u32(file, symbol.value);
            file::write_s16(file, symbol.section);
            file::write_u16(file, symbol.type);
            file::write_u8(file, symbol.sclass);
            file::write_u8(file, symbol.aux_records.size);
            for (const auto& aux : symbol.aux_records) {
                write_aux_record(file, aux);
            }
        }
        write_string_table(file, coff);
    }

    status_t build_sections(file_t& file, coff_t& coff) {
        coff.size.headers = coff.offset
                            + coff::header_size
                            + (coff.headers.size * coff::section::header_size);
        coff.offset       = align(coff.size.headers, coff.align.file);
        coff.rva          = align(coff.size.headers, coff.align.section);
        for (auto& hdr : coff.headers) {
            auto status = build_section(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        update_symbol_table(coff);
        return status_t::ok;
    }

    u0 write_section_headers(file_t& file, coff_t& coff) {
        for (auto& hdr : coff.headers) {
            write_section_header(file, coff, hdr);
        }
    }

    status_t write_sections_data(file_t& file, coff_t& coff) {
        for (auto& hdr : coff.headers) {
            auto status = write_section_data(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    u0 write_header(file_t& file, coff_t& coff, u16 opt_hdr_size) {
        file::write_u16(file, coff.machine);
        file::write_u16(file, coff.headers.size);
        file::write_u32(file, std::time(nullptr));
        file::write_u32(file, coff.symbol_table.file.offset);
        file::write_u32(file, coff.symbol_table.file.size / symbol_table::entry_size);
        file::write_u16(file, opt_hdr_size);
        file::write_u16(file, coff.flags.image);
    }

    u0 write_aux_record(file_t& file, const aux_record_t& record) {
        switch (record.type) {
            case aux_record_type_t::xf:
                file::write_u32(file, 0);
                file::write_u16(file, record.xf.line_num);
                file::write_u32(file, 0);
                file::write_u16(file, 0);
                file::write_u32(file, record.xf.ptr_next_func);
                file::write_u16(file, 0);
                break;
            case aux_record_type_t::file: {
                const auto sym = record.file.sym;
                if (sym->inlined) {
                    const auto len     = sym->name.slice.length;
                    const s32  pad_len = std::max<s32>(symbol_table::entry_size - len, 0);
                    for (u32 i = 0; i < std::min<u32>(len, 18); ++i) {
                        file::write_u8(file, sym->name.slice[i]);
                    }
                    for (u32 i = 0; i < pad_len; ++i) {
                        file::write_u8(file, 0);
                    }
                } else {
                    u64 name = sym->name.offset;
                    name <<= u32(32);
                    file::write_u64(file, name);
                    for (u32 i = 0; i < 8; ++i) {
                        file::write_u8(file, 0);
                    }
                }
                break;
            }
            case aux_record_type_t::section:
                file::write_u32(file, record.section.len);
                file::write_u16(file, record.section.num_relocs);
                file::write_u16(file, record.section.num_lines);
                file::write_u32(file, record.section.check_sum);
                file::write_u16(file, record.section.sect_num);
                file::write_u8(file, record.section.comdat_sel);
                file::write_u8(file, 0);    // XXX: unused
                file::write_u8(file, 0);    //      "
                file::write_u8(file, 0);    //      "
                break;
            case aux_record_type_t::func_def:
                file::write_u32(file, record.func_def.tag_idx);
                file::write_u32(file, record.func_def.total_size);
                file::write_u32(file, record.func_def.ptr_line_num);
                file::write_u32(file, record.func_def.ptr_next_func);
                file::write_u16(file, 0);
                break;
            case aux_record_type_t::weak_extern:
                file::write_u32(file, record.weak_extern.tag_idx);
                file::write_u32(file, record.weak_extern.flags);
                file::write_u64(file, 0);
                file::write_u16(file, 0);
                break;
        }
    }

    status_t build_section(file_t& file, coff_t& coff, section_hdr_t& hdr) {
        using type_t = binfmt::section::type_t;

        hdr.offset = coff.offset;
        hdr.rva    = coff.rva;
        const auto& sc = hdr.section->subclass;
        switch (hdr.section->type) {
            case type_t::code:
                if (!coff.code.base)
                    coff.code.base = hdr.rva;
                hdr.size           = sc.data.length;
                coff.code.size += hdr.size;
                coff.size.image    = align(coff.size.image + hdr.size, coff.align.section);
                coff.offset        = align(coff.offset + hdr.size, coff.align.file);
                coff.rva           = align(coff.rva + hdr.size, coff.align.section);
                break;
            case type_t::data:
            case type_t::custom:
                if (hdr.section->flags.init) {
                    if (!coff.init_data.base)
                        coff.init_data.base = hdr.rva;
                    hdr.size                = sc.data.length;
                    coff.init_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                } else {
                    if (!coff.uninit_data.base)
                        coff.uninit_data.base = hdr.rva;
                    hdr.size                  = sc.size;
                    coff.uninit_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                }
                break;
            case type_t::reloc:
            case type_t::import:
            case type_t::export_:
                return status_t::invalid_section_type;
            case type_t::tls:
            case type_t::debug:
            case type_t::resource:
            case type_t::exception:
                return status_t::not_implemented;
        }

        auto& section_rec = hdr.symbol->aux_records[0];
        section_rec.section.len = hdr.size;

        return status_t::ok;
    }

    u0 write_section_header(file_t& file, coff_t& coff, section_hdr_t& hdr) {
        using type_t = binfmt::section::type_t;

        const auto type = hdr.section->type;
        const auto& flags = hdr.section->flags;

        if (hdr.symbol->inlined) {
            file::write_pad8(file, hdr.symbol->name.slice);
        } else {
            file::write_u64(file, hdr.symbol->name.offset);
        }

        file::write_u32(file, hdr.size);
        file::write_u32(file, hdr.rva);

        if (type == type_t::data && !hdr.section->flags.init) {
            file::write_u32(file, 0);
            file::write_u32(file, 0);
        } else {
            file::write_u32(file, align(hdr.size, coff.align.file));
            file::write_u32(file, hdr.offset);
        }

        file::write_u32(file, hdr.relocs.offset);
        file::write_u32(file, hdr.line_nums.offset);
        file::write_u16(file, hdr.relocs.size);
        file::write_u16(file, hdr.line_nums.size);

        u32 bitmask{};
        if (flags.code) bitmask |= section::code;
        if (flags.data) {
            if (flags.init)
                bitmask |= section::init_data;
            else
                bitmask |= section::uninit_data;
        }
        if (flags.read) bitmask |= section::memory_read;
        if (flags.write) bitmask |= section::memory_write;
        if (flags.exec) bitmask |= section::memory_execute;

        file::write_u32(file, bitmask);
    }

    status_t write_section_data(file_t& file, coff_t& coff, section_hdr_t& hdr) {
        using type_t = binfmt::section::type_t;

        const auto type = hdr.section->type;

        if (type == type_t::data && !hdr.section->flags.init)
            return status_t::ok;

        const auto& sc = hdr.section->subclass;
        file::seek(file, hdr.offset);

        switch (type) {
            case type_t::code:
            case type_t::data:
            case type_t::custom:
                file::write_str(file, sc.data);
                break;
            case type_t::import:
            case type_t::export_:
                return status_t::invalid_section_type;
            case type_t::tls:
            case type_t::debug:
            case type_t::resource:
            case type_t::exception:
                return status_t::not_implemented;
            default:
                break;
        }

        return status_t::ok;
    }
}
