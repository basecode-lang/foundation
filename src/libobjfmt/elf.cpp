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

#include <basecode/objfmt/elf.h>

namespace basecode::objfmt::container {
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

            section_hdr_t hdrs[file->sections.size];
            for (u32 i = 0; i < file->sections.size; ++i) {
                auto& hdr = hdrs[i];
                hdr.section    = &file->sections[i];
                hdr.name.slice = {};
                hdr.name.ndx   = 0;
                hdr.addr       = hdr.offset = hdr.size       = {};
                hdr.align      = hdr.flags  = hdr.entry_size = {};
                hdr.link       = hdr.info   = {};
                hdr.number     = i + 1;
            }

            elf_opts_t opts{};
            opts.alloc = g_elf_sys.alloc;
            opts.headers.section = hdrs;
            opts.headers.size_section = 0;
            opts.headers.num_section = file->sections.size;

            opts.headers.program = {};
            opts.headers.size_program = 0;
            opts.headers.num_program = 0;

            elf_t elf{};
            elf::init(elf, opts);
            defer(elf::free(elf));

            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            using type_t = objfmt::section::type_t;

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
            UNUSED(elf);
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
            for (u32 i = 0; i < 7; ++i)
                session::write_u8(s, 0);
            session::write_u16(s, u16(file::type_t::exec));
            session::write_u16(s, elf.machine);
            session::write_u32(s, version_current);
            session::write_u64(s, elf.entry_point);
            session::write_u64(s, elf.program.offset);
            session::write_u64(s, elf.section.offset);
            session::write_u32(s, elf.proc_flags);
            session::write_u16(s, elf.header_size);
            session::write_u16(s, elf.headers.size_program);
            session::write_u16(s, elf.headers.num_program);
            session::write_u16(s, elf.headers.size_section);
            session::write_u16(s, elf.headers.num_section);
            session::write_u16(s, elf.str_ndx);
        }

        status_t init(elf_t& elf, const elf_opts_t& opts) {
            elf.alloc = opts.alloc;
            return status_t::ok;
        }

        u0 write_section_header(session_t& s, elf_t& elf, section_hdr_t& hdr) {
            session::write_u32(s, hdr.name.ndx);
            session::write_u32(s, u32(hdr.type));
            session::write_u64(s, hdr.flags);
            session::write_u64(s, hdr.addr);
            session::write_u64(s, hdr.offset);
            session::write_u64(s, hdr.size);
            session::write_u32(s, hdr.link);
            session::write_u32(s, hdr.info);
            session::write_u32(s, 0); // pad
            session::write_u64(s, hdr.align);
            session::write_u64(s, hdr.entry_size);
        }

        u0 write_pgm_section_header(session_t& s, elf_t& elf, program_hdr_t& hdr) {
        }

        status_t get_section_name(const objfmt::section_t* section, str::slice_t& name) {
            const auto entry = name_map::find(internal::g_elf_sys.section_names, section->type, section->flags);
            if (!entry)
                return status_t::cannot_map_section_name;
            name = entry->name;
            return status_t::ok;
        }
    }
}
