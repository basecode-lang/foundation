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
#include <basecode/objfmt/coff.h>
#include <basecode/core/string.h>
#include <basecode/objfmt/objfmt.h>

namespace basecode::objfmt::container {
    static str::slice_t s_section_names[section::max_spec_type_count];

    namespace internal {
        static u0 fini() {
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
            UNUSED(alloc);
            s_section_names[u32(section::type_t::code)]     = string::interned::fold(".text"_ss);
            s_section_names[u32(section::type_t::data)]     = string::interned::fold(".rdata"_ss);
            s_section_names[u32(section::type_t::debug)]    = string::interned::fold(".debug"_ss);
            s_section_names[u32(section::type_t::uninit)]   = string::interned::fold(".bss"_ss);
            s_section_names[u32(section::type_t::import)]   = string::interned::fold(".idata"_ss);
            s_section_names[u32(section::type_t::export_)]  = string::interned::fold(".edata"_ss);
            s_section_names[u32(section::type_t::resource)] = string::interned::fold(".rsrc"_ss);
            return status_t::ok;
        }

        system_t                    g_coff_sys {
            .init   = init,
            .fini   = fini,
            .read   = read,
            .write  = write,
            .type   = type_t::coff
        };
    }

    namespace coff {
        system_t* system() {
            return &internal::g_coff_sys;
        }

        u0 free(coff_t& coff) {
            UNUSED(coff);
        }

        status_t init(coff_t& coff,
                      section_hdr_t* hdrs,
                      u32 num_hdrs,
                      objfmt::machine::type_t machine,
                      alloc_t* alloc) {
            UNUSED(alloc);
            coff.hdrs         = hdrs;
            coff.num_hdrs     = num_hdrs;
            coff.symbol_table = {};
            coff.string_table = {};
            coff.size = {};
            coff.code = coff.init_data = coff.uninit_data = {};
            switch (machine) {
                case objfmt::machine::type_t::unknown:
                    return status_t::invalid_machine_type;
                case objfmt::machine::type_t::x86_64:
                    coff.machine = machine::amd64;
                    break;
                case objfmt::machine::type_t::aarch64:
                    coff.machine = machine::arm64;
                    break;
            }
            return status_t::ok;
        }

        u0 write_symbol_table(session_t& s, coff_t& coff) {
            session::seek(s, coff.symbol_table.offset);
            auto symbols = hashtab::values(const_cast<symbol_table_t&>(s.file->symbols));
            defer(array::free(symbols));
            std::sort(
                std::begin(symbols),
                std::end(symbols),
                [](auto lhs, auto rhs) {
                    return lhs < rhs;
                });
            u32 num_long_strs{};
            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                if (coff.hdrs[i].name.length > 8)
                    num_long_strs++;
            }
            for (auto symbol : symbols) {
                if (symbol->length > 8)
                    num_long_strs++;
            }
            str::slice_t strs[num_long_strs];
            u32 idx{};
            coff.string_table.offset = coff.symbol_table.offset + coff.symbol_table.size;
            coff.string_table.size   = sizeof(u32);
            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                const auto& hdr   = coff.hdrs[i];
                const auto& slice = hdr.name;
                if (slice.length > 8) {
                    strs[idx++] = slice;
                    u64 name = coff.string_table.size;
                    name <<= u32(32);
                    session::write_u64(s, name);
                    coff.string_table.size += slice.length + 1;
                } else {
                    session::write_pad8(s, slice);
                }
                session::write_u32(s, 0);
                session::write_s16(s, hdr.number);
                session::write_u16(s, 0);
                session::write_u8(s, 3);
                session::write_u8(s, 1);

