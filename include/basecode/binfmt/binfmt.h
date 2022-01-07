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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/binfmt/types.h>

namespace basecode::binfmt {
    namespace pe {
        namespace base_reloc {
            namespace type {
                constexpr u8 unknown  = 0;
            }
        }

        namespace dir_entry {
            u0 free(pe_t& pe, pe_dir_type_t type);

            status_t init(pe_t& pe, pe_dir_type_t type);
        }

        u0 free(pe_t& pe);

        status_t init(pe_t& pe, const pe_opts_t& opts);

        status_t write_section_data(file_t& file,
                                    pe_t& pe,
                                    coff_header_t& hdr);

        status_t build_sections(file_t& file, pe_t& pe);

        status_t write_pe_header(file_t& file, pe_t& pe);

        status_t write_dos_header(file_t& file, pe_t& pe);

        status_t write_sections_data(file_t& file, pe_t& pe);

        status_t write_optional_header(file_t& file, pe_t& pe);

        status_t build_section(file_t& file, pe_t& pe, coff_header_t& hdr);
    }

    namespace elf {
        namespace file {
            str::slice_t class_name(u8 cls);

            str::slice_t os_abi_name(u8 os_abi);

            str::slice_t version_name(u8 version);

            str::slice_t file_type_name(u16 type);

            str::slice_t endianess_name(u8 endianess);
        }

        namespace machine {
            str::slice_t name(u16 type);
        }

        namespace dynamic {
            namespace type {
                str::slice_t name(u32 type);
            }
        }

        namespace section {
            namespace type {
                str::slice_t name(u32 type);
            }

            namespace flags {
                const s8* name(u32 flag);

                u0 chars(u32 flags, s8* chars);

                u0 names(u32 flags, const s8* names[13]);
            }
        }

        namespace symtab {
            u64 hash_name(str::slice_t str);

            elf_sym_t* get(const elf_t& elf, u32 sect_num, u32 sym_idx);
        }

        u0 free(elf_t& elf);

        status_t read(elf_t& elf, file_t& file);

        status_t write(elf_t& elf, file_t& file);

        status_t get_section_name(const module_t* module,
                                  const binfmt::section_t* section,
                                  str::slice_t& name);

        status_t init(elf_t& elf, const elf_opts_t& opts);

        u0 format_report(str_buf_t& buf, const elf_t& elf);

        u32 section_alignment(const binfmt::section_t* section);

        u32 section_file_size(const binfmt::section_t* section);
    }

    namespace coff {
        namespace unwind {
            status_t get(const coff_t& coff,
                         const coff_header_t& hdr,
                         u32 idx,
                         coff_unwind_t& u);
        }

        namespace reloc {
            namespace type::x86_64 {
                str::slice_t name(u16 type);
            }

            namespace type::aarch64 {
                str::slice_t name(u16 type);
            }

            coff_reloc_t get(const coff_t& coff,
                             const coff_header_t& hdr,
                             u32 idx);
        }

        namespace section {
            coff_sym_t* get_symbol(coff_t& coff, coff_header_t& hdr);
        }

        namespace flags {
            str::slice_t name(u32 flag);
        }

        namespace comdat {
            str::slice_t name(u32 sel);
        }

        namespace strtab {
            u0 free(coff_t& coff);

            u0 init(coff_t& coff, alloc_t* alloc);

            u32 add(coff_t& coff, str::slice_t str);

            const s8* get(coff_t& coff, u64 offset);
        }

        namespace symtab {
            namespace sclass {
                str::slice_t name(u8 sclass);
            }

            u0 free(coff_t& sym);

            u0 init(coff_t& sym, alloc_t* alloc);

            coff_sym_t* make_symbol(coff_t& coff);

            coff_sym_t* make_aux(coff_t& coff,
                                 coff_sym_t* sym,
                                 coff_sym_type_t type);

            coff_sym_t* find_symbol(coff_t& coff, u64 name);

            coff_sym_t* make_symbol(coff_t& coff, str::slice_t name);

            coff_sym_t* get_aux(coff_t& coff, coff_sym_t* sym, u32 idx);

            coff_sym_t* make_symbol(coff_t& coff, u64 name, u32 offset);
        }

        namespace line_num {
            coff_line_num_t get(const coff_t& coff,
                                const coff_header_t& hdr,
                                u32 idx);
        }

        u0 free(coff_t& coff);

        u0 update_symbol_table(coff_t& coff);

        status_t build_section(file_t& file,
                               coff_t& coff,
                               coff_header_t& hdr);

        status_t get_section_name(const binfmt::section_t* section,
                                  str::slice_t& name);

        status_t write_section_data(file_t& file,
                                    coff_t& coff,
                                    coff_header_t& hdr);

        status_t read_header(file_t& file, coff_t& coff);

        status_t write_section_header(file_t& file,
                                      coff_t& coff,
                                      coff_header_t& hdr);

        status_t write_header(file_t& file, coff_t& coff);

        status_t build_sections(file_t& file, coff_t& coff);

        u0 set_section_flags(file_t& file, coff_header_t& hdr);

        status_t read_symbol_table(file_t& file, coff_t& coff);

        status_t write_string_table(file_t& file, coff_t& coff);

