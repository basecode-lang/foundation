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

#include <basecode/core/error.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stable_array.h>

namespace basecode::binfmt {
    using module_table_t        = hashtab_t<module_id, module_t*>;
    using module_array_t        = stable_array_t<module_t>;
    using symbol_array_t        = stable_array_t<symbol_t>;
    using section_array_t       = stable_array_t<section_t>;

    struct system_t final {
        alloc_t*                alloc;
        module_table_t          modtab;
        module_array_t          modules;
        symbol_array_t          symbols;
        section_array_t         sections;
        io_system_t*            systems[max_type_count];
        module_id               id;
    };

    system_t                    g_binfmt_sys{};

    // N.B. forward declare back-end accessor functions
    namespace pe {
        io_system_t* system();
    }

    namespace elf {
        io_system_t* system();
    }

    namespace coff {
        io_system_t* system();
    }

    namespace macho {
        io_system_t* system();
    }

    namespace archive {
        io_system_t* system();
    }

    namespace io {
    }

    namespace name_map {
        u0 free(name_array_t& names) {
            array::free(names);
        }

        u0 add(name_array_t& names,
               section::type_t type,
               name_flags_t flags,
               str::slice_t name) {
            auto entry = &array::append(names);
            entry->type      = type;
            entry->flags     = flags;
            entry->flags.pad = {};
            entry->name      = string::interned::fold(name);
        }

        u0 init(name_array_t& names, alloc_t* alloc) {
            array::init(names, alloc);
        }

        const name_map_t* find(const name_array_t& names,
                               section::type_t type,
                               name_flags_t flags) {
            for (const auto& entry : names) {
                if (entry.type == type
                    &&  std::memcmp(&entry.flags,
                                    &flags,
                                    sizeof(name_flags_t)) == 0) {
                    return &entry;
                }
            }
            return nullptr;
        }
    }

    // XXX: if files are smaller than some currently unknown size,
    //      we should use the buf_t in alloc mode and call save.  if greater
    //      than this size, we should map_new/unmap
    //
    namespace file {
        u0 free(file_t& file) {
            buf::cursor::free(file.crsr);
            buf::free(file.buf);
            path::free(file.path);
        }

