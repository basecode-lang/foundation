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
#include <basecode/binfmt/elf.h>
#include <basecode/core/string.h>

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

        static status_t read(file_t& file) {
            UNUSED(file);
            return status_t::read_error;
        }

        static status_t write(file_t& file) {
            const auto module = file.module;

            elf_opts_t opts{};
            opts.file        = &file;
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = file.opts.base_addr;

            elf_t elf{};
            elf::init(elf, opts);
            defer(elf::free(elf));

            switch (file.machine) {
                case machine::type_t::unknown:
                    return status_t::invalid_machine_type;
                case machine::type_t::x86_64:
                    elf.machine = elf::machine::x86_64;
                    break;
                case machine::type_t::aarch64:
                    elf.machine = elf::machine::aarch64;
                    break;
            }

            elf.section.offset = elf::file::header_size;
            u64 rva         = elf.entry_point;
            u64 data_offset = elf.section.offset + ((module->sections.size + 2) * elf::section::header_size);
            switch (file.output_type) {
                case output_type_t::obj:
                    break;
                case output_type_t::lib:
                    break;
                case output_type_t::exe:
                case output_type_t::dll:
                    for (u32 i = 0; i < module->sections.size; ++i) {
                        auto                 section = &module->sections[i];
                        u64                  size{};
                        u64                  flags{};
                        u64                  entry_size{};
                        u32                  number{};
                        elf_header_type_t    header_type{};
                        elf::section::type_t section_type{};

                        switch (section->type) {
                            case section::type_t::tls: {
                                break;
                            }
                            case section::type_t::data: {
                                number       = ++elf.num_section;
                                header_type  = elf_header_type_t::section;
                                if (!section->flags.init) {
                                    section_type = elf::section::type_t::nobits;
                                } else {
                                    section_type = elf::section::type_t::progbits;
                                    size         = section->subclass.data.length;
                                }
                                flags = elf::section::flags::alloc;
                                if (section->flags.write)
                                    flags |= elf::section::flags::write;
                                if (section->flags.exec)
                                    flags |= elf::section::flags::exec_instr;
                                break;
                            }
                            case section::type_t::code: {
                                number       = ++elf.num_section;
                                header_type  = elf_header_type_t::section;
                                section_type = elf::section::type_t::progbits;
                                size         = section->subclass.data.length;
                                flags        = elf::section::flags::alloc | elf::section::flags::exec_instr;
                                break;
                            }
                            case section::type_t::debug: {
                                break;
                            }
                            case section::type_t::reloc: {
                                break;
                            }
                            case section::type_t::import: {
                                break;
                            }
                            case section::type_t::custom: {
                                break;
                            }
                            case section::type_t::export_: {
                                break;
                            }
                            case section::type_t::resource: {
                                break;
                            }
                            case section::type_t::exception: {
                                break;
                            }
                        }

                        auto& hdr = array::append(elf.headers);
                        hdr.section = section;
                        hdr.number  = number;
                        hdr.type    = header_type;

                        switch (header_type) {
                            case elf_header_type_t::none:
                                return status_t::invalid_section_type;
                            case elf_header_type_t::section: {
                                auto& sc = hdr.subclass.section;
                                sc.flags      = flags;
                                sc.type       = u32(section_type);
                                str::slice_t name{};
                                auto status = elf::get_section_name(section, name);
                                if (!OK(status))
                                    return status;
                                sc.slice      = section->subclass.data;
                                sc.addr.base  = rva;
                                sc.size       = size;
                                sc.offset     = data_offset;
                                sc.entry_size = entry_size;
                                // XXX: shouldn't be hard coded
                                sc.addr.align = sizeof(u64);
                                sc.name_index = elf::strtab::add_str(elf.section_names, name);
                                break;
                            }
                            case elf_header_type_t::segment: {
                                break;
                            }
                        }

                        rva         = align(rva + size, sizeof(u64));
                        data_offset = align(data_offset + size, sizeof(u64));
                    }
                    break;
            }

            if (elf.num_section > 0) {
                elf.str_ndx = ++elf.num_section;

                auto& hdr = array::append(elf.headers);
                hdr.section = {};
                hdr.number  = elf.str_ndx;
                hdr.type    = elf_header_type_t::section;

                auto& sc = hdr.subclass.section;
                sc.flags      = 0;
                sc.type       = u32(elf::section::type_t::strtab);
                sc.offset     = data_offset;
                sc.entry_size = 0;
                sc.addr.align = sizeof(u64);
                const auto name = string::interned::fold(".shstrtab"_ss);
                sc.name_index = elf::strtab::add_str(elf.section_names, name);
                sc.size       = elf.section_names.size;
                data_offset   = align(data_offset + sc.size, sizeof(u64));
            }

            status_t status;

            elf::write_file_header(file, elf);
            if (file.output_type == output_type_t::exe
            ||  file.output_type == output_type_t::dll) {
                elf::write_segments(file, elf);
            }
            elf::write_pad_section(file, elf);
            elf::write_sections(file, elf);

            status = file::save(file);
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
        namespace strtab {
            u0 free(elf_strtab_t& strtab) {
                array::free(strtab.strings);
            }

            u0 write(elf_strtab_t& strtab, file_t& file) {
                auto& s = *file.session;
                session::write_u8(s, 0);
                for (const auto& str : strtab.strings)
                    session::write_cstr(s, str);
            }

            u0 init(elf_strtab_t& strtab, alloc_t* alloc) {
                array::init(strtab.strings, alloc);
                strtab.size = 1;
            }

            u32 add_str(elf_strtab_t& strtab, str::slice_t str) {
                const auto offset = strtab.size;
                array::append(strtab.strings, str);
                strtab.size += str.length + 1;
                return offset;
            }
        }

        system_t* system() {
            return &internal::g_elf_backend;
        }

        u0 free(elf_t& elf) {
            array::free(elf.headers);
            strtab::free(elf.strings);
            strtab::free(elf.section_names);
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

        u0 write_symbol_hash(file_t& file, elf_t& elf) {
        }

        u0 write_pad_section(file_t& file, elf_t& elf) {
            auto& s = *file.session;
            session::write_u32(s, 0);
            session::write_u32(s, 0);
            session::write_u64(s, 0);
            session::write_u64(s, 0);
            session::write_u64(s, 0);
            session::write_u64(s, 0);
            session::write_u32(s, 0);
            session::write_u32(s, 0);
            session::write_u64(s, 0);
            session::write_u64(s, 0);
        }

        u0 write_file_header(file_t& file, elf_t& elf) {
            auto& s = *file.session;
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
            switch (file.output_type) {
                case output_type_t::obj:
                    session::write_u16(s, u16(file::type_t::rel));
                    break;
                case output_type_t::lib:
                    // XXX
                    session::write_u16(s, u16(file::type_t::none));
                    break;
                case output_type_t::exe:
                    session::write_u16(s, u16(file::type_t::exec));
                    break;
                case output_type_t::dll:
                    session::write_u16(s, u16(file::type_t::dyn));
                    break;
            }
            session::write_u16(s, elf.machine);
            session::write_u32(s, version_current);
            session::write_u64(s, elf.entry_point);
            session::write_u64(s, elf.program.offset);
            session::write_u64(s, elf.section.offset);
            session::write_u32(s, elf.proc_flags);
            session::write_u16(s, elf::file::header_size);
            session::write_u16(s, elf::segment::header_size);
            session::write_u16(s, elf.num_segment);
            session::write_u16(s, elf::section::header_size);
            session::write_u16(s, elf.num_section + 1);
            session::write_u16(s, elf.str_ndx);
        }

        u0 write_symbol_table(file_t& file, elf_t& elf) {
        }

        u0 write_string_table(file_t& file, elf_t& elf) {
        }

        status_t write_segments(file_t& file, elf_t& elf) {
            for (const auto& hdr : elf.headers) {
                if (hdr.type != elf_header_type_t::segment)
                    continue;
                auto status = write_header(file, elf, hdr);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        status_t write_sections(file_t& file, elf_t& elf) {
            for (const auto& hdr : elf.headers) {
                if (hdr.type != elf_header_type_t::section)
                    continue;
                auto status = write_header(file, elf, hdr);
                if (!OK(status))
                    return status;
            }
            for (const auto& hdr : elf.headers) {
                if (hdr.type != elf_header_type_t::section)
                    continue;
                const auto& sc = hdr.subclass.section;
                session::seek(*file.session, sc.offset);
                if (sc.type == u32(elf::section::type_t::strtab)) {
                    strtab::write(elf.section_names, file);
                } else {
                    session::write_str(*file.session, sc.slice);
                }
            }
            return status_t::ok;
        }

        status_t init(elf_t& elf, const elf_opts_t& opts) {
            elf.alloc       = opts.alloc;
            array::init(elf.headers, elf.alloc);
            strtab::init(elf.strings, elf.alloc);
            strtab::init(elf.section_names, elf.alloc);
            elf.entry_point = opts.entry_point == 0 ? 0x00400000 : opts.entry_point;
            return status_t::ok;
        }

        u0 write_sym(file_t& file, elf_t& elf, const elf_sym_t& sym) {
            auto& s = *file.session;
            session::write_u32(s, sym.name_index);
            session::write_u8(s, sym.info);
            session::write_u8(s, sym.other);
            session::write_u16(s, sym.index);
            session::write_u64(s, sym.value);
            session::write_u64(s, sym.size);
        }

        u0 write_dyn(file_t& file, elf_t& elf, const elf_dyn_t& dyn) {
            auto& s = *file.session;
            session::write_s64(s, dyn.tag);
            session::write_u64(s, dyn.value);
        }

        u0 write_rel(file_t& file, elf_t& elf, const elf_rel_t& rel) {
            auto& s = *file.session;
            session::write_u64(s, rel.offset);
            session::write_u64(s, rel.info);
        }

        u0 write_note(file_t& file, elf_t& elf, const elf_note_t& note) {
            auto& s = *file.session;
            const auto name_aligned_len = align(note.name.length, sizeof(u64));
            const auto desc_aligned_len = align(note.descriptor.length, sizeof(u64));
            session::write_u64(s, note.name.length);
            session::write_u64(s, note.descriptor.length);
            session::write_str(s, note.name);
            session::write_u8(s, 0);
            for (u32 i = 0; i < std::min<s32>(0, (name_aligned_len - note.name.length) + 1); ++i) {
                session::write_u8(s, 0);
            }
            session::write_str(s, note.descriptor);
            for (u32 i = 0; i < std::min<s32>(0, (desc_aligned_len - note.descriptor.length) + 1); ++i) {
                session::write_u8(s, 0);
            }
            session::write_u8(s, 0);
            session::write_u64(s, note.type);
        }

        u0 write_rela(file_t& file, elf_t& elf, const elf_rela_t& rela) {
            auto& s = *file.session;
            session::write_u64(s, rela.offset);
            session::write_u64(s, rela.info);
            session::write_s64(s, rela.addend);
        }

        status_t write_header(file_t& file, elf_t& elf, const elf_header_t& hdr) {
            auto& s = *file.session;
            switch (hdr.type) {
                case elf_header_type_t::section: {
                    auto& sc = hdr.subclass.section;
                    session::write_u32(s, sc.name_index);
                    session::write_u32(s, sc.type);
                    session::write_u64(s, sc.flags);
                    session::write_u64(s, sc.addr.base);
                    session::write_u64(s, sc.offset);
                    session::write_u64(s, sc.size);
                    session::write_u32(s, sc.link);
                    session::write_u32(s, sc.info);
                    session::write_u64(s, sc.addr.align);
                    session::write_u64(s, sc.entry_size);

                    break;
                }
                case elf_header_type_t::segment: {
                    auto& sc = hdr.subclass.segment;
                    session::write_u32(s, sc.type);
                    session::write_u32(s, sc.flags);
                    session::write_u64(s, sc.file.offset);
                    session::write_u64(s, sc.addr.virt);
                    session::write_u64(s, sc.addr.phys);
                    session::write_u64(s, sc.file.size);
                    session::write_u64(s, sc.addr.size);
                    session::write_u64(s, sc.align);
                    break;
                }
                default:
                    return status_t::invalid_section_type;
            }
            return status_t::ok;
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
    }
}
