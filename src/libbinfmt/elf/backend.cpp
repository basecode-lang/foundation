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
#include <basecode/binfmt/ar.h>
#include <basecode/binfmt/elf.h>

namespace basecode::binfmt::io::elf {
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
            auto& s = *file.session;

            switch (file.file_type) {
                case file_type_t::obj:
                    break;
                case file_type_t::lib: {
                    auto status = ar::read(s.ar, file.path);
                    if (!OK(status))
                        return status;

                    // XXX: 1. loop over each file in the archive
                    //      2. read as obj
                    //      3. create module_t matching read obj
                    break;
                }
                case file_type_t::exe:
                case file_type_t::dll:
                    break;
            }

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            using type_t = basecode::binfmt::section::type_t;

            const auto module = file.module;

            opts_t opts{};
            opts.file        = &file;
            opts.alloc       = g_elf_sys.alloc;
            opts.entry_point = file.opts.base_addr;

            elf_t elf{};
            elf::init(elf, opts);
            defer(elf::free(elf));

            elf.section.offset = elf::file::header_size;

            u64 rva             {elf.entry_point};
            str::slice_t name   {};

            for (u32 i = 0; i < module->sections.size; ++i) {
                auto          section = &module->sections[i];
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
                        auto status = elf::get_section_name(section, name);
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
                hashtab::for_each_pair(const_cast<symbol_table_t&>(module->symbols),
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
                case file_type_t::obj:
                    break;
                case file_type_t::lib:
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

            elf::write_file_header(file, elf);

            if (file.file_type == file_type_t::exe
            || file.file_type == file_type_t::dll) {
                auto status = elf::write_segments(file, elf);
                if (!OK(status))
                    return status;
            }

            auto status = elf::write_sections(file, elf);
            if (!OK(status))
                return status;

            elf::write_blocks(file, elf);

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