        status_t save(file_t& file) {
            auto status = buf::save(file.buf, file.path);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        status_t map_existing(file_t& file) {
            if (!OK(buf::map_existing(file.buf, file.path)))
                return status_t::read_error;
            return status_t::ok;
        }

        status_t init(file_t& file, alloc_t* alloc) {
            path::init(file.path, alloc);
            buf::init(file.buf, alloc);
            buf::cursor::init(file.crsr, file.buf);
            return status_t::ok;
        }

        status_t unmap(file_t& file, b8 sync_flush) {
            auto status = buf::unmap(file.buf, sync_flush);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        status_t map_new(file_t& file, usize file_size) {
            if (!OK(buf::map_new(file.buf, file.path, file_size)))
                return status_t::read_error;
            return status_t::ok;
        }
    }

    namespace session {
        u0 free(session_t& s) {
            for (auto& file : s.files)
                file::free(file);
            array::free(s.files);
        }

        status_t read(session_t& s) {
            for (auto& file : s.files) {
                const auto idx = u32(file.bin_type);
                if (idx > max_type_count - 1)
                    return status_t::invalid_container_type;
                auto status = g_binfmt_sys.systems[idx]->read(file);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        status_t write(session_t& s) {
            for (auto& file : s.files) {
                const auto idx = u32(file.bin_type);
                if (idx > max_type_count - 1)
                    return status_t::invalid_container_type;
                auto status = g_binfmt_sys.systems[idx]->write(file);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        file_t* add_file(session_t& s,
                         module_t* module,
                         const path_t& path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type) {
            return add_file(s,
                            module,
                            path::c_str(path),
                            machine,
                            bin_type,
                            output_type,
                            path.str.length);
        }

        file_t* add_file(session_t& s,
                         const path_t& path,
                         type_t bin_type,
                         file_type_t file_type) {
            return add_file(s,
                            path::c_str(path),
                            bin_type,
                            file_type,
                            path.str.length);
        }

        file_t* add_file(session_t& s,
                         const s8* path,
                         type_t bin_type,
                         file_type_t file_type,
                         s32 path_len) {
            auto file = &array::append(s.files);
            file::init(*file, s.alloc);
            path::set(file->path, path, path_len);
            file->session    = &s;
            file->module     = {};
            file->machine    = {};
            file->bin_type   = bin_type;
            file->file_type  = file_type;
            return file;
        }

        file_t* add_file(session_t& s,
                         module_t* module,
                         const s8* path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type,
                         s32 path_len) {
            auto file = &array::append(s.files);
            file::init(*file, s.alloc);
            path::set(file->path, path, path_len);
            file->session    = &s;
            file->module     = module;
            file->machine    = machine;
            file->bin_type   = bin_type;
            file->file_type  = output_type;
            return file;
        }

        status_t init(session_t& s, alloc_t* alloc) {
            s.alloc = alloc;
            array::init(s.files, s.alloc);
            return status_t::ok;
        }
    }

    namespace system {
        u0 fini() {
            for (auto sys : g_binfmt_sys.systems)
                sys->fini();
            for (auto section : g_binfmt_sys.sections)
                section::free(*section);
            for (auto mod : g_binfmt_sys.modules)
                module::free(*mod);
            stable_array::free(g_binfmt_sys.modules);
            stable_array::free(g_binfmt_sys.symbols);
            stable_array::free(g_binfmt_sys.sections);
            hashtab::free(g_binfmt_sys.modtab);
        }

        status_t init(alloc_t* alloc) {
            g_binfmt_sys.alloc = alloc;
            hashtab::init(g_binfmt_sys.modtab, g_binfmt_sys.alloc);
            stable_array::init(g_binfmt_sys.modules, g_binfmt_sys.alloc);
            stable_array::init(g_binfmt_sys.symbols, g_binfmt_sys.alloc);
            stable_array::init(g_binfmt_sys.sections, g_binfmt_sys.alloc);
            g_binfmt_sys.systems[u32(type_t::ar)]    = archive::system();
            g_binfmt_sys.systems[u32(type_t::pe)]    = pe::system();
            g_binfmt_sys.systems[u32(type_t::elf)]   = elf::system();
            g_binfmt_sys.systems[u32(type_t::coff)]  = coff::system();
            g_binfmt_sys.systems[u32(type_t::macho)] = macho::system();
            for (auto sys : g_binfmt_sys.systems) {
                auto status = sys->init(alloc);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        u0 free_module(module_t* mod) {
            hashtab::remove(g_binfmt_sys.modtab, mod->id);
            module::free(*mod);
            stable_array::erase(g_binfmt_sys.modules, mod);
        }

        module_t* get_module(module_id id) {
            return hashtab::find(g_binfmt_sys.modtab, id);
        }

        module_t* make_module(module_type_t type) {
            return make_module(type, ++g_binfmt_sys.id);
        }

        module_t* make_module(module_type_t type, module_id id) {
            auto new_mod = &stable_array::append(g_binfmt_sys.modules);
            auto status = module::init(*new_mod, type, id);
            if (!OK(status)) {
                error::report::add(status, error_report_level_t::error);
                return nullptr;
            }
            hashtab::insert(g_binfmt_sys.modtab, id, new_mod);
            return new_mod;
        }
    }

    namespace import {
        u0 add_symbol(import_t* import, symbol_t* symbol) {
            auto idx = array::contains(import->symbols, symbol);
            if (idx != -1) {
                error::report::add(status_t::duplicate_import,
                                   error_report_level_t::error);
                return;
            }
            array::append(import->symbols, symbol);
        }
    }

    namespace section {
        u0 free(section_t& section) {
            switch (section.type) {
                case section::type_t::reloc:
                    array::free(section.subclass.relocs);
                    break;
                case section::type_t::group:
                    array::free(section.subclass.group.sections);
                    break;
                case section::type_t::import:
                    for (auto& import : section.subclass.imports)
                        array::free(import.symbols);
                    array::free(section.subclass.imports);
                    break;
                case section::type_t::data:
                    if (!section.flags.strings)
                        break;
                case section::type_t::strtab:
                    string_table::free(section.subclass.strtab);
                    break;
                case section::type_t::symtab:
                    symbol_table::free(section.subclass.symtab);
                    break;
                default:
                    break;
            }
        }

        status_t init(section_t* section,
                      section::type_t type,
                      const section_opts_t& opts) {
            section->alloc       = g_binfmt_sys.alloc;
            section->type        = type;
            section->info        = opts.info;
            section->link        = opts.link;
            section->size        = opts.size;
            section->flags       = opts.flags;
            section->align       = opts.align;
            section->ext_type    = opts.ext_type;
            section->name_offset = opts.name_offset;
            switch (section->type) {
                case section::type_t::data:
                    if (section->flags.strings) {
                        goto init_strtab;
                    } else {
                        section->subclass.data = {};
                    }
                    break;
                case section::type_t::text:
                case section::type_t::custom:
                    section->subclass.data = {};
                    break;
                case section::type_t::reloc:
                    array::init(section->subclass.relocs, section->alloc);
                    break;
                case section::type_t::group:
                    array::init(section->subclass.group.sections,
                                section->alloc);
                    break;
                case section::type_t::import:
                    array::init(section->subclass.imports, section->alloc);
                    break;
                case section::type_t::strtab:
                init_strtab: if (!opts.strtab.buf) {
                        auto status = string_table::init(section->subclass.strtab);
                        if (!OK(status))
                            return status;
                    } else {
                        auto status = string_table::init(section->subclass.strtab,
                                                         opts.strtab.buf,
                                                         opts.strtab.size_in_bytes);
                        if (!OK(status))
                            return status;
                    }
                    break;
                case section::type_t::symtab:
                    symbol_table::init(section->subclass.symtab);
                    break;
                default:
                    break;
            }
            return status_t::ok;
        }

        symbol_t* add_symbol(section_t* section,
                             u32 name_offset,
                             const symbol_opts_t& opts) {
            if (!section || section->type != section::type_t::symtab) {
                error::report::add(status_t::invalid_section_type,
                                   error_report_level_t::error);
                return nullptr;
            }
            return symbol_table::make_symbol(section->subclass.symtab,
                                             name_offset,
                                             opts);
        }

        u0 set_data(section_t* section, const u8* data) {
            section->subclass.data = data;
        }

        symbol_t* get_symbol(section_t* section, u32 idx) {
            auto& ssc = section->subclass.symtab;
            return ssc[idx];
        }

        u32 add_string(section_t* section, str::slice_t str) {
            if (!section || section->type != section::type_t::strtab) {
                error::report::add(status_t::invalid_section_type,
                                   error_report_level_t::error);
                return 0;
            }
            return string_table::append(section->subclass.strtab, str);
        }

        import_t* add_import(section_t* section, symbol_t* module_symbol) {
            if (!section || section->type != section::type_t::import) {
                error::report::add(status_t::invalid_section_type,
                                   error_report_level_t::error);
                return nullptr;
            }
            auto import = &array::append(section->subclass.imports);
            import->module_symbol = module_symbol;
            import->section       = section;
            import->flags         = {};
            array::init(import->symbols);
            return import;
        }
    }

    namespace module {
        u0 free(module_t& module) {
            switch (module.type) {
                case module_type_t::archive: {
                    auto& sc = module.subclass.archive;
                    array::free(sc.members);
                    array::free(sc.offsets);
                    break;
                }
                case module_type_t::object: {
                    auto& sc = module.subclass.object;
                    array::free(sc.sections);
                    break;
                }
            }
        }

        section_t* make_import(module_t& module) {
            section_opts_t opts{};
            opts.flags = {
                .write = true,
                .alloc = true,
            };
            return make_section(module, section::type_t::import, opts);
        }

        u0 find_sections(const module_t& module,
                         str::slice_t name,
                         section_ptr_array_t& list) {
            if (module.type != module_type_t::object)
                return;
            auto& sc = module.subclass.object;
            if (!sc.strtab)
                return;
            auto name_offset = string_table::find(sc.strtab->subclass.strtab,
                                                  name);
            if (name_offset == -1)
                return;
            array::reset(list);
            for (const auto section : sc.sections) {
                if (name_offset == section->name_offset)
                    array::append(list, section);
            }
        }

        section_t* make_section(module_t& module,
                                section::type_t type,
                                const section_opts_t& opts) {
            if (module.type != module_type_t::object) {
                error::report::add(status_t::invalid_section_type,
                                   error_report_level_t::error);
                return nullptr;
            }
            if (type == section::type_t::custom && opts.name_offset == 0) {
                error::report::add(status_t::spec_section_custom_name,
                                   error_report_level_t::error);
                return nullptr;
            }
            auto& sc = module.subclass.object;
            auto section = &stable_array::append(g_binfmt_sys.sections);
            section->module = &module;
            auto status = section::init(section, type, opts);
            if (!OK(status)) {
                error::report::add(status, error_report_level_t::error);
                return nullptr;
            }
            array::append(sc.sections, section);
            section->number = sc.sections.size;
            return section;
        }

        section_t* make_bss(module_t& module, u32 size) {
            section_opts_t opts{};
            opts.flags = {
                .write  = true,
                .alloc  = true,
            };
            opts.size = size;
            return make_section(module, section::type_t::bss, opts);
        }

        section_t* get_section(const module_t& module, u32 idx) {
            auto msc = &module.subclass.object;
            return idx < msc->sections.size ? msc->sections[idx] : nullptr;
        }

        section_t* make_data(module_t& module, u8* data, u32 size) {
            section_opts_t opts{};
            opts.flags = {
                .write  = true,
                .alloc  = true,
            };
            opts.size = size;
            auto section = make_section(module, section::type_t::data, opts);
            if (!section) {
                // XXX: need an error condition here
                return nullptr;
            }
            section::set_data(section, data);
            return section;
        }

        section_t* make_text(module_t& module, u8* data, u32 size) {
            section_opts_t opts{};
            opts.flags = {
                .exec   = true,
                .alloc  = true,
            };
            opts.size = size;
            auto section = make_section(module, section::type_t::text, opts);
            if (!section) {
                // XXX: need an error condition here
                return nullptr;
            }
            section::set_data(section, data);
            return section;
        }

        section_t* make_default_string_table(module_t& module) {
            auto sc = &module.subclass.object;
            section_opts_t opts{};
            auto section = make_section(module, section::type_t::strtab, opts);
            sc->strtab = section;
            return section;
        }

        section_t* make_default_symbol_table(module_t& module) {
            auto sc = &module.subclass.object;
            section_opts_t opts{};
            if (!sc->strtab) {
                auto strtab_section = make_default_string_table(module);
                if (!strtab_section) {
                    // XXX: need an error condition here
                    return nullptr;
                }
                sc->strtab = strtab_section;
            }
            opts.link = sc->strtab;
            auto section = make_section(module, section::type_t::symtab, opts);
            sc->symtab = section;
            return section;
        }

        section_t* make_rodata(module_t& module, u8* data, u32 size) {
            section_opts_t opts{};
            opts.flags = {
                .write  = false,
                .alloc  = true,
            };
            opts.size = size;
            auto section = make_section(module, section::type_t::data, opts);
            if (!section) {
                // XXX: need an error condition here
                return nullptr;
            }
            section::set_data(section, data);
            return section;
        }

        status_t reserve_sections(module_t& module, u32 num_sections) {
            auto msc = &module.subclass.object;
            array::resize(msc->sections, num_sections);
            for (u32 i = 0; i < num_sections; ++i) {
                auto section = &stable_array::append(g_binfmt_sys.sections);
                std::memset(section, 0, sizeof(section_t));
                section->module = &module;
                section->number = i + 1;
                msc->sections[i] = section;
            }
            return status_t::ok;
        }

        status_t init(module_t& module, module_type_t type, module_id id) {
            module.alloc = g_binfmt_sys.alloc;
            module.id    = id;
            module.type  = type;
            switch (module.type) {
                case module_type_t::archive: {
                    auto& sc = module.subclass.archive;
                    array::init(sc.members, module.alloc);
                    array::init(sc.offsets, module.alloc);
                    break;
                }
                case module_type_t::object:
                    auto& sc = module.subclass.object;
                    array::init(sc.sections, module.alloc);
                    break;
            }
            return status_t::ok;
        }
    }

    namespace symbol_table {
        u0 free(symbol_table_t& table) {
            hashtab::free(table.index);
            array::free(table.symbols);
        }

        status_t init(symbol_table_t& table) {
            table.alloc = g_binfmt_sys.alloc;
            array::init(table.symbols, table.alloc);
            hashtab::init(table.index, table.alloc);
            return status_t::ok;
        }

        symbol_t* make_symbol(symbol_table_t& symtab,
                              u32 name_offset,
                              const symbol_opts_t& opts) {
            auto next_symbol = &stable_array::append(g_binfmt_sys.symbols);
            next_symbol->ndx = symtab.symbols.size;
            array::append(symtab.symbols, next_symbol);
            next_symbol->next        = {};
            next_symbol->type        = opts.type;
            next_symbol->size        = opts.size;
            next_symbol->value       = opts.value;
            next_symbol->scope       = opts.scope;
            next_symbol->section     = opts.section;
            next_symbol->visibility  = opts.visibility;
            next_symbol->name_offset = name_offset;
            auto prev_symbol = hashtab::find(symtab.index, name_offset);
            if (prev_symbol) {
                while (true) {
                    if (!prev_symbol->next)
                        break;
                    prev_symbol = prev_symbol->next;
                }
                prev_symbol->next = next_symbol;
            } else {
                hashtab::insert(symtab.index, name_offset, next_symbol);
            }
            return next_symbol;
        }
    }

    namespace string_table {
        static u32 append_offs(string_table_t& table);

        static u0 reserve_buf(string_table_t& table, u32 new_capacity);

        static u0 reserve_offs(string_table_t& table, u32 new_capacity);

        u0 free(string_table_t& table) {
            memory::free(table.alloc, table.offs.data);
            if (table.mode == string_table_mode_t::exclusive)
                memory::free(table.alloc, table.buf.data);
        }

        u0 reset(string_table_t& table) {
            table.buf  = {};
            table.offs = {};
        }

        status_t init(string_table_t& table) {
            table.buf  = {};
            table.offs = {};
            table.mode  = string_table_mode_t::exclusive;
            table.alloc = g_binfmt_sys.alloc;
            return status_t::ok;
        }

        static u32 append_offs(string_table_t& table) {
            if (table.offs.size + 1 > table.offs.capacity)
                reserve_offs(table, table.offs.capacity * 2 + 8);
            auto& offset = table.offs.data[table.offs.size++];
            offset = table.buf.size == 0 ? 1 : table.buf.size;
            return offset;
        }

        u32 append(string_table_t& table, str::slice_t str) {
            const auto offset = append_offs(table);
            const auto len = str.length + 1;
            if (table.mode == string_table_mode_t::exclusive
                && (table.buf.size + len > table.buf.capacity)) {
                reserve_buf(table, table.buf.capacity * 2 + 8);
            }
            auto data = table.buf.data + offset;
            if (str.length > 0)
                std::memcpy(data, str.data, str.length);
            data[str.length] = '\0';
            table.buf.size += str.length + 1;
            return offset;
        }

        const s8* get(const string_table_t& table, u32 offset) {
            return (const s8*) (table.buf.data + offset);
        }

        s32 find(const string_table_t& table, str::slice_t str) {
#if defined(__AVX2__)
            const __m256i first = _mm256_set1_epi8(str[0]);
            const __m256i last  = _mm256_set1_epi8(str[str.length - 1]);

            for (u64 i = 0; i < table.buf.size; i += 32) {
                const __m256i hay_first = _mm256_loadu_si256(
                    (const __m256i*) (table.buf.data + i));
                const __m256i hay_last  = _mm256_loadu_si256(
                    (const __m256i*) (table.buf.data + i + (str.length - 1)));
                u32 match_mask = _mm256_movemask_epi8(
                    _mm256_and_si256(_mm256_cmpeq_epi8(first, hay_first),
                                     _mm256_cmpeq_epi8(last, hay_last)));
                while (match_mask) {
                    const auto bit_pos = __builtin_ctz(match_mask);
                    if (std::memcmp(table.buf.data + i + bit_pos + 1,
                                    str.data + 1,
                                    str.length - 2) == 0) {
                        return i + bit_pos;
                    }
                    match_mask &= (match_mask - 1);
                }
            }

            return -1;
#elif defined(__SSE4_2__)
            static_assert(false, "implement SSE 4.2 version!");
#else
            static_assert(false, "implement non-SIMD version!");
#endif
        }

        static u0 reserve_buf(string_table_t& table, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(table.alloc, table.buf.data);
                table.buf = {};
                return;
            }
            if (new_capacity == table.buf.capacity)
                return;
            new_capacity = std::max(table.buf.size, new_capacity);
            table.buf.data = (u8*) memory::realloc(table.alloc,
                                                   table.buf.data,
                                                   new_capacity);
            table.buf.capacity = new_capacity;
        }

        static u0 reserve_offs(string_table_t& table, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(table.alloc, table.offs.data);
                table.offs = {};
                return;
            }
            if (new_capacity == table.offs.capacity)
                return;
            new_capacity = std::max(table.offs.size, new_capacity);
            table.offs.data = (u32*) memory::realloc(table.alloc,
                                                     table.offs.data,
                                                     new_capacity * sizeof(u32),
                                                     alignof(u32));
            const auto data = table.offs.data + table.offs.size;
            const auto size_to_clear = new_capacity > table.offs.capacity ?
                                       new_capacity - table.offs.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(u32));
            table.offs.capacity = new_capacity;
        }

        status_t init(string_table_t& table, u8* buf, u32 size_in_bytes) {
            table.alloc        = g_binfmt_sys.alloc;
            table.mode         = string_table_mode_t::shared;
            table.buf.data     = buf;
            table.buf.size     = {};
            table.buf.capacity = size_in_bytes;
            for (u32 i = 0; i < table.buf.capacity; ++i) {
                if (table.buf.data[i] == 0)
                    append_offs(table);
                ++table.buf.size;
            }
            return status_t::ok;
        }
    }
}
