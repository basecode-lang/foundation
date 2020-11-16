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
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stopwatch.h>

namespace basecode::binfmt::io::elf {
    namespace internal {
        struct elf_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
            name_list_t             segment_names;
        };

        elf_system_t                g_elf_sys{};

        static u0 format_elf(const elf_t& elf) {
            format::print("ELF Header:\n");
            format::print("  Magic:    ");
            format::print_hex_dump(elf.file_header->magic, 16, false, false);
            format::print("  Class:                             {}\n", elf::file::class_name(elf.file_header->magic[elf::file::magic_class]));
            format::print("  Data:                              {}\n", elf::file::endianess_name(elf.file_header->magic[elf::file::magic_data]));
            format::print("  Version:                           {}\n", elf::file::version_name(elf.file_header->magic[elf::file::magic_version]));
            format::print("  OS/ABI:                            {}\n", elf::file::os_abi_name(elf.file_header->magic[elf::file::magic_os_abi]));
            format::print("  ABI Version:                       {}\n", elf.file_header->magic[elf::file::magic_abi_version]);
            format::print("  Type:                              {}\n", elf::file::file_type_name(elf.file_header->type));
            format::print("  Machine:                           {}\n", elf::machine::name(elf.file_header->machine));
            format::print("  Version:                           0x{:x}\n", elf.file_header->version);
            format::print("  Entry point address:               0x{:x}\n", elf.file_header->entry_point);
            format::print("  Start of program headers:          {} (bytes into file)\n", elf.file_header->pgm_hdr_offset);
            format::print("  Start of section headers:          {} (bytes into file)\n", elf.file_header->sect_hdr_offset);
            format::print("  Flags:                             0x{:x}\n", elf.file_header->flags);
            format::print("  Size of this header:               {} (bytes)\n", file::header_size);
            format::print("  Size of program headers:           {} (bytes)\n", elf.file_header->pgm_hdr_size);
            format::print("  Number of program headers:         {}\n", elf.file_header->pgm_hdr_count);
            format::print("  Size of section headers:           {} (bytes)\n", elf.file_header->sect_hdr_size);
            format::print("  Number of section headers:         {}\n", elf.file_header->sect_hdr_count);
            format::print("  Section header string table index: {}\n\n", elf.file_header->strtab_ndx);
            format::print("Section Headers:\n");
            format::print("  [Nr] Name              Type             Address           Offset\n");
            format::print("       Size              EntSize          Flags  Link  Info  Align\n");

            s8 name[17];
            name[16] = '\0';

            s8 flag_chars[14];
            auto strtab = ((u8*) (elf.file_header)) + elf.strtab.sect->offset;

            for (u32 i = 0; i < elf.file_header->sect_hdr_count; ++i) {
                const auto& hdr = elf.sections[i];
                std::memcpy(name, strtab + hdr.name_offset, 16);
                elf::section::flags::chars(hdr.flags, flag_chars);

                format::print(" [{:>3}] {:<17} {:<16} {:016x} {:08x}\n",
                              i,
                              name,
                              section::type::name(hdr.type),
                              hdr.addr,
                              hdr.offset);
                format::print("       {:016x}  {:016x} {:<7}{:>4}  {:>4}    {:<}\n",
                              hdr.size,
                              hdr.entity_size,
                              flag_chars,
                              hdr.link,
                              hdr.info,
                              hdr.addr_align);
            }

            const auto symtab_size = elf.symtab.sect->size / elf.symtab.sect->entity_size;

            format::print("\nSymbol table '{}' contains {} entries:\n", ".symtab", symtab_size);
            format::print("  Num:     Value         Size Type     Bind   Vis       Ndx Name\n");
            for (u32 i = 0; i < symtab_size; ++i) {
                auto sym = elf::symtab::get(elf, elf.symtab.ndx, i);
                std::memcpy(name, strtab + sym->name_offset, 16);
                format::print("{:>5}: {:016x} {:>5} NOTYPE   LOCAL   DEFAULT  {} {}\n",
                              i,
                              sym->value,
                              sym->size,
                              sym->section_ndx,
                              name);
            }
        }

        static u0 fini() {
            name_map::free(g_elf_sys.section_names);
            name_map::free(g_elf_sys.segment_names);
        }

