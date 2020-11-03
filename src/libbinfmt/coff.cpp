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

namespace basecode::binfmt::io {
    namespace internal {
        struct coff_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
        };

        coff_system_t               g_coff_sys{};

        static u0 fini() {
            name_map::free(g_coff_sys.section_names);
        }

        static status_t read(session_t& s) {
            UNUSED(s);
            return status_t::read_error;
        }

        static status_t write(session_t& s) {
            UNUSED(s);
            return status_t::write_error;
        }

        static status_t init(alloc_t* alloc) {
            g_coff_sys.alloc = alloc;

            name_map::init(g_coff_sys.section_names, g_coff_sys.alloc);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::tls,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".tls"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::code,
                          {
                              .code = true,
                              .data = false,
                              .read = true,
                              .exec = true,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".text"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".data"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".rdata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::debug,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = true
                          },
                          ".debug"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = false,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".bss"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::import,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".idata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::export_,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".edata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::resource,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".rsrc"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::exception,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".pdata"_ss);

            return status_t::ok;
        }

        system_t                    g_coff_backend {
            .init   = init,
            .fini   = fini,
            .read   = read,
            .write  = write,
            .type   = type_t::coff
        };
    }

    namespace coff {
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
                for (auto& sym : coff.symbol_table.list)
                    array::free(sym.aux_records);
                array::free(coff.symbol_table.list);
            }

            u0 init(coff_t& coff, alloc_t* alloc) {
                array::init(coff.symbol_table.list, alloc);
            }

            coff_symbol_t* make_symbol(coff_t& coff, str::slice_t name) {
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

            coff_aux_record_t* make_aux_record(coff_t& coff, coff_symbol_t* sym, coff_aux_record_type_t type) {
                auto aux = &array::append(sym->aux_records);
                aux->type    = type;
                aux->section = {};
                coff.symbol_table.file.size += entry_size;
                return aux;
            }
        }

        system_t* system() {
            return &internal::g_coff_backend;
        }

        u0 free(coff_t& coff) {
            array::free(coff.headers);
            string_table::free(coff);
            symbol_table::free(coff);
        }

        status_t init(coff_t& coff,
                      const file_t* file,
                      alloc_t* alloc) {
            using type_t = binfmt::section::type_t;

            coff.alloc = alloc;

            array::init(coff.headers, coff.alloc);
            string_table::init(coff, coff.alloc);
            symbol_table::init(coff, coff.alloc);

            coff.align.file    = 0x200;
            coff.align.section = 0x1000;

            switch (file->machine) {
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

            for (u32 i = 0; i < file->sections.size; ++i) {
                auto& hdr = array::append(coff.headers);
                hdr = {};
                hdr.number  = i + 1;
                hdr.section = &file->sections[i];

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

                hdr.symbol = coff::symbol_table::make_symbol(coff, name);
                hdr.symbol->section = hdr.number;
                hdr.symbol->value   = {};
                hdr.symbol->type    = SYMBOL_TYPE(symbol::type::derived::none,
                                                  symbol::type::base::null_);
                hdr.symbol->sclass  = storage::class_t::static_;

                auto aux = coff::symbol_table::make_aux_record(coff,
                                                               hdr.symbol,
                                                               coff_aux_record_type_t::section);
                aux->section.len        = hdr.size;
                aux->section.sect_num   = hdr.number;
                aux->section.check_sum  = 0;
                aux->section.comdat_sel = 0;
                aux->section.num_relocs = hdr.relocs.size;
                aux->section.num_lines  = hdr.line_nums.size;
            }

            auto symbols = hashtab::values(const_cast<symbol_table_t&>(file->symbols));
            defer(array::free(symbols));

            for (auto symbol : symbols) {
                const auto intern_rc = string::interned::get(symbol->name);
                if (!OK(intern_rc.status))
                    return status_t::symbol_not_found;
                auto sym = coff::symbol_table::make_symbol(coff, intern_rc.slice);
                sym->value   = symbol->value;
                sym->section = symbol->section;
                sym->type    = symbol->type;
                sym->sclass  = symbol->sclass;
            }

            return status;
        }

        u0 update_symbol_table(coff_t& coff) {
            coff.symbol_table.file.offset = coff.offset;
            coff.string_table.file.offset = coff.symbol_table.file.offset + coff.symbol_table.file.size;
        }

        u0 write_string_table(session_t& s, coff_t& coff) {
            session::write_u32(s, coff.string_table.file.size);
            for (auto str : coff.string_table.list)
                session::write_cstr(s, str);
        }

        u0 write_symbol_table(session_t& s, coff_t& coff) {
            session::seek(s, coff.symbol_table.file.offset);
//            std::sort(
//                std::begin(coff.symbol_table.list),
//                std::end(coff.symbol_table.list),
//                [](auto lhs, auto rhs) {
//                    return lhs < rhs;
//                });
            for (const auto& symbol : coff.symbol_table.list) {
                if (symbol.inlined) {
                    session::write_pad8(s, symbol.name.slice);
                } else {
                    session::write_u64(s, symbol.name.offset);
                }
                session::write_u32(s, symbol.value);
                session::write_s16(s, symbol.section);
                session::write_u16(s, u16(symbol.type));
                session::write_u8(s, u8(symbol.sclass));
                session::write_u8(s, symbol.aux_records.size);
                for (const auto& aux : symbol.aux_records)
                    write_aux_record(s, aux);
            }
            write_string_table(s, coff);
        }

        status_t build_sections(session_t& s, coff_t& coff) {
            coff.size.headers = coff.offset
                              + coff::header_size
                              + (coff.headers.size * coff::section::header_size);
            coff.offset = align(coff.size.headers, coff.align.file);
            coff.rva    = align(coff.size.headers, coff.align.section);
            for (auto& hdr : coff.headers) {
                auto status = build_section(s, coff, hdr);
                if (!OK(status))
                    return status;
            }
            update_symbol_table(coff);
            return status_t::ok;
        }

        u0 write_section_headers(session_t& s, coff_t& coff) {
            for (auto& hdr : coff.headers)
                write_section_header(s, coff, hdr);
        }

        status_t write_sections_data(session_t& s, coff_t& coff) {
            for (auto& hdr : coff.headers) {
                auto status = write_section_data(s, coff, hdr);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        u0 write_header(session_t& s, coff_t& coff, u16 opt_hdr_size) {
            session::write_u16(s, coff.machine);
            session::write_u16(s, coff.headers.size);
            session::write_u32(s, std::time(nullptr));
            session::write_u32(s, coff.symbol_table.file.offset);
            session::write_u32(s, coff.symbol_table.file.size / symbol_table::entry_size);
            session::write_u16(s, opt_hdr_size);
            session::write_u16(s, coff.flags.image);
        }

        u0 write_aux_record(session_t& s, const coff_aux_record_t& record) {
            switch (record.type) {
                case coff_aux_record_type_t::xf:
                    session::write_u32(s, 0);
                    session::write_u16(s, record.xf.line_num);
                    session::write_u32(s, 0);
                    session::write_u16(s, 0);
                    session::write_u32(s, record.xf.ptr_next_func);
                    session::write_u16(s, 0);
                    break;
                case coff_aux_record_type_t::file: {
                    const auto sym = record.file.sym;
                    if (sym->inlined) {
                        const auto len = sym->name.slice.length;
                        const s32 pad_len = std::max<s32>(symbol_table::entry_size - len, 0);
                        for (u32 i = 0; i < std::min<u32>(len, 18); ++i)
                            session::write_u8(s, sym->name.slice[i]);
                        for (u32 i = 0; i < pad_len; ++i)
                            session::write_u8(s, 0);
                    } else {
                        u64 name = sym->name.offset;
                        name <<= u32(32);
                        session::write_u64(s, name);
                        for (u32 i = 0; i < 8; ++i)
                            session::write_u8(s, 0);
                    }
                    break;
                }
                case coff_aux_record_type_t::section:
                    session::write_u32(s, record.section.len);
                    session::write_u16(s, record.section.num_relocs);
                    session::write_u16(s, record.section.num_lines);
                    session::write_u32(s, record.section.check_sum);
                    session::write_u16(s, record.section.sect_num);
                    session::write_u8(s, record.section.comdat_sel);
                    session::write_u8(s, 0);    // XXX: unused
                    session::write_u8(s, 0);    //      "
                    session::write_u8(s, 0);    //      "
                    break;
                case coff_aux_record_type_t::func_def:
                    session::write_u32(s, record.func_def.tag_idx);
                    session::write_u32(s, record.func_def.total_size);
                    session::write_u32(s, record.func_def.ptr_line_num);
                    session::write_u32(s, record.func_def.ptr_next_func);
                    session::write_u16(s, 0);
                    break;
                case coff_aux_record_type_t::weak_extern:
                    session::write_u32(s, record.weak_extern.tag_idx);
                    session::write_u32(s, record.weak_extern.flags);
                    session::write_u64(s, 0);
                    session::write_u16(s, 0);
                    break;
            }
        }

        status_t build_section(session_t& s, coff_t& coff, coff_section_hdr_t& hdr) {
            using type_t = binfmt::section::type_t;

            hdr.offset = coff.offset;
            hdr.rva    = coff.rva;
            const auto& sc = hdr.section->subclass;
            switch (hdr.section->type) {
                case type_t::code:
                    if (!coff.code.base)
                        coff.code.base = hdr.rva;
                    hdr.size        = sc.data.length;
                    coff.code.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                    break;
                case type_t::data:
                case type_t::custom:
                    if (hdr.section->flags.init) {
                        if (!coff.init_data.base)
                            coff.init_data.base = hdr.rva;
                        hdr.size        = sc.data.length;
                        coff.init_data.size += hdr.size;
                        coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                        coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                        coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                    } else {
                        if (!coff.uninit_data.base)
                            coff.uninit_data.base = hdr.rva;
                        hdr.size = sc.size;
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

        u0 write_section_header(session_t& s, coff_t& coff, coff_section_hdr_t& hdr) {
            using type_t = binfmt::section::type_t;

            const auto type = hdr.section->type;
            const auto& flags = hdr.section->flags;

            if (hdr.symbol->inlined) {
                session::write_pad8(s, hdr.symbol->name.slice);
            } else {
                session::write_u64(s, hdr.symbol->name.offset);
            }

            session::write_u32(s, hdr.size);
            session::write_u32(s, hdr.rva);

            if (type == type_t::data && !hdr.section->flags.init) {
                session::write_u32(s, 0);
                session::write_u32(s, 0);
            } else {
                session::write_u32(s, align(hdr.size, coff.align.file));
                session::write_u32(s, hdr.offset);
            }

            session::write_u32(s, hdr.relocs.offset);
            session::write_u32(s, hdr.line_nums.offset);
            session::write_u16(s, hdr.relocs.size);
            session::write_u16(s, hdr.line_nums.size);

            u32 bitmask{};
            if (flags.code)     bitmask |= section::code;
            if (flags.data) {
                if (flags.init)
                    bitmask |= section::init_data;
                else
                    bitmask |= section::uninit_data;
            }
            if (flags.read)     bitmask |= section::memory_read;
            if (flags.write)    bitmask |= section::memory_write;
            if (flags.exec)     bitmask |= section::memory_execute;

            session::write_u32(s, bitmask);
        }

        status_t write_section_data(session_t& s, coff_t& coff, coff_section_hdr_t& hdr) {
            using type_t = binfmt::section::type_t;

            const auto type = hdr.section->type;

            if (type == type_t::data && !hdr.section->flags.init)
                return status_t::ok;

            const auto& sc = hdr.section->subclass;
            session::seek(s, hdr.offset);

            switch (type) {
                case type_t::code:
                case type_t::data:
                case type_t::custom:
                    session::write_str(s, sc.data);
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

        status_t get_section_name(const binfmt::section_t* section, str::slice_t& name) {
            const auto entry = name_map::find(internal::g_coff_sys.section_names, section->type, section->flags);
            if (!entry)
                return status_t::cannot_map_section_name;
            name = entry->name;
            return status_t::ok;
        }
    }
}
