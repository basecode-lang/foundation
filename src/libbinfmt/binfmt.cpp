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

#include <basecode/binfmt/io.h>
#include <basecode/core/string.h>
#include <basecode/core/config.h>
#include <basecode/core/filesys.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stable_array.h>

namespace basecode::binfmt {
    using module_map_t          = hashtab_t<module_id, module_t*>;
    using module_list_t         = stable_array_t<module_t>;

    struct system_t final {
        alloc_t*                alloc;
        module_list_t           modules;
        module_map_t            module_map;
        module_id               id;
    };

    system_t                    g_binfmt_sys;

    namespace system {
        u0 fini() {
            for (auto mod : g_binfmt_sys.modules)
                module::free(*mod);
            stable_array::free(g_binfmt_sys.modules);
            hashtab::free(g_binfmt_sys.module_map);
            io::fini();
        }

        status_t init(alloc_t* alloc) {
            g_binfmt_sys.alloc = alloc;
            hashtab::init(g_binfmt_sys.module_map, g_binfmt_sys.alloc);
            stable_array::init(g_binfmt_sys.modules, g_binfmt_sys.alloc);
            auto   file_path = "../etc/binfmt.fe"_path;
            path_t config_path{};
            filesys::bin_rel_path(config_path, file_path);
            defer({
                path::free(config_path);
                path::free(file_path);
            });
            {
                auto status = io::init(g_binfmt_sys.alloc);
                if (!OK(status))
                    return status;
            }
            fe_Object* result{};
            {
                auto status = config::eval(config_path, &result);
                if (!OK(status))
                    return status_t::config_eval_error;
            }
            return status_t::ok;
        }

        u0 free_module(module_t* mod) {
            hashtab::remove(g_binfmt_sys.module_map, mod->id);
            module::free(*mod);
            stable_array::erase(g_binfmt_sys.modules, mod);
        }

        module_t* get_module(module_id id) {
            return hashtab::find(g_binfmt_sys.module_map, id);
        }

        status_t make_module(module_type_t type, module_t** mod) {
            return make_module(type, ++g_binfmt_sys.id, mod);
        }

        status_t make_module(module_type_t type, module_id id, module_t** mod) {
            auto new_mod = &stable_array::append(g_binfmt_sys.modules);
            auto status = module::init(*new_mod, type, id);
            if (!OK(status))
                return status;
            hashtab::insert(g_binfmt_sys.module_map, id, new_mod);
            *mod = new_mod;
            return status_t::ok;
        }
    }

    namespace import {
        status_t add_symbol(import_t* import, symbol_id symbol) {
            auto idx = array::contains(import->symbols, symbol);
            if (idx != -1)
                return status_t::duplicate_import;
            array::append(import->symbols, symbol);
            return status_t::ok;
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
                case section::type_t::code:
                case section::type_t::custom:
                    section->subclass.data = {};
                    break;
                case section::type_t::reloc:
                    array::init(section->subclass.relocs, section->alloc);
                    break;
                case section::type_t::group:
                    array::init(section->subclass.group.sections, section->alloc);
                    break;
                case section::type_t::import:
                    array::init(section->subclass.imports, section->alloc);
                    break;
                default:
                    break;
            }
            return status_t::ok;
        }

        result_t data(module_t& module, section_id id, const u8* data) {
            auto section = module::get_section(module, id);
            if (!section)
                return {0, status_t::section_not_found};
            section->subclass.data = data;
            return {section->id, status_t::ok};
        }

        import_t* get_import(module_t& module, section_id id, import_id import) {
            auto section = module::get_section(module, id);
            if (!section || section->type != section::type_t::import)
                return nullptr;
            return &section->subclass.imports[import - 1];
        }

