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

#pragma once

#include <basecode/binfmt/types.h>

namespace basecode::binfmt {
    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc);
    }

    namespace import {
        status_t add_symbol(import_t* import, symbol_id symbol);
    }

    namespace section {
        u0 free(section_t& section);

        result_t reserve(module_t& module, section_id id, u64 size);

        import_t* get_import(module_t& module, section_id id, import_id import);

        result_t import_module(module_t& module, section_id id, symbol_id symbol);

        status_t init(section_t& section, section::type_t type, symbol_id symbol);

        result_t data(module_t& module, section_id id, const u8* data, u32 length);
    }

    namespace module {
        u0 free(module_t& module);

        status_t init(module_t& module);

        symbol_t* get_symbol(const module_t& module, symbol_id id);

        section_t* get_section(const module_t& module, section_id id);

        symbol_t* find_symbol(const module_t& module, const s8* name, s32 len = -1);

        u0 find_sections(const module_t& module, symbol_id symbol, section_ptr_list_t& list);

        const symbol_t* find_symbol(const module_t& module, const String_Concept auto& name) {
            return find_symbol(module, (const s8*) name.data, name.length);
        }

        result_t make_section(module_t& module, section::type_t type, const section_opts_t& opts = {});

        result_t make_symbol(module_t& module, const symbol_opts_t& opts, const s8* name, s32 len = -1);

        result_t make_symbol(module_t& module, const String_Concept auto& name, const symbol_opts_t& opts = {}) {
            return make_symbol(module, opts, (const s8*) name.data, name.length);
        }
    }
}
