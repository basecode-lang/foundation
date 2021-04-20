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
    namespace system {
        u0 fini();

        u0 free_module(module_t* mod);

        module_t* get_module(module_id id);

        module_t* make_module(module_type_t type);

        status_t init(alloc_t* alloc = context::top()->alloc);

        module_t* make_module(module_type_t type, module_id id);
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
