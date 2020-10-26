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
#include <basecode/objfmt/types.h>
#include <basecode/core/filesys.h>

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
            container::init(g_objfmt_sys.alloc);
            auto objfmt_config_path = "../etc/objfmt.fe"_path;
            path_t config_path{};
            filesys::bin_rel_path(config_path, objfmt_config_path);
            defer({
                path::free(config_path);
                path::free(objfmt_config_path);
            });
            fe_Object* result{};
            auto status = config::eval(config_path, &result);
            if (!OK(status)) {
                return status_t::config_eval_error;
            }
            return status_t::ok;
        }
    }

    namespace section {
        u0 free(section_t& section) {
            hashtab::free(section.symbols);
            switch (section.type) {
                case section_type_t::import:
                    hashtab::free(section.subclass.import.table);
                    break;
                default:
                    break;
            }
        }

        u0 reserve(section_t& section, u64 size) {
            section.subclass.uninit.size = size;
        }

        u0 data(section_t& section, const u8* data, u32 length) {
            section.subclass.data.range = slice::make(data, length);
        }

        status_t import(section_t& section, import_t** result, const s8* name, s32 len) {
            if (section.type != section_type_t::import)
                return status_t::invalid_section_type;
            const auto import_name = string::interned::fold(name, len);
            *result = hashtab::emplace(section.subclass.import.table, import_name);
            (*result)->flags = {};
            return *result ? status_t::ok : status_t::import_failure;
        }

        status_t init(section_t& section, section_type_t type, const s8* name, s32 len) {
            section.alloc = g_objfmt_sys.alloc;
            hashtab::init(section.symbols, section.alloc);
            section.type             = type;
            section.name             = string::interned::fold(name, len);
            section.address.physical = {};
            section.address.virtual_ = {};
            section.flags            = {};
            switch (section.type) {
                case section_type_t::uninit:
                    section.subclass.uninit.size = {};
                    break;
                case section_type_t::import:
                    hashtab::init(section.subclass.import.table, section.alloc);
                    break;
                default:
                    section.subclass.data.range = {};
                    break;
            }
            return status_t::ok;
        }
    }

    namespace obj_file {
        u0 free(obj_file_t& file) {
            path::free(file.path);
            for (auto& section : file.sections)
                section::free(section);
            array::free(file.sections);
            stable_array::free(file.symbols);
        }

        status_t init(obj_file_t& file) {
            file.alloc = g_objfmt_sys.alloc;
            path::init(file.path, file.alloc);
            array::init(file.sections, file.alloc);
            stable_array::init(file.symbols, file.alloc);
            return status_t::ok;
        }
    }
}
