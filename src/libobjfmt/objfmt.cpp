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
        status_t add_symbol(import_t* import, const symbol_t* symbol) {
            for (auto imported : import->symbols) {
                if (imported == symbol)
                    return status_t::duplicate_import;
            }
            array::append(import->symbols, const_cast<symbol_t*>(symbol));
            return status_t::ok;
        }
    }

    namespace section {
        u0 free(section_t* section) {
            switch (section->type) {
                case section_type_t::import:
                    for (auto& import : section->subclass.imports)
                        array::free(import.symbols);
                    array::free(section->subclass.imports);
                    break;
                default:
                    break;
            }
            array::free(section->symbols);
        }

        u0 reserve(section_t* section, u64 size) {
            section->subclass.size = size;
        }

        u0 data(section_t* section, const u8* data, u32 length) {
            section->subclass.data = slice::make(data, length);
        }

        status_t init(section_t* section, section_type_t type, const symbol_t* symbol) {
            section->alloc = g_objfmt_sys.alloc;
            array::init(section->symbols, section->alloc);
            section->type             = type;
            section->flags            = {};
            section->symbol           = symbol;
            section->address.physical = {};
            section->address.virtual_ = {};
            switch (section->type) {
                case section_type_t::uninit:
                    section->subclass.size = {};
                    break;
                case section_type_t::import:
                    array::init(section->subclass.imports, section->alloc);
                    break;
                default:
                    section->subclass.data = {};
                    break;
            }
            return status_t::ok;
        }

        status_t import_module(section_t* section, import_t** result, const symbol_t* module) {
            if (section->type != section_type_t::import)
                return status_t::invalid_section_type;
            auto import = &array::append(section->subclass.imports);
            import->module  = module;
            import->section = section;
            import->flags   = {};
            array::init(import->symbols);
            *result = import;
            return *result ? status_t::ok : status_t::import_failure;
        }
    }

    namespace file {
        u0 free(file_t& file) {
            path::free(file.path);
            for (auto section : file.sections)
                section::free(section);
            array::free(file.sections);
            hashtab::free(file.symbols);
            memory::system::free(file.symbol_slab);
            memory::system::free(file.section_slab);
        }

        status_t init(file_t& file) {
            file.alloc = g_objfmt_sys.alloc;
            path::init(file.path, file.alloc);
            array::init(file.sections, file.alloc);
            hashtab::init(file.symbols, file.alloc);
            {
                slab_config_t slab_config{};
                slab_config.backing   = file.alloc;
                slab_config.buf_size  = sizeof(symbol_t);
                slab_config.buf_align = alignof(symbol_t);
                slab_config.num_pages = 1;
                file.symbol_slab = memory::system::make(alloc_type_t::slab, &slab_config);
            }
            {
                slab_config_t slab_config{};
                slab_config.backing   = file.alloc;
                slab_config.buf_size  = sizeof(section_t);
                slab_config.buf_align = alignof(section_t);
                slab_config.num_pages = 1;
                file.section_slab = memory::system::make(alloc_type_t::slab, &slab_config);
            }
            return status_t::ok;
        }

        status_t find_symbol(file_t& file, symbol_t** result, const s8* name, s32 len) {
            const auto key = slice::make(name, len == -1 ? strlen(name) : len);
            *result = hashtab::find(file.symbols, key);
            return *result ? status_t::ok : status_t::symbol_not_found;
        }

        status_t make_symbol(file_t& file, symbol_t** result, const s8* name, s32 len) {
            if (OK(find_symbol(file, result, name, len)))
                return status_t::duplicate_symbol;
            const auto key = string::interned::fold(name, len);
            auto symbol = (symbol_t*) memory::alloc(file.symbol_slab);
            symbol->name    = key;
            symbol->section = {};
            symbol->type    = {};
            hashtab::insert(file.symbols, key, symbol);
            *result = symbol;
            return status_t::ok;
        }

        status_t find_section(file_t& file, section_t** result, const symbol_t* symbol) {
            *result = {};
            for (auto section : file.sections) {
                if (section->symbol == symbol) {
                    *result = section;
                    return status_t::ok;
                }
            }
            return status_t::symbol_not_found;
        }

        status_t make_section(file_t& file, section_t** result, section_type_t type, const symbol_t* symbol) {
            *result = {};
            if (OK(find_section(file, result, symbol)))
                return status_t::ok;
            auto section = (section_t*) memory::alloc(file.section_slab);
            section->file = &file;
            auto status = section::init(section, type, symbol);
            if (!OK(status))
                return status;
            array::append(file.sections, section);
            *result = section;
            return status_t::ok;
        }
    }
}
