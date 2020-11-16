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

        u0 free_module(module_t* mod);

        module_t* get_module(module_id id);

        status_t init(alloc_t* alloc = context::top()->alloc);

        status_t make_module(module_type_t type, module_t** mod);

        status_t make_module(module_type_t type, module_id id, module_t** mod);
    }

    namespace import {
        status_t add_symbol(import_t* import, symbol_id symbol);
    }

    namespace section {
        u0 free(section_t& section);

        result_t set_data(module_t& module, section_id id, const u8* data);

        result_t add_string(module_t& module, section_id id, str::slice_t str);

        import_t* get_import(module_t& module, section_id id, import_id import);

        result_t import_module(module_t& module, section_id id, symbol_id symbol);

        status_t init(section_t& section, section::type_t type, symbol_id symbol);

        result_t add_symbol(module_t& module, section_id id, const symbol_opts_t& opts);
    }

    namespace module {
        u0 free(module_t& module);

        u0 find_sections(const module_t& module,
                         str::slice_t name,
                         section_ptr_list_t& list);

        result_t make_section(module_t& module,
                              section::type_t type,
                              const section_opts_t& opts = {});

        u0 set_default_strtab(module_t& module, section_id id);

        u0 set_default_symtab(module_t& module, section_id id);

        section_t* get_section(const module_t& module, section_id id);

        status_t init(module_t& module, module_type_t type, module_id id);
    }

    namespace symbol_table {
        u0 free(symbol_table_t& table);

        status_t init(symbol_table_t& table);

        symbol_t* get_symbol(const symbol_table_t& symtab, symbol_id id);

        result_t make_symbol(symbol_table_t& symtab, const symbol_opts_t& opts);
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