        status_t write_symbol_table(file_t& file, coff_t& coff);

        status_t write_sections_data(file_t& file, coff_t& coff);

        status_t init(coff_t& coff, file_t& file, alloc_t* alloc);

        status_t read_section_headers(file_t& file, coff_t& coff);

        status_t write_section_headers(file_t& file, coff_t& coff);
    }

    namespace macho {
        u0 free(macho_t& macho);

        status_t read(macho_t& macho, file_t& file);

        status_t write(macho_t& macho, file_t& file);

        status_t get_section_name(const module_t* module,
                                  const binfmt::section_t* section,
                                  str::slice_t& name);

        status_t init(macho_t& macho, const macho_opts_t& opts);
    }

    namespace file {
        u0 free(file_t& file);

        status_t save(file_t& file);

        status_t map_existing(file_t& file);

        status_t init(file_t& file, alloc_t* alloc);

        status_t map_new(file_t& file, usize file_size);

        status_t unmap(file_t& file, b8 sync_flush = false);
    }

    namespace name_map {
        u0 add(name_array_t& names,
               section::type_t type,
               name_flags_t flags,
               str::slice_t name);

        u0 free(name_array_t& names);

        u0 init(name_array_t& names, alloc_t* alloc);

        const name_map_t* find(const name_array_t& names,
                               section::type_t type,
                               name_flags_t flags);
    }

    namespace session {
        u0 free(session_t& s);

        status_t read(session_t& s);

        status_t write(session_t& s);

        file_t* add_file(session_t& s,
                         const s8* path,
                         format_type_t bin_type,
                         file_type_t file_type,
                         s32 path_len = -1);

        file_t* add_file(session_t& s,
                         const path_t& path,
                         format_type_t bin_type,
                         file_type_t file_type);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const s8* path,
                         machine::type_t machine,
                         format_type_t bin_type,
                         file_type_t output_type,
                         s32 path_len = -1);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const path_t& path,
                         machine::type_t machine,
                         format_type_t bin_type,
                         file_type_t output_type);

        file_t* add_file(session_t& s,
                         const String_Concept auto& path,
                         format_type_t bin_type,
                         file_type_t file_type) {
            return add_file(s,
                            (const s8*) path.data,
                            bin_type,
                            file_type,
                            path.length);
        }

        file_t* add_file(session_t& s,
                         module_t* module,
                         const String_Concept auto& path,
                         machine::type_t machine,
                         format_type_t bin_type,
                         file_type_t output_type) {
            return add_file(s,
                            module,
                            (const s8*) path.data,
                            machine,
                            bin_type,
                            output_type,
                            path.length);
        }

        status_t init(session_t& s,
                      alloc_t* alloc = context::top()->alloc.main);
    }

    namespace system {
        u0 fini();

        u0 free_module(module_t* mod);

        module_t* get_module(module_id id);

        module_t* make_module(module_type_t type);

        module_t* make_module(module_type_t type, module_id id);

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    namespace member {
    }

    namespace import {
        u0 add_symbol(import_t* import, symbol_t* symbol);
    }

    namespace section {
        u0 free(section_t& section);

        status_t init(section_t* section,
                      section::type_t type,
                      const section_opts_t& opts);

        symbol_t* add_symbol(section_t* section,
                             u32 name_offset,
                             const symbol_opts_t& opts = {});

        u0 set_data(section_t* section, const u8* data);

        symbol_t* get_symbol(section_t* section, u32 idx);

        u32 add_string(section_t* section, str::slice_t str);

        import_t* add_import(section_t* section, symbol_t* module_symbol);
    }

    namespace module {
        u0 free(module_t& module);

        u0 find_members(const module_t& module,
                        str::slice_t name,
                        member_ptr_array_t& list);

        u0 find_sections(const module_t& module,
                         str::slice_t name,
                         section_ptr_array_t& list);

        section_t* make_section(module_t& module,
                                section::type_t type,
                                const section_opts_t& opts = {});

        section_t* make_import(module_t& module);

        section_t* make_bss(module_t& module, u32 size);

        section_t* make_default_string_table(module_t& module);

        section_t* make_default_symbol_table(module_t& module);

        section_t* get_section(const module_t& module, u32 idx);

        member_t* make_member(module_t& module, str::slice_t name);

        section_t* make_data(module_t& module, u8* data, u32 size);

        section_t* make_text(module_t& module, u8* data, u32 size);

        section_t* make_rodata(module_t& module, u8* data, u32 size);

        status_t reserve_sections(module_t& module, u32 num_sections);

        status_t init(module_t& module, module_type_t type, module_id id);
    }

    namespace symbol_table {
        u0 free(symbol_table_t& table);

        status_t init(symbol_table_t& table);

        symbol_t* make_symbol(symbol_table_t& symtab,
                              u32 name_offset,
                              const symbol_opts_t& opts);
    }

    namespace string_table {
        u0 free(string_table_t& table);

        u0 reset(string_table_t& table);

        status_t init(string_table_t& table);

        u32 append(string_table_t& table, str::slice_t str);

        const s8* get(const string_table_t& table, u32 offset);

        s32 find(const string_table_t& table, str::slice_t str);

        status_t init(string_table_t& table, u8* buf, u32 size_in_bytes);
    }
}
