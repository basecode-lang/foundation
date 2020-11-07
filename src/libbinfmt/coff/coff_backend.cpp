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

#include <basecode/binfmt/coff.h>

namespace basecode::binfmt::io::coff {
    namespace internal {
        struct coff_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
        };

        coff_system_t               g_coff_sys{};

        static u0 fini() {
            name_map::free(g_coff_sys.section_names);
        }

        static status_t read(file_t& file) {
            if (file.file_type != file_type_t::obj)
                return status_t::invalid_input_type;

            if (!OK(buf::map(file.buf, file.path)))
                return status_t::read_error;
            defer(buf::unmap(file.buf));

            coff_t coff{};
            auto status = coff::init(coff, file, g_coff_sys.alloc);
            if (!OK(status))
                return status;
            defer(coff::free(coff));

            status = coff::read_header(file, coff);
            if (!OK(status))
                return status_t::read_error;

            status = coff::read_symbol_table(file, coff);
            if (!OK(status))
                return status_t::read_error;

            status = coff::read_section_headers(file, coff);
            if (!OK(status))
                return status_t::read_error;

//                format::print("{} ", FLAG_CHK(hdr.flags, section::content_code) ? "X" : " ");
//                format::print("{} ", FLAG_CHK(hdr.flags, section::data_init) ? "X" : " ");
//                format::print("{} ", FLAG_CHK(hdr.flags, section::data_uninit) ? "X" : " ");
//                if (FLAG_CHK(hdr.flags, section::link_info)) {
//                    format::print("I ");
//                } else if (FLAG_CHK(hdr.flags, section::link_comdat)) {
//                    format::print("C ");
//                } else {
//                    format::print("  ");
//                }
//                format::print("{} ", (hdr.flags & section::memory_shared) == section::memory_shared ? "X" : " ");
//                format::print("{} ", (hdr.flags & section::memory_execute) == section::memory_execute ? "X" : " ");
//                format::print("{} ", (hdr.flags & section::memory_read) == section::memory_read ? "X" : " ");
//                format::print("{} ", (hdr.flags & section::memory_write) == section::memory_write ? "X" : " ");
//            format::print("No Name             RVA      Size     Raw Off  Raw Size   Align\n");
//            for (auto& hdr : coff.headers) {
//                const auto sym = coff::section::get_symbol(coff, hdr);
//                format::print("{:02} ", hdr.number);
//                format::print("{:<16} ", sym ? sym->subclass.sym.name : "<NO NAME>"_ss);
//                format::print("{:08x} {:08x} {:08x} {:08x} {:04x}",
//                              hdr.rva.base,
//                              hdr.rva.size,
//                              hdr.file.offset,
//                              hdr.file.size,
//                              16);
//                format::print("\n");
//                if (hdr.relocs.file.size > 0) {
//                    for (u32 i = 0; i < hdr.relocs.file.size; ++i) {
//                        auto reloc = coff::reloc::get(coff, hdr, i);
//                        format::print("{:<32} {:08x} ",
//                                      coff.machine == coff::machine::amd64 ? coff::reloc::type::x86_64::name(reloc.type) :
//                                                                                  coff::reloc::type::aarch64::name(reloc.type),
//                                      reloc.rva);
//                        const auto& reloc_sym = coff.symtab.list[reloc.symtab_idx];
//                        format::print("{} ", reloc_sym.subclass.sym.name);
//                        format::print("\n");
//                    }
//                    format::print("\n");
//                }
//            }
//
//            format::print("\nNo  Sec Value            Class  Type Name\n");
//            u32 i{};
//            for (const auto& sym : coff.symtab.list) {
//                auto sc = &sym.subclass.sym;
//                format::print("{:>03} ", i++);
//                format::print("{:>03} ", sc->section);
//                format::print("{:016x} ", sc->value);
//                switch (sc->sclass) {
//                    case symtab::sclass::file: {
//                        format::print("FILE   ");
//                        break;
//                    }
//                    case symtab::sclass::static_: {
//                        format::print("STATIC ");
//                        break;
//                    }
//                    case symtab::sclass::function: {
//                        format::print("FUNC   ");
//                        break;
//                    }
//                    case symtab::sclass::external_: {
//                        format::print("EXTERN ");
//                        break;
//                    }
//                    default:
//                        format::print("NONE   ");
//                        break;
//                }
//                switch (sc->type) {
//                    case symtab::type::function: {
//                        format::print("FUNC ");
//                        break;
//                    }
//                    default:
//                        format::print("NONE ");
//                        break;
//                }
//                format::print("{} ", sc->name);
//                format::print("\n");
//            }

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            UNUSED(file);
            return status_t::write_error;
        }

        static status_t init(alloc_t* alloc) {
            using type_t = basecode::binfmt::section::type_t;

            g_coff_sys.alloc = alloc;

            name_map::init(g_coff_sys.section_names, g_coff_sys.alloc);

            name_map::add(g_coff_sys.section_names,
                          type_t::tls,
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
                          type_t::code,
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
                          type_t::data,
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
                          type_t::data,
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
                          type_t::debug,
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
                          type_t::data,
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
                          type_t::import,
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
                          type_t::export_,
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
                          type_t::resource,
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
                          type_t::exception,
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

    system_t* system() {
        return &internal::g_coff_backend;
    }

    status_t get_section_name(const binfmt::section_t* section, str::slice_t& name) {
        const auto entry = name_map::find(internal::g_coff_sys.section_names, section->type, section->flags);
        if (!entry)
            return status_t::cannot_map_section_name;
        name = entry->name;
        return status_t::ok;
    }
}
