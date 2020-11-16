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

#include <basecode/binfmt/cv.h>
#include <basecode/binfmt/coff.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/slice_utils.h>

namespace basecode::binfmt::io::coff {
    namespace internal {
        struct coff_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
        };

        coff_system_t               g_coff_sys{};

        static u0 format_coff(coff_t& coff, file_t& file) {
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
            format::print("No Name             RVA      Size     Raw Off  Raw Size Align Symbol\n");
            for (auto& hdr : coff.headers) {
                format::print("{:02} ", hdr.number);
                format::print("{:<16} ", hdr.name);
                format::print("{:08x} {:08x} {:08x} {:08x} {:04x}",
                              hdr.rva.base,
                              hdr.rva.size,
                              hdr.file.offset,
                              hdr.file.size,
                              16);
                format::print("\n");
                if (hdr.line_nums.file.size > 0) {
                    for (u32 i = 0; i < hdr.line_nums.file.size; ++i) {
                        auto line_num = coff::line_num::get(coff, hdr, i);
                        format::print("   {:<16} {:08x} {:<32} ",
                                      "COFF line_num",
                                      0,
                                      line_num.number == 0 ? "SYMBOL_TABLE_REF" : "FUNC_LINE_NUM");
                        format::print("\n");
                    }
                }
                if (hdr.relocs.file.size > 0) {
                    for (u32 i = 0; i < hdr.relocs.file.size; ++i) {
                        auto reloc = coff::reloc::get(coff, hdr, i);
                        auto reloc_name = coff.machine == coff::machine::amd64 ? coff::reloc::type::x86_64::name(reloc.type) :
                                          coff::reloc::type::aarch64::name(reloc.type);
                        format::print("   {:<16} {:08x} {:<32} ",
                                      "COFF reloc",
                                      reloc.rva,
                                      reloc_name);
                        const auto& reloc_sym = coff.symtab.list[reloc.symtab_idx];
                        format::print("{} ", reloc_sym.subclass.sym.name);
                        format::print("\n");
                    }
                    format::print("\n");
                }
                if (hdr.name == ".drectve"_ss) {
                    auto data = slice::make(coff.buf + hdr.file.offset, hdr.file.size);
                    array_t<str::slice_t> args{};
                    array::init(args, coff.alloc);
                    defer(array::free(args));
                    slice::to_fields(data, args, ' ');
                    format::print("   FLAGS:");
                    for (auto& arg : args)
                        format::print("      {}\n", arg);
                    format::print("\n");
                } else if (hdr.name == ".pdata"_ss) {
                    u32 idx{};
                    unwind_t u{};
                    format::print("    UNWINDS:\n");
                    while (OK(coff::unwind::get(coff, hdr, idx++, u))) {
                        format::print("      BEGIN: {:08x} END: {:08x} INFO: {:08x}\n",
                                      u.begin_rva,
                                      u.end_rva,
                                      u.info);
                    }
                    format::print("\n");
                } else if (hdr.name == ".xdata"_ss) {
                    format::print("    FREE FORMAT EXCEPTION DATA:\n");
                    format::print_hex_dump(coff.buf + hdr.file.offset, hdr.file.size, false, true, 6);
                    format::print("\n");
                } else if (hdr.name == ".debug$S"_ss) {
                    format::print("    SYMBOLIC DEBUG DATA:\n");
                    format::print_hex_dump(coff.buf + hdr.file.offset, hdr.file.size, false, true, 6);
                    format::print("\n");

                    cv::cv_t cv{};
                    cv::init(cv, coff.alloc);
                    defer(cv::free(cv));

                    if (!OK(cv::read_symbol_data(cv, file, hdr.file.offset, hdr.file.size))) {
                        format::print("crap! reading symbols sucks\n");
                    }
                } else if (hdr.name == ".debug$T"_ss || hdr.name == ".debug$P"_ss) {
                    format::print("    TYPE INFO DEBUG DATA:\n");
                    format::print_hex_dump(coff.buf + hdr.file.offset, hdr.file.size, false, true, 6);
                    format::print("\n");

                    cv::cv_t cv{};
                    cv::init(cv, coff.alloc);
                    defer(cv::free(cv));

                    if (!OK(cv::read_type_data(cv, file, hdr.file.offset, hdr.file.size))) {
                        format::print("crap! reading types sucks\n");
                    }
                }
            }

            format::print("\nNo  Sect  Value            Class  Type Name\n");
            u32 idx{};
            for (const auto& sym : coff.symtab.list) {
                switch (sym.type) {
                    case sym_type_t::sym: {
                        auto sc = &sym.subclass.sym;
                        format::print("{:>03} ", idx);
                        switch (sc->section) {
                            case symtab::section::undef:
                                format::print("UNDEF ");
                                break;
                            case symtab::section::absolute:
                                format::print("ABS   ");
                                break;
                            case symtab::section::debug:
                                format::print("DEBUG ");
                                break;
                            default:
                                format::print("{:>05} ", sc->section);
                                break;
                        }
                        format::print("{:016x} ", sc->value);
                        switch (sc->sclass) {
                            case symtab::sclass::file: {
                                format::print("FILE   ");
                                break;
                            }
                            case symtab::sclass::static_: {
                                format::print("STATIC ");
                                break;
                            }
                            case symtab::sclass::function: {
                                format::print("FUNC   ");
                                break;
                            }
                            case symtab::sclass::external_: {
                                format::print("EXTERN ");
                                break;
                            }
                            default:
                                format::print("NONE   ");
                                break;
                        }
                        switch (sc->type) {
                            case symtab::type::function: {
                                format::print("FUNC ");
                                break;
                            }
                            default:
                                format::print("NONE ");
                                break;
                        }
                        format::print("{} \n", sc->name);
                        break;
                    }
                    case sym_type_t::aux_xf: {
                        auto sc = &sym.subclass.aux_xf;
                        format::print("                           AUX    XF   {}\n", sc->line_num);
                        break;
                    }
                    case sym_type_t::aux_file: {
                        auto sc = &sym.subclass.aux_file;
                        format::print("                           AUX    FILE {}\n", slice::make(sc->bytes, sizeof(sc->bytes)));
                        break;
                    }
                    case sym_type_t::aux_section: {
                        auto sc = &sym.subclass.aux_section;
                        format::print("                           AUX    SECT LEN: {:08x} CHKSUM: {:08x} {}: {:03} SEL: {}\n",
                                      sc->len,
                                      sc->check_sum,
                                      sc->comdat_sel == 0 ? "NUM" : "ASSOC",
                                      sc->sect_num,
                                      coff::comdat::name(sc->comdat_sel));
                        break;
                    }
                    case sym_type_t::aux_func_def: {
                        auto sc = &sym.subclass.aux_func_def;
                        format::print("                           AUX    FUNC {}\n", sc->tag_idx);
                        break;
                    }
                    case sym_type_t::aux_token_def: {
                        auto sc = &sym.subclass.aux_token_def;
                        format::print("                           AUX    TOKE {}\n", sc->symtab_idx);
                        break;
                    }
                    case sym_type_t::aux_weak_extern: {
                        auto sc = &sym.subclass.aux_weak_extern;
                        format::print("                           AUX    WEAK {}\n", sc->tag_idx);
                        break;
                    }
                }
                ++idx;
            }
        }

