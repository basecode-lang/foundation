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

        u0 write_header(session_t& s, coff_t& coff) {
            session::write_u16(s, u16(s.file->machine));
            session::write_u16(s, coff.num_hdrs);
            session::write_u32(s, std::time(nullptr));
            session::write_u32(s, coff.symbol_table.offset);
            session::write_u32(s, coff.symbol_table.size / symbol_table::entry_size);
            session::write_u16(s, coff.size.opt_hdr);
            session::write_u16(s, coff.flags.image);
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

        u0 write_section_headers(session_t& s, coff_t& coff) {
            using type_t = objfmt::section::type_t;

            for (u32 i = 0; i < coff.num_hdrs; ++i) {
                const auto& hdr = coff.hdrs[i];
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
        }

        str::slice_t get_section_name(objfmt::section::type_t type) {
            return s_section_names[u32(type)];
        }

        status_t init(coff_t& coff, section_hdr_t* hdrs, u32 num_hdrs, alloc_t* alloc) {
            UNUSED(alloc);
            coff.hdrs                = hdrs;
            coff.num_hdrs            = num_hdrs;
            coff.symbol_table        = {};
            coff.string_table        = {};
            return status_t::ok;
        }
    }
}