                session::write_u32(s, hdr.size);
                session::write_u16(s, hdr.relocs.size);
                session::write_u16(s, hdr.line_nums.size);
                session::write_u32(s, 0);
                session::write_u16(s, 0);
                session::write_u8(s, 0);
                session::write_u8(s, 0);    // XXX: unused
                session::write_u8(s, 0);    //      "
                session::write_u8(s, 0);    //      "
            }
            for (auto symbol : symbols) {
                const auto intern_rc = string::interned::get(symbol->name);
                if (intern_rc.slice.length > 8) {
                    strs[idx++] = intern_rc.slice;
                    u64 name = coff.string_table.size;
                    name <<= u32(32);
                    session::write_u64(s, name);
                    coff.string_table.size += intern_rc.slice.length + 1;
                } else {
                    session::write_pad8(s, intern_rc.slice);
                }
                session::write_u32(s, symbol->value);
                session::write_s16(s, symbol->section);
                session::write_u16(s, u16(symbol->type));
                session::write_u8(s, 2);
                session::write_u8(s, 0);
            }
            session::write_u32(s, coff.string_table.size);
            for (auto str : strs)
                session::write_cstr(s, str);
        }

        status_t build_sections(session_t& s, coff_t& coff) {
            coff.size.headers = coff.offset
                              + coff::header_size
                              + (coff.num_hdrs * coff::section::header_size);
            coff.offset = align(coff.size.headers, coff.align.file);
            coff.rva    = align(coff.size.headers, coff.align.section);
            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                auto status = build_section(s, coff, coff.hdrs[i]);
                if (!OK(status))
                    return status;
            }
            coff.symbol_table.offset = coff.offset;
            return status_t::ok;
        }

        u0 write_section_headers(session_t& s, coff_t& coff) {
            for (u32 i = 0; i < coff.num_hdrs; ++i)
                write_section_header(s, coff, coff.hdrs[i]);
        }

        status_t write_sections_data(session_t& s, coff_t& coff) {
            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                auto status = write_section_data(s, coff, coff.hdrs[i]);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        str::slice_t get_section_name(objfmt::section::type_t type) {
            return s_section_names[u32(type)];
        }

        u0 write_header(session_t& s, coff_t& coff, u16 opt_hdr_size) {
            session::write_u16(s, coff.machine);
            session::write_u16(s, coff.num_hdrs);
            session::write_u32(s, std::time(nullptr));
            session::write_u32(s, coff.symbol_table.offset);
            session::write_u32(s, coff.symbol_table.size / symbol_table::entry_size);
            session::write_u16(s, opt_hdr_size);
            session::write_u16(s, coff.flags.image);
        }

        status_t build_section(session_t& s, coff_t& coff, section_hdr_t& hdr) {
            using type_t = objfmt::section::type_t;

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
                    if (!coff.init_data.base)
                        coff.init_data.base = hdr.rva;
                    hdr.size        = sc.data.length;
                    coff.init_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                    break;
                case type_t::uninit:
                    hdr.size = sc.size;
                    coff.uninit_data.size += hdr.size;
                    coff.size.image = align(coff.size.image + hdr.size, coff.align.section);
                    coff.rva        = align(coff.rva + hdr.size, coff.align.section);
                    break;
                case type_t::import:
                case type_t::export_:
                    return status_t::invalid_section_type;
                case type_t::debug:
                case type_t::resource:
                    return status_t::not_implemented;
            }

            return status_t::ok;
        }

        u0 write_section_header(session_t& s, coff_t& coff, section_hdr_t& hdr) {
            using type_t = objfmt::section::type_t;

            const auto type = hdr.section->type;
            const auto& flags = hdr.section->flags;

            if (type == type_t::custom) {
                const auto symbol = objfmt::file::get_symbol(*s.file, hdr.section->symbol);
                const auto intern_rc = string::interned::get(symbol->name);
                session::write_pad8(s, intern_rc.slice);
            } else {
                session::write_pad8(s, hdr.name);
            }

            session::write_u32(s, hdr.size);
            session::write_u32(s, hdr.rva);

            if (type == type_t::uninit) {
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
                if (type == type_t::data
                ||  type == type_t::import) {
                    bitmask |= section::init_data;
                } else if (type == type_t::uninit) {
                    bitmask |= section::uninit_data;
                }
            }
            if (flags.read)     bitmask |= section::memory_read;
            if (flags.write)    bitmask |= section::memory_write;
            if (flags.exec)     bitmask |= section::memory_execute;

            session::write_u32(s, bitmask);
        }

        status_t write_section_data(session_t& s, coff_t& coff, section_hdr_t& hdr) {
            using type_t = objfmt::section::type_t;

            const auto type = hdr.section->type;

            if (type == type_t::uninit)
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
                case type_t::debug:
                case type_t::resource:
                    return status_t::not_implemented;
                default:
                    break;
            }

            return status_t::ok;
        }
    }
}
