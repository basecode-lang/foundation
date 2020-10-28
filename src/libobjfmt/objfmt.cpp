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

#include <basecode/core/error.h>
#include <basecode/core/string.h>
#include <basecode/core/config.h>
#include <basecode/core/filesys.h>
#include <basecode/objfmt/objfmt.h>
#include <basecode/objfmt/container.h>

namespace basecode::objfmt {
    struct system_t final {
        alloc_t*                alloc;
    };

    system_t                    g_objfmt_sys;

    namespace system {
        u0 fini() {
            container::fini();
        }

        status_t init(alloc_t* alloc) {
            g_objfmt_sys.alloc = alloc;
            auto   file_path = "../etc/objfmt.fe"_path;
            path_t config_path{};
            filesys::bin_rel_path(config_path, file_path);
            defer({
                path::free(config_path);
                path::free(file_path);
            });
            {
                auto status = container::init(g_objfmt_sys.alloc);
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
                case section::type_t::import:
                    for (auto& import : section.subclass.imports)
                        array::free(import.symbols);
                    array::free(section.subclass.imports);
                    break;
                default:
                    break;
            }
            array::free(section.symbols);
        }

        u0 reserve(section_t* section, u64 size) {
            section->subclass.size = size;
        }

        import_t* get_import(section_t* section, import_id id) {
            if (section->type != section::type_t::import)
                return nullptr;
            return &section->subclass.imports[id - 1];
        }

        u0 data(section_t* section, const u8* data, u32 length) {
            section->subclass.data = slice::make(data, length);
        }

        result_t import_module(section_t* section, symbol_id module) {
            if (section->type != section::type_t::import)
                return {0, status_t::invalid_section_type};
            auto import = &array::append(section->subclass.imports);
            import->id      = section->subclass.imports.size;
            import->module  = module;
            import->section = section->id;
            import->flags   = {};
            array::init(import->symbols);
            return {import->id, status_t::ok};
        }

        status_t init(section_t* section, section::type_t type, symbol_id symbol) {
            section->alloc = g_objfmt_sys.alloc;
            array::init(section->symbols, section->alloc);
            section->type             = type;
            section->flags            = {};
            section->symbol           = symbol;
            switch (section->type) {
                case section::type_t::uninit:
                    section->subclass.size = {};
                    break;
                case section::type_t::import:
                    array::init(section->subclass.imports, section->alloc);
                    break;
                default:
                    section->subclass.data = {};
                    break;
            }
            return status_t::ok;
        }
    }

    namespace file {
        u0 free(file_t& file) {
            path::free(file.path);
            for (auto section : file.sections)
                section::free(section);
            array::free(file.sections);
            hashtab::free(file.symbols);
        }

        status_t init(file_t& file) {
            file.alloc = g_objfmt_sys.alloc;
            path::init(file.path, file.alloc);
            array::init(file.sections, file.alloc);
            hashtab::init(file.symbols, file.alloc);
            return status_t::ok;
        }

        symbol_t* get_symbol(const file_t& file, symbol_id id) {
            return hashtab::find(const_cast<symbol_table_t&>(file.symbols), id);
        }

        section_t* get_section(const file_t& file, section_id id) {
            return (section_t*) &file.sections[id - 1];
        }

        symbol_t* find_symbol(const file_t& file, const s8* name, s32 len) {
            const auto rc = string::interned::fold_for_result(name, len);
            return hashtab::find(const_cast<symbol_table_t&>(file.symbols), rc.id);
        }

        result_t make_section(file_t& file, section::type_t type, symbol_id symbol) {
            if (type != section::type_t::custom && symbol != 0)
                return {0, status_t::spec_section_custom_name};
            auto section = &array::append(file.sections);
            section->id   = file.sections.size;
            section->file = &file;
            auto status = section::init(section, type, symbol);
            if (!OK(status))
                return {0, status};
            return {section->id, status_t::ok};
        }

        result_t make_symbol(file_t& file, symbol::type_t type, const s8* name, s32 len) {
            {
                auto symbol = find_symbol(file, name, len);
                if (symbol)
                    return {0, status_t::duplicate_symbol};
            }
            const auto rc     = string::interned::fold_for_result(name, len);
            auto       symbol = hashtab::emplace(file.symbols, rc.id);
            symbol->name    = rc.id;
            symbol->section = {};
            symbol->type    = type;
            return {symbol->name, status_t::ok};
        }

        u0 find_sections(const file_t& file, symbol_id symbol, section_ptr_list_t& list) {
            array::reset(list);
            for (auto& section : file.sections) {
                if (section.symbol == symbol)
                    array::append(list, (section_t*) &section);
            }
        }
    }
}
