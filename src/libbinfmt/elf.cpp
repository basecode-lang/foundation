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

#include <basecode/binfmt/elf.h>

namespace basecode::binfmt::io {
    namespace internal {
        struct elf_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
            name_list_t             segment_names;
        };

        elf_system_t                g_elf_sys{};

        static u0 fini() {
            name_map::free(g_elf_sys.section_names);
            name_map::free(g_elf_sys.segment_names);
        }

        static status_t read(session_t& s) {
            UNUSED(s);
            return status_t::read_error;
        }

        static status_t write(session_t& s) {
            const auto file = s.file;

            elf_opts_t opts{};
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = s.opts.base_addr;

            elf_t elf{};
            elf::init(elf, opts);
            defer(elf::free(elf));

            for (u32 i = 0; i < file->sections.size; ++i) {
                auto section = &file->sections[i];
                switch (section->type) {
                    case section::type_t::data:
                    case section::type_t::code:
                    case section::type_t::debug:
                    case section::type_t::custom: {
                        auto& hdr = array::append(elf.sections);
                        hdr = {};
                        hdr.section = section;
                        hdr.number  = elf.sections.size;
                        break;
                    }
                    case section::type_t::tls:
                    case section::type_t::reloc:
                    case section::type_t::import:
                    case section::type_t::export_:
                    case section::type_t::resource:
                    case section::type_t::exception: {
                        auto& hdr = array::append(elf.segments);
                        hdr = {};
                        hdr.section = section;
                        hdr.number  = elf.segments.size;
                        break;
                    }
                }
            }

            switch (file->machine) {
                case machine::type_t::unknown:
                    return status_t::invalid_machine_type;
                case machine::type_t::x86_64:
                    elf.machine = elf::machine::x86_64;
                    break;
                case machine::type_t::aarch64:
                    elf.machine = elf::machine::aarch64;
                    break;
            }

            status_t status;

            elf::write_header(s, elf);

            status = session::save(s);
            if (!OK(status))
                return status_t::write_error;

            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            using type_t = binfmt::section::type_t;

            g_elf_sys.alloc = alloc;
            name_map::init(g_elf_sys.section_names, g_elf_sys.alloc);
            name_map::init(g_elf_sys.segment_names, g_elf_sys.alloc);

            name_map::add(g_elf_sys.section_names,
                          type_t::code,
                          {
                              .code = true,
                              .data = false,
                              .read = true,
                              .exec = true,
                              .write = false,
                              .alloc = true,
                              .can_free = false
                          },
                          ".text"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = true,
                              .can_free = false
                          },
                          ".data"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = true,
                              .can_free = false
                          },
                          ".rodata"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = false,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = true,
                              .can_free = false
                          },
                          ".bss"_ss);

            return status_t::ok;
        }

        system_t                    g_elf_backend {
            .init   = init,
            .fini   = fini,
            .read   = read,
            .write  = write,
            .type   = type_t::elf
        };
    }

    namespace elf {
        system_t* system() {
            return &internal::g_elf_backend;
        }

        u0 free(elf_t& elf) {
            array::free(elf.sections);
            array::free(elf.segments);
        }

        u64 hash(const u8* name) {
            u64 h = 0, g;
            while (*name) {
                h      = (h << u32(4)) + *name++;
                if ((g = h & 0xf0000000))
                    h ^= g >> u32(24);
                h &= u32(0x0fffffff);
            }
            return h;
        }

        u0 write_header(session_t& s, elf_t& elf) {
            session::write_u8(s, 0x7f);
            session::write_u8(s, 'E');
            session::write_u8(s, 'L');
            session::write_u8(s, 'F');
            session::write_u8(s, class_64);
            session::write_u8(s, data_2lsb);
            session::write_u8(s, version_current);
            session::write_u8(s, os_abi_linux);
            for (u32 i = 0; i < 8; ++i)
                session::write_u8(s, 0);
            session::write_u16(s, u16(file::type_t::exec));
            session::write_u16(s, elf.machine);
            session::write_u32(s, version_current);
            session::write_u64(s, elf.entry_point);
            session::write_u64(s, elf.program.offset);
            session::write_u64(s, elf.section.offset);
            session::write_u32(s, elf.proc_flags);
            session::write_u16(s, elf::file::header_size);
            session::write_u16(s, elf::segment::header_size);
            session::write_u16(s, elf.segments.size);
            session::write_u16(s, elf::section::header_size);
            session::write_u16(s, elf.sections.size);
            session::write_u16(s, elf.str_ndx);
        }

        status_t init(elf_t& elf, const elf_opts_t& opts) {
            elf.alloc       = opts.alloc;
            array::init(elf.sections, elf.alloc);
            array::init(elf.segments, elf.alloc);
            elf.entry_point = opts.entry_point == 0 ? 0x00400000 : opts.entry_point;
            return status_t::ok;
        }

        u0 write_sym(session_t& s, elf_t& elf, elf_sym_t& sym) {
            session::write_u32(s, sym.name_index);
            session::write_u8(s, sym.info);
            session::write_u8(s, sym.other);
            session::write_u16(s, sym.index);
            session::write_u64(s, sym.value);
            session::write_u64(s, sym.size);
        }

        u0 write_dyn(session_t& s, elf_t& elf, s64 tag, u64 value) {
            session::write_s64(s, tag);
            session::write_u64(s, value);
        }

        u0 write_rel(session_t& s, elf_t& elf, u64 offset, u64 info) {
            session::write_u64(s, offset);
            session::write_u64(s, info);
        }

        u0 write_section_header(session_t& s, elf_t& elf, elf_section_hdr_t& hdr) {
            session::write_u32(s, hdr.name_index);
            session::write_u32(s, hdr.type);
            session::write_u64(s, hdr.flags);
            session::write_u64(s, hdr.addr.base);
            session::write_u64(s, hdr.offset);
            session::write_u64(s, hdr.size);
            session::write_u32(s, hdr.link);
            session::write_u32(s, hdr.info);
            session::write_u32(s, 0); // pad
            session::write_u64(s, hdr.addr.align);
            session::write_u64(s, hdr.entry_size);
        }

        u0 write_rela(session_t& s, elf_t& elf, u64 offset, u64 info, s64 addend) {
            session::write_u64(s, offset);
            session::write_u64(s, info);
            session::write_s64(s, addend);
        }

        u0 write_pgm_section_header(session_t& s, elf_t& elf, elf_program_hdr_t& hdr) {
            session::write_u32(s, hdr.type);
            session::write_u32(s, hdr.flags);
            session::write_u64(s, hdr.file.offset);
            session::write_u64(s, hdr.addr.virt);
            session::write_u64(s, hdr.addr.phys);
            session::write_u64(s, hdr.file.size);
            session::write_u64(s, hdr.addr.size);
            session::write_u64(s, hdr.align);
        }

        status_t get_section_name(const binfmt::section_t* section, str::slice_t& name) {
            const auto entry = name_map::find(internal::g_elf_sys.section_names,
                                              section->type,
                                              section->flags);
            if (!entry)
                return status_t::cannot_map_section_name;
            name = entry->name;
            return status_t::ok;
        }

        u0 write_note(session_t& s, elf_t& elf, u32 name_size, u32 desc_size, u32 type) {
            session::write_u32(s, name_size);
            session::write_u32(s, desc_size);
            session::write_u32(s, type);
        }
    }
}
