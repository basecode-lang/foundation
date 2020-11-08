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
            format::print_hex_dump(elf.magic, 16, false, false);
            format::print("  Class:                             {}\n", elf.magic[4]);
            format::print("  Data:                              {}\n", elf.magic[5]);
            format::print("  Version:                           {}\n", elf.magic[6]);
            format::print("  OS/ABI:                            {}\n", elf.magic[7]);
            format::print("  ABI Version:                       {}\n", elf.magic[8]);
            format::print("  Type:                              {}\n", elf.file_type);
            format::print("  Machine:                           {}\n", elf.machine);
            format::print("  Version:                           {}\n", 1);
            format::print("  Entry point address:               {}\n", elf.entry_point);
            format::print("  Start of program headers:          {} (bytes into file)\n", elf.segment.offset);
            format::print("  Start of section headers:          {} (bytes into file)\n", elf.section.offset);
            format::print("  Flags:                             0x{:x}\n", elf.proc_flags);
            format::print("  Size of this header:               {} (bytes)\n", file::header_size);
            format::print("  Size of program headers:           {} (bytes)\n", elf.segment.count > 0 ? segment::header_size : 0);
            format::print("  Number of program headers:         {}\n", elf.segment.count);
            format::print("  Size of section headers:           {} (bytes)\n", elf.section.count > 0 ? section::header_size : 0);
            format::print("  Number of section headers:         {}\n", elf.section.count);
            format::print("  Section header string table index: {}\n\n", elf.str_ndx);
            format::print("Section Headers:\n");
            format::print("  [Nr] Name              Type             Address           Offset\n");
            format::print("       Size              EntSize          Flags  Link  Info  Align\n");

            for (const auto& hdr : elf.headers) {
                if (hdr.type != header_type_t::section)
                    continue;
                const auto& sc = hdr.subclass.section;
                format::print("  [{:>}]\n", hdr.number);
                format::print("       {:016x}  {:016x}\n", sc.size, sc.entry_size);
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

            if (!OK(buf::map(file.buf, file.path)))
                return status_t::read_error;
            defer(buf::unmap(file.buf));

            status_t status;

            opts_t opts{};
            opts.file        = &file;
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = {};

            elf_t elf{};
            status = elf::init(elf, opts);
            if (!OK(status))
                return status;
            defer(elf::free(elf));

            status = read_file_header(file, elf);
            if (!OK(status))
                return status;

            status = read_sections(file, elf);
            if (!OK(status))
                return status;

            stopwatch::stop(timer);
            stopwatch::print_elapsed("binfmt read ELF obj time"_ss, 40, timer);

            format_elf(elf);

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            using type_t = basecode::binfmt::section::type_t;

            const auto module = file.module;
            auto& module_sc   = module->subclass.object;

            opts_t opts{};
            opts.file        = &file;
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = file.opts.base_addr;

            status_t status{};

            elf_t elf{};
            status = elf::init(elf, opts);
            if (!OK(status))
                return status;
            defer(elf::free(elf));

            elf.section.offset = elf::file::header_size;

            u64 rva             {elf.entry_point};
            str::slice_t name   {};

            for (u32 i = 0; i < module_sc.sections.size; ++i) {
                auto          section = &module_sc.sections[i];
                u64           size{};
                u64           flags{};
                u64           entry_size{};
                u32           number{};
                u32           alignment;
                u32           section_type{};
                header_type_t header_type{};

                switch (section->type) {
                    case type_t::tls: {
                        break;
                    }
                    case type_t::data: {
                        number       = ++elf.section.count;
                        header_type  = elf::header_type_t::section;
                        if (!section->flags.init) {
                            section_type = elf::section::type::nobits;
                            size         = section->subclass.size;
                        } else {
                            section_type = elf::section::type::progbits;
                            size         = section->subclass.data.length;
                            auto& block = array::append(elf.blocks);
                            block.type       = elf::block_type_t::slice;
                            block.offset     = elf.data.rel_offset;
                            block.data.slice = &section->subclass.data;
                        }
                        flags = elf::section::flags::alloc;
                        if (section->flags.write)
                            flags |= elf::section::flags::write;
                        if (section->flags.exec)
                            flags |= elf::section::flags::exec_instr;
                        break;
                    }
                    case type_t::code: {
                        number       = ++elf.section.count;
                        header_type  = elf::header_type_t::section;
                        section_type = elf::section::type::progbits;
                        size         = section->subclass.data.length;
                        flags        = elf::section::flags::alloc | elf::section::flags::exec_instr;
                        auto& block = array::append(elf.blocks);
                        block.type       = elf::block_type_t::slice;
                        block.offset     = elf.data.rel_offset;
                        block.data.slice = &section->subclass.data;
                        break;
                    }
                    case type_t::debug: {
                        break;
                    }
                    case type_t::reloc: {
                        break;
                    }
                    case type_t::import: {
                        // .dyntab
                        //      - DT_STRTAB
                        //      - DT_STRSZ
                        //      - DT_SYMTAB
                        //      - DT_SYMENT
                        //      - DT_REL        (.reltab)
                        //      - DT_RELSZ
                        //      - DT_RELENT
                        //      - DT_HASH
                        //      - DT_NEEDED     (shared library string name)
                        //      - DT_NULL
                        break;
                    }
                    case type_t::custom: {
                        break;
                    }
                    case type_t::export_: {
                        break;
                    }
                    case type_t::resource: {
                        break;
                    }
                    case type_t::exception: {
                        break;
                    }
                }

                auto& hdr = array::append(elf.headers);
                hdr.section = section;
                hdr.number  = number;
                hdr.type    = header_type;

                alignment = std::max<u32>(section->align, sizeof(u64));

                switch (header_type) {
                    case header_type_t::none:
                        return status_t::invalid_section_type;
                    case header_type_t::section: {
                        auto& sc = hdr.subclass.section;
                        sc.flags = flags;
                        sc.type  = section_type;
                        status = elf::get_section_name(section, name);
                        if (!OK(status))
                            return status;
                        sc.addr.base  = rva;
                        sc.addr.align = alignment;
                        sc.entry_size = entry_size;
                        if (section->flags.init) {
                            sc.offset = elf.data.rel_offset;
                            sc.size   = size;
                        } else {
                            sc.offset = sc.size = {};
                        }
                        sc.name_index = elf::strtab::add_str(elf.section_names, name);
                        break;
                    }
                    case header_type_t::segment: {
                        break;
                    }
                }

                rva = align(rva + size, alignment);
                if (section->flags.init)
                    elf.data.rel_offset = align(elf.data.rel_offset + size, alignment);
            }

            if (module->symbols.size > 0) {
                elf::symtab::rehash(elf.symbols, module->symbols.size + 1);
                elf::symtab::add_sym(elf.symbols, nullptr);

                // XXX: sort the symbols list based on ELF ordering rules
                //
                hashtab::for_each_pair(module->symbols,
                                       [](const auto idx, const auto& key, auto& symbol, auto* user) -> u32 {
                                           auto& elf = *(elf_t*)user;
                                           auto status = elf::symtab::add_sym(elf.symbols, &symbol);
                                           if (!OK(status))
                                               return u32(status);
                                           return 0;
                                       },
                                       &elf);

                auto& strtab = elf::strtab::make_section(elf,
                                                         ".strtab"_ss,
                                                         &elf.strings);
                elf.symbols.link = strtab.number - 1;

                // XXX: first_global_idx is hard coded!!!
                //      need to set this while walking the symbols list
                auto& symtab = elf::symtab::make_section(elf,
                                                         ".symtab"_ss,
                                                         &elf.symbols,
                                                         1);
                elf.symbols.hash.link = symtab.number - 1;

                elf::hash::make_section(elf,
                                        ".hash"_ss,
                                        &elf.symbols.hash);
            }

            if (elf.section.count > 0) {
                auto& hdr = elf::strtab::make_section(elf,
                                                      ".shstrtab"_ss,
                                                      &elf.section_names);
                elf.str_ndx = hdr.number - 1;
            }

            switch (file.file_type) {
                case file_type_t::none:
                case file_type_t::obj:
                    break;
                case file_type_t::exe:
                case file_type_t::dll:
                    // XXX:
                    //      NULL        (of course)
                    //      PT_INTERP   default: /lib/ld-linux.so.2
                    //      PT_LOAD
                    //      PT_DYNAMIC  (import)
                    //      PT_NOTE     (maybe?)
                    //      GNU_EH_FRAME
                    //      GNU_STACK
                    //      GNU_RELRO
                    break;
            }

            elf.data.base_offset = elf.section.offset
                                   + elf.segment.offset
                                   + (elf.section.count * elf::section::header_size)
                                   + (elf.segment.count * elf::segment::header_size);
            for (auto& hdr : elf.headers) {
                switch (hdr.type) {
                    case header_type_t::none:
                        break;
                    case header_type_t::section:
                        hdr.subclass.section.offset += elf.data.base_offset;
                        break;
                    case header_type_t::segment:
                        hdr.subclass.segment.file.offset += elf.data.base_offset;
                        break;
                }
            }

            for (auto& block : elf.blocks)
                block.offset += elf.data.base_offset;

            // XXX: FIXME
            file.buf.mode = buf_mode_t::alloc;

            status = elf::write_file_header(file, elf);
            if (!OK(status))
                return status;

            if (file.file_type == file_type_t::exe
            || file.file_type == file_type_t::dll) {
                status = elf::write_segments(file, elf);
                if (!OK(status))
                    return status;
            }

            status = elf::write_sections(file, elf);
            if (!OK(status))
                return status;

            return elf::write_blocks(file, elf);
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
                              .init = true,
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

    system_t* system() {
        return &internal::g_elf_backend;
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