        result_t import_module(module_t& module, section_id id, symbol_id module_symbol) {
            auto section = module::get_section(module, id);
            if (!section)
                return {0, status_t::section_not_found};
            if (section->type != section::type_t::import)
                return {0, status_t::invalid_section_type};
            auto import = &array::append(section->subclass.imports);
            import->id            = section->subclass.imports.size;
            import->module_symbol = module_symbol;
            import->section       = section->id;
            import->flags         = {};
            array::init(import->symbols);
            return {import->id, status_t::ok};
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
                    for (auto section : sc.sections)
                        section::free(section);
                    array::free(sc.sections);
                    break;
                }
            }
            array::free(module.symbols);
            hashtab::free(module.symtab);
            string_table::free(module.strtab);
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
            array::init(module.symbols, module.alloc);
            hashtab::init(module.symtab, module.alloc);
            return status_t::ok;
        }

        symbol_t* get_symbol(const module_t& module, symbol_id id) {
            return (symbol_t*) &module.symbols[id - 1];
        }

        section_t* get_section(const module_t& module, section_id id) {
            if (module.type != module_type_t::object)
                return nullptr;
            auto& sc = module.subclass.object;
            return (section_t*) &sc.sections[id - 1];
        }

        symbol_t* find_symbol(const module_t& module, const s8* name, s32 len) {
            const auto rc = string::interned::fold_for_result(name, len);
            auto id = hashtab::find(module.symtab, rc.id);
            if (!id)
                return nullptr;
            return (symbol_t*) &module.symbols[*id - 1];
        }

        u0 find_sections(const module_t& module, str::slice_t name, section_ptr_list_t& list) {
            if (module.type != module_type_t::object)
                return;
            auto& sc = module.subclass.object;
            array::reset(list);
            for (auto& section : sc.sections) {
                const auto str = string_table::get(module.strtab, section.name_offset);
                if (std::memcmp(name.data, str, name.length) == 0)
                    array::append(list, (section_t*) &section);
            }
        }

        result_t make_symbol(module_t& module, const symbol_opts_t& opts, u32 name_offset) {
            auto next_symbol = &array::append(module.symbols);
            next_symbol->id          = module.symbols.size;
            next_symbol->next        = {};
            next_symbol->type        = opts.type;
            next_symbol->size        = opts.size;
            next_symbol->value       = opts.value;
            next_symbol->scope       = opts.scope;
            next_symbol->section     = opts.section;
            next_symbol->visibility  = opts.visibility;
            next_symbol->name_offset = name_offset;
            auto prev_symbol_id = hashtab::find(module.symtab, name_offset);
            if (prev_symbol_id) {
                symbol_t* tmp_symbol{};
                symbol_id next_id = *prev_symbol_id;
                while (next_id) {
                    tmp_symbol = &module.symbols[next_id - 1];
                    next_id = tmp_symbol->next;
                }
                tmp_symbol->next = next_symbol->id;
            } else {
                hashtab::insert(module.symtab, name_offset, next_symbol->id);
            }
            return {next_symbol->id, status_t::ok};
        }

        result_t make_section(module_t& module, section::type_t type, const section_opts_t& opts) {
            if (module.type != module_type_t::object)
                return {0, status_t::invalid_section_type};
            if (type == section::type_t::custom && opts.name_offset == 0)
                return {0, status_t::spec_section_custom_name};
            auto& sc = module.subclass.object;
            auto section = &array::append(sc.sections);
            section->id     = sc.sections.size;
            section->module = &module;
            auto status = section::init(section, type, opts);
            if (!OK(status))
                return {0, status};
            return {section->id, status_t::ok};
        }

        result_t make_symbol(module_t& module, const symbol_opts_t& opts, const s8* name, s32 len) {
            const auto str         = slice::make(name, len);
            auto       name_offset = string_table::find(module.strtab, str);
            if (name_offset == -1)
                name_offset = string_table::append(module.strtab, str);
            return make_symbol(module, opts, name_offset);
        }
    }

    namespace string_table {
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
            const auto size_to_clear = new_capacity > table.offs.capacity ? new_capacity - table.offs.capacity : 0;
            std::memset(data, 0, size_to_clear * sizeof(u32));
            table.offs.capacity = new_capacity;
        }

        static u32 append_offs(string_table_t& table) {
            if (table.offs.size + 1 > table.offs.capacity)
                reserve_offs(table, table.offs.capacity * 2 + 8);
            auto& offset = table.offs.data[table.offs.size++];
            offset = table.buf.size;
            return offset;
        }

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
            table.alloc = g_binfmt_sys.alloc;
            table.mode  = string_table_mode_t::exclusive;
            return status_t::ok;
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
                const __m256i hay_first = _mm256_loadu_si256((const __m256i*) (table.buf.data + i));
                const __m256i hay_last  = _mm256_loadu_si256((const __m256i*) (table.buf.data + i + (str.length - 1)));
                u32 match_mask = _mm256_movemask_epi8(_mm256_and_si256(_mm256_cmpeq_epi8(first, hay_first),
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
