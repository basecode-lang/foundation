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
    namespace section {
        symbol_t* get_symbol(coff_t& coff, section_hdr_t& hdr) {
            if (!hdr.symbol) return nullptr;
            return &coff.symbol_table.list[hdr.symbol - 1];
        }
    }

    namespace reloc {
        reloc_t get(const section_hdr_t& hdr, u32 idx) {
            reloc_t r{};

            auto p = hdr.relocs.buf + (idx * 10);
            std::memcpy(&r.rva, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&r.symtab_idx, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&r.type, p, sizeof(u16));

            return r;
        }
    }

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

        const s8* get(coff_t& coff, u64 offset) {
            return coff.string_table.buf + (offset >> u32(32));
        }
    }

    namespace symbol_table {
        u0 free(coff_t& coff) {
            for (auto& sym : coff.symbol_table.list) {
                array::free(sym.aux_records);
            }
            array::free(coff.symbol_table.list);
        }

        symbol_t* make_symbol(coff_t& coff) {
            auto sym = &array::append(coff.symbol_table.list);
            sym->id = coff.symbol_table.list.size;
            array::init(sym->aux_records, coff.alloc);
            return sym;
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

        symbol_t* make_symbol(coff_t& coff, u64 offset) {
            auto sym = &array::append(coff.symbol_table.list);
            sym->id = coff.symbol_table.list.size;
            array::init(sym->aux_records, coff.alloc);
            sym->name.offset = offset;
            sym->inlined     = false;
            return sym;
        }

        symbol_t* make_symbol(coff_t& coff, str::slice_t name) {
            auto sym = &array::append(coff.symbol_table.list);
            sym->id = coff.symbol_table.list.size;
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

        coff.alloc = alloc;

        array::init(coff.headers, coff.alloc);
        string_table::init(coff, coff.alloc);
        symbol_table::init(coff, coff.alloc);

        coff.timestamp     = std::time(nullptr);
        coff.align.file    = 0x200;
        coff.align.section = 0x1000;

        switch (file.machine) {
            case binfmt::machine::type_t::unknown:
                coff.machine = 0;
                break;
            case binfmt::machine::type_t::x86_64:
                coff.machine = machine::amd64;
                break;
            case binfmt::machine::type_t::aarch64:
                coff.machine = machine::arm64;
                break;
        }

        if (!file.module)
            return status_t::ok;

        status_t status{};

        const auto module = file.module;
        auto& module_sc = module->subclass.object;

        for (u32 i = 0; i < module_sc.sections.size; ++i) {
            auto& hdr = array::append(coff.headers);
            hdr = {};
            hdr.number  = i + 1;
            hdr.section = &module_sc.sections[i];

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

            auto sym = coff::symbol_table::make_symbol(coff, name);
            sym->section = hdr.number;
            sym->value   = {};
            sym->type    = 0;
            sym->sclass  = symbol_table::storage_class::static_;
            hdr.symbol = sym->id;

            auto aux = coff::symbol_table::make_aux_record(coff, sym, aux_record_type_t::section);
            aux->section.len        = hdr.rva.size;
            aux->section.sect_num   = hdr.number;
            aux->section.check_sum  = 0;
            aux->section.comdat_sel = 0;
            aux->section.num_relocs = hdr.relocs.file.size;
            aux->section.num_lines  = hdr.line_nums.file.size;
        }

        hashtab::for_each_pair(module->symbols,
                               [](const auto idx, const auto& key, auto& symbol, auto* user) -> u32 {
                                   auto& coff = *((coff_t*)user);
                                   const auto intern_rc = string::interned::get(symbol.name);
                                   if (!OK(intern_rc.status))
                                       return u32(status_t::symbol_not_found);
                                   auto sym = coff::symbol_table::make_symbol(coff, intern_rc.slice);
                                   sym->value   = symbol.value;
                                   sym->section = symbol.section;
                                   switch (symbol.type) {
                                       case symbol::type_t::none:
                                       case symbol::type_t::object:
                                           sym->type = 0;
                                           break;
                                       case symbol::type_t::file:
                                           sym->type   = 0;
                                           sym->sclass = symbol_table::storage_class::file;
                                           break;
                                       case symbol::type_t::function:
                                           sym->type = symbol_table::type::function;
                                           break;
                                   }
                                   if (!sym->sclass) {
                                       switch (symbol.scope) {
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
                                   return 0;
                               },
                               &coff);

        return status;
    }

    u0 update_symbol_table(coff_t& coff) {
        coff.symbol_table.file.offset = coff.offset;
        coff.string_table.file.offset = coff.symbol_table.file.offset + coff.symbol_table.file.size;
    }

    u0 set_section_flags(section_hdr_t& hdr) {
        const auto& flags = hdr.section->flags;

        if (flags.data) {
            if (flags.init)
                hdr.flags |= section::init_data;
            else
                hdr.flags |= section::uninit_data;
        }
        if (flags.code)     hdr.flags |= section::code;
        if (flags.read)     hdr.flags |= section::memory_read;
        if (flags.write)    hdr.flags |= section::memory_write;
        if (flags.exec)     hdr.flags |= section::memory_execute;
    }

    u0 write_header(file_t& file, coff_t& coff) {
        file::write_u16(file, coff.machine);
        file::write_u16(file, coff.headers.size);
        file::write_u32(file, coff.timestamp);
        file::write_u32(file, coff.symbol_table.file.offset);
        file::write_u32(file, coff.symbol_table.file.size / symbol_table::entry_size);
        file::write_u16(file, coff.size.opt_hdr);
        file::write_u16(file, coff.flags.image);
    }

    status_t read_header(file_t& file, coff_t& coff) {
        if (!OK(file::read_u16(file, coff.machine)))
            return status_t::read_error;

        u16 num_headers{};
        if (!OK(file::read_u16(file, num_headers)))
            return status_t::read_error;
        array::resize(coff.headers, num_headers);

        if (!OK(file::read_u32(file, coff.timestamp)))
            return status_t::read_error;

        if (!OK(file::read_u32(file, coff.symbol_table.file.offset)))
            return status_t::read_error;

        if (!OK(file::read_u32(file, coff.symbol_table.num_symbols)))
            return status_t::read_error;
        coff.symbol_table.file.size   = coff.symbol_table.num_symbols * symbol_table::entry_size;
        coff.string_table.file.offset = coff.symbol_table.file.offset + coff.symbol_table.file.size;
        coff.string_table.buf         = (const s8*) file.crsr.buf->data + coff.string_table.file.offset + 4;

        if (!OK(file::read_u16(file, coff.size.opt_hdr)))
            return status_t::read_error;

        if (!OK(file::read_u16(file, coff.flags.image)))
            return status_t::read_error;

        return status_t::ok;
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
//                [](auto lhs, auto rhs) {
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

    status_t read_string_table(file_t& file, coff_t& coff) {
        if (!OK(file::read_u32(file, coff.string_table.file.size)))
            return status_t::read_error;

        auto size = coff.string_table.file.size;
        u8* start = CRSR_PTR(file.crsr);
        u8* end   = start;
        while (size) {
            while (*end != '\0')
                ++end;
            const auto len = end - start;
            const auto slice = slice::make(start, len);
            string_table::add(coff, slice);
            size -= len + 1;
            start += len + 1;
            end = start;
        }

        return status_t::ok;
    }

    status_t read_symbol_table(file_t& file, coff_t& coff) {
        file::push(file);
        defer(file::pop(file));

        file::seek(file, coff.symbol_table.file.offset);

        u64_bytes_t u64_thunk{};
        s8          buf[18];
        u32         zero{};
        u32         num_symbols = coff.symbol_table.num_symbols;

        while (num_symbols) {
            auto sym = symbol_table::make_symbol(coff);

            if (!OK(file::read_u64(file, u64_thunk.value)))
                return status_t::read_error;

            if (memcmp(u64_thunk.bytes, &zero, 4) == 0) {
                sym->name.offset = u64_thunk.value;
                sym->inlined     = false;
            } else {
                u32 len{};
                for (u8 ch : u64_thunk.bytes) {
                    if (ch == 0)
                        break;
                    ++len;
                }
                const auto slice = slice::make(u64_thunk.bytes, len);
                sym->name.slice = string::interned::fold(slice);
                sym->inlined    = true;
            }

            if (!OK(file::read_u32(file, sym->value)))
                return status_t::read_error;

            if (!OK(file::read_s16(file, sym->section)))
                return status_t::read_error;

            if (!OK(file::read_u16(file, sym->type)))
                return status_t::read_error;

            if (!OK(file::read_u8(file, sym->sclass)))
                return status_t::read_error;

            u8 num_aux_recs{};
            if (!OK(file::read_u8(file, num_aux_recs)))
                return status_t::read_error;

            if (num_aux_recs > 0) {
                array::resize(sym->aux_records, num_aux_recs);
                for (u32 j = 0; j < num_aux_recs; ++j) {
                    auto& aux = sym->aux_records[j];
                    switch (sym->sclass) {
                        case symbol_table::storage_class::file: {
                            if (!OK(file::read_obj(file, buf, 18)))
                                return status_t::read_error;
                            aux.type = aux_record_type_t::file;
                            aux.file.value = string::interned::fold(slice::make(buf, 18));
                            break;
                        }
                        case symbol_table::storage_class::static_: {
                            if (sym->for_section || sym->type == 0) {
                                if (!OK(file::read_u32(file, aux.section.len)))
                                    return status_t::read_error;
                                if (!OK(file::read_u16(file, aux.section.num_relocs)))
                                    return status_t::read_error;
                                if (!OK(file::read_u16(file, aux.section.num_lines)))
                                    return status_t::read_error;
                                if (!OK(file::read_u32(file, aux.section.check_sum)))
                                    return status_t::read_error;
                                if (!OK(file::read_u16(file, aux.section.sect_num)))
                                    return status_t::read_error;
                                if (!OK(file::read_u8(file, aux.section.comdat_sel)))
                                    return status_t::read_error;
                                if (!OK(file::seek_fwd(file, 3)))
                                    return status_t::read_error;
                                aux.type = aux_record_type_t::section;
                            }
                            break;
                        }
                        case symbol_table::storage_class::function: {
                            if (!OK(file::seek_fwd(file, 4)))
                                return status_t::read_error;
                            if (!OK(file::read_u16(file, aux.xf.line_num)))
                                return status_t::read_error;
                            if (!OK(file::seek_fwd(file, 6)))
                                return status_t::read_error;
                            if (!OK(file::read_u32(file, aux.xf.ptr_next_func)))
                                return status_t::read_error;
                            if (!OK(file::seek_fwd(file, 2)))
                                return status_t::read_error;
                            aux.type = aux_record_type_t::xf;
                            break;
                        }
                        case symbol_table::storage_class::external_: {
                            if (sym->section == 0) {
                                if (sym->value == 0 && sym->section == 0) {
                                    if (!OK(file::read_u32(file, aux.weak_extern.tag_idx)))
                                        return status_t::read_error;
                                    if (!OK(file::read_u32(file, aux.weak_extern.flags)))
                                        return status_t::read_error;
                                    if (!OK(file::seek_fwd(file, 10)))
                                        return status_t::read_error;
                                    aux.type = aux_record_type_t::weak_extern;
                                } else if (sym->type == symbol_table::type::function) {
                                    if (!OK(file::read_u32(file, aux.func_def.tag_idx)))
                                        return status_t::read_error;
                                    if (!OK(file::read_u32(file, aux.func_def.total_size)))
                                        return status_t::read_error;
                                    if (!OK(file::read_u32(file, aux.func_def.ptr_next_func)))
                                        return status_t::read_error;
                                    if (!OK(file::seek_fwd(file, 2)))
                                        return status_t::read_error;
                                    aux.type = aux_record_type_t::func_def;
                                }
                            }
                            break;
                        }
                        default: {
                            file::seek_fwd(file, 18);
                            break;
                        }
                    }
                    --num_symbols;
                }
            }

            --num_symbols;
        }

        return status_t::ok;
    }

    status_t write_sections_data(file_t& file, coff_t& coff) {
        for (auto& hdr : coff.headers) {
            auto status = write_section_data(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t read_section_headers(file_t& file, coff_t& coff) {
        u32 zero{};
        for (u32 i = 0; i < coff.headers.size; ++i) {
            auto& hdr = coff.headers[i];
            hdr.number = i + 1;

            u64_bytes_t u64_thunk{};
            if (!OK(file::read_u64(file, u64_thunk.value)))
                return status_t::read_error;

            symbol_t* sym;
            if (memcmp(u64_thunk.bytes, &zero, 4) == 0) {
                sym = symbol_table::make_symbol(coff, u64_thunk.value);
            } else {
                u32 len{};
                for (u8 ch : u64_thunk.bytes) {
                    if (ch == 0)
                        break;
                    ++len;
                }
                const auto slice = slice::make(u64_thunk.bytes, len);
                sym = symbol_table::make_symbol(coff, string::interned::fold(slice));
            }
            sym->section     = hdr.number;
            sym->sclass      = symbol_table::storage_class::static_;
            sym->for_section = true;
            hdr.symbol       = sym->id;

            if (!OK(file::read_u32(file, hdr.rva.size)))
                return status_t::read_error;

            if (!OK(file::read_u32(file, hdr.rva.base)))
                return status_t::read_error;

            if (!OK(file::read_u32(file, hdr.file.size)))
                return status_t::read_error;

            if (!OK(file::read_u32(file, hdr.file.offset)))
                return status_t::read_error;

            if (!OK(file::read_u32(file, hdr.relocs.file.offset)))
                return status_t::read_error;

            if (!OK(file::read_u32(file, hdr.line_nums.file.offset)))
                return status_t::read_error;

            u16 num_relocs{};
            if (!OK(file::read_u16(file, num_relocs)))
                return status_t::read_error;

            u16 num_line_nos{};
            if (!OK(file::read_u16(file, num_line_nos)))
                return status_t::read_error;

            hdr.relocs.buf          = (const u8*) file.crsr.buf->data + hdr.relocs.file.offset;
            hdr.relocs.file.size    = num_relocs;
            hdr.line_nums.buf       = (const u8*) file.crsr.buf->data + hdr.line_nums.file.offset;
            hdr.line_nums.file.size = num_line_nos;

            if (!OK(file::read_u32(file, hdr.flags)))
                return status_t::read_error;
        }
        return status_t::ok;
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
                file::write_str(file, record.file.value);
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

        hdr.file.offset = coff.offset;
        hdr.rva.base    = coff.rva;
        const auto& sc = hdr.section->subclass;
        switch (hdr.section->type) {
            case type_t::code:
                if (!coff.code.base)
                    coff.code.base = hdr.rva.base;
                hdr.rva.size    = sc.data.length;
                hdr.file.size   = align(hdr.rva.size, coff.align.file);
                coff.code.size += hdr.rva.size;
                coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                coff.offset     = align(coff.offset + hdr.rva.size, coff.align.file);
                coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
                break;
            case type_t::data:
            case type_t::custom:
                if (hdr.section->flags.init) {
                    if (!coff.init_data.base)
                        coff.init_data.base = hdr.rva.base;
                    hdr.rva.size  = sc.data.length;
                    hdr.file.size = align(hdr.rva.size, coff.align.file);
                    coff.init_data.size += hdr.rva.size;
                    coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.rva.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
                } else {
                    if (!coff.uninit_data.base)
                        coff.uninit_data.base = hdr.rva.base;
                    hdr.rva.size    = sc.size;
                    hdr.file.offset = 0;
                    hdr.file.size   = 0;
                    coff.uninit_data.size += hdr.rva.size;
                    coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                    coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
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

        set_section_flags(hdr);

        auto sym = section::get_symbol(coff, hdr);
        auto& section_rec = sym->aux_records[0];
        section_rec.section.len = hdr.rva.size;

        return status_t::ok;
    }

    u0 write_section_header(file_t& file, coff_t& coff, section_hdr_t& hdr) {
        auto sym = section::get_symbol(coff, hdr);

        if (sym->inlined) {
            file::write_pad8(file, sym->name.slice);
        } else {
            file::write_u64(file, sym->name.offset);
        }

        file::write_u32(file, hdr.rva.size);
        file::write_u32(file, hdr.rva.base);
        file::write_u32(file, hdr.file.size);
        file::write_u32(file, hdr.file.offset);
        file::write_u32(file, hdr.relocs.file.offset);
        file::write_u32(file, hdr.line_nums.file.offset);
        file::write_u16(file, hdr.relocs.file.size);
        file::write_u16(file, hdr.line_nums.file.size);
        file::write_u32(file, hdr.flags);
    }

    status_t write_section_data(file_t& file, coff_t& coff, section_hdr_t& hdr) {
        using type_t = binfmt::section::type_t;

        const auto type = hdr.section->type;

        if (type == type_t::data && !hdr.section->flags.init)
            return status_t::ok;

        const auto& sc = hdr.section->subclass;
        file::seek(file, hdr.file.offset);

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