        static u0 fini() {
            name_map::free(g_coff_sys.section_names);
        }

        static status_t read(file_t& file) {
            stopwatch_t timer{};
            stopwatch::start(timer);

            if (file.file_type != file_type_t::obj)
                return status_t::invalid_input_type;

            status_t status;

            status = io::file::map_existing(file);
            if (!OK(status))
                return status_t::read_error;

            coff_t coff{};
            status = coff::init(coff, file, g_coff_sys.alloc);
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

            stopwatch::stop(timer);
            stopwatch::print_elapsed("binfmt read COFF obj time"_ss, 40, timer);

            format_coff(coff, file);

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

//            name_map::add(g_coff_sys.section_names,
//                          type_t::tls,
//                          {
//                              .code = false,
//                              .init = true,
//                              .exec = false,
//                              .write = true,
//                              .alloc = true,
//                          },
//                          ".tls"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::code,
                          {
                              .code = true,
                              .exec = true,
                              .write = false,
                          },
                          ".text"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = true,
                          },
                          ".data"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".rdata"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::debug,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".debug"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::data,
                          {
                              .code = false,
                              .init = false,
                              .exec = false,
                              .write = true,
                          },
                          ".bss"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::import,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = true,
                          },
                          ".idata"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::export_,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".edata"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::rsrc,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
                          },
                          ".rsrc"_ss);

            name_map::add(g_coff_sys.section_names,
                          type_t::unwind,
                          {
                              .code = false,
                              .init = true,
                              .exec = false,
                              .write = false,
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
        if (section->name_offset > 0) {
            const auto strtab_section = binfmt::module::get_section(*section->module, section->module->subclass.object.strtab);
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

        const auto entry = name_map::find(internal::g_coff_sys.section_names,
                                          section->type,
                                          flags);
        if (!entry)
            return status_t::cannot_map_section_name;
        name = entry->name;
        return status_t::ok;
    }
}