        static status_t read(file_t& file) {
            stopwatch_t timer{};
            stopwatch::start(timer);

            if (file.file_type != file_type_t::obj)
                return status_t::invalid_input_type;

            status_t status;

            status = io::file::map_existing(file);
            if (!OK(status))
                return status;

            opts_t opts{};
            opts.file        = &file;
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = {};

            elf_t elf{};
            status = elf::init(elf, opts);
            if (!OK(status))
                return status;
            defer(elf::free(elf));

            status = read(elf, file);
            if (!OK(status))
                return status;

            stopwatch::stop(timer);
            stopwatch::print_elapsed("binfmt ELF read time"_ss, 40, timer);

            format_elf(elf);

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            using section_type_t = binfmt::section::type_t;

            stopwatch_t timer{};
            stopwatch::start(timer);

            status_t status{};
            auto msc = &file.module->subclass.object;

            opts_t opts{};
            opts.file         = &file;
            opts.alloc        = g_elf_sys.alloc;
            opts.entry_point  = file.opts.base_addr;

            // XXX: these need to come in on the file!
            opts.clazz       = elf::class_64;
            opts.endianess   = elf::data_2lsb;
            opts.os_abi      = elf::os_abi_sysv;
            opts.abi_version = 0;
            opts.version     = elf::version_current;

            u32 data_size       {};
            u32 num_segments    {};
            u32 num_sections    {msc->sections.size + 1};

            for (auto& section : msc->sections) {
                const auto alignment = std::max<u32>(section.align, 8);
                switch (section.type) {
                    case section_type_t::data: {
                        if (section.flags.init)
                            data_size = align(data_size + section.size, alignment);
                        break;
                    }
                    default:
                        data_size = align(data_size + section.size, alignment);
                        break;
                }
            }

            opts.header_offset = elf::file::header_size + data_size;
            usize file_size = opts.header_offset
                + (num_segments * segment::header_size)
                + (num_sections * section::header_size);

            status = io::file::map_new(file, file_size);
            if (!OK(status))
                return status;

            elf_t elf{};
            defer(
                elf::free(elf);
                io::file::unmap(file);
                stopwatch::stop(timer);
                stopwatch::print_elapsed("binfmt ELF write time"_ss, 40, timer);
                );

            status = elf::init(elf, opts);
            if (!OK(status))
                return status;

            status = elf::write(elf, file);
            if (!OK(status))
                return status;

            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            using type_t = binfmt::section::type_t;

            g_elf_sys.alloc = alloc;
            name_map::init(g_elf_sys.section_names, g_elf_sys.alloc);
            name_map::init(g_elf_sys.segment_names, g_elf_sys.alloc);

            name_map::add(g_elf_sys.section_names,
                          type_t::init,
                          {
                              .code = true,
                              .init = true,
                              .exec = false,
                              .write = true,
                          },
                          ".init"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::code,
                          {
                              .code = true,
                              .init = true,
                              .exec = true,
                              .write = false,
                          },
                          ".text"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = true,
                          },
                          ".data"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".rodata"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = false,
                              .exec = false,
                              .write = true,
                          },
                          ".bss"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::reloc,
                          {},
                          ".rela"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::reloc,
                          {},
                          ".rela"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::group,
                          {},
                          ".group"_ss);

            name_map::add(g_elf_sys.section_names,
                          type_t::unwind,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".eh_frame"_ss);

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

    system_t* system() {
        return &internal::g_elf_backend;
    }

    status_t get_section_name(const module_t* module,
                              const binfmt::section_t* section,
                              str::slice_t& name) {
        using section_type_t = binfmt::section::type_t;

        if (section->name_offset > 0) {
            const auto strtab_section = binfmt::module::get_section(*module, module->subclass.object.strtab);
            if (!strtab_section)
                return status_t::cannot_map_section_name;
            const auto str = binfmt::string_table::get(strtab_section->subclass.strtab, section->name_offset);
            name.data   = (const u8*) str;
            name.length = strlen(str);
            return status_t::ok;
        }

        name_flags_t flags{};
        flags.pad   = {};
        flags.exec  = section->flags.exec;
        flags.code  = section->flags.code;
        flags.init  = section->flags.init;
        flags.write = section->flags.write;

        switch (section->type) {
            case section_type_t::reloc: {
                const auto entry = name_map::find(internal::g_elf_sys.section_names,
                                                  section->type,
                                                  flags);
                if (!entry)
                    return status_t::cannot_map_section_name;
                if (!section->info) {
                    name = entry->name;
                    break;
                }

                auto linked_section = binfmt::module::get_section(*module, section->info + 1);
                if (!linked_section)
                    return status_t::missing_linked_section;

                if (linked_section->type == section_type_t::custom) {
                    name = string::interned::fold(format::format("{}.custom", entry->name));
                } else {
                    flags.exec  = linked_section->flags.exec;
                    flags.code  = linked_section->flags.code;
                    flags.init  = linked_section->flags.init;
                    flags.write = linked_section->flags.write;
                    const auto linked_entry = name_map::find(internal::g_elf_sys.section_names,
                                                             linked_section->type,
                                                             flags);
                    if (!linked_entry)
                        return status_t::cannot_map_section_name;

                    name = string::interned::fold(format::format("{}{}",
                                                                 entry->name,
                                                                 linked_entry->name));
                }
                break;
            }
            case section_type_t::custom: {
                name = string::interned::fold(".custom");
                break;
            }
            default: {
                const auto entry = name_map::find(internal::g_elf_sys.section_names,
                                                  section->type,
                                                  flags);
                if (!entry)
                    return status_t::cannot_map_section_name;
                name = entry->name;
                break;
            }
        }

        return status_t::ok;
    }
}
