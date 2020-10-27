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

#include <basecode/objfmt/types.h>

namespace basecode::objfmt {
    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc);
    }

    namespace import {
        status_t add_symbol(import_t* import, symbol_id symbol);
    }

    namespace section {
        u0 free(section_t& section);

        u0 reserve(section_t* section, u64 size);

        import_t* get_import(section_t* section, import_id id);

        u0 data(section_t* section, const u8* data, u32 length);

        result_t import_module(section_t* section, symbol_id symbol);

        status_t init(section_t& section, section::type_t type, symbol_id symbol);
    }

    namespace file {
        u0 free(file_t& file);

        status_t init(file_t& file);

        symbol_t* get_symbol(file_t& file, symbol_id id);

        section_t* get_section(file_t& file, section_id id);

        section_t* find_section(file_t& file, symbol_id symbol);

        symbol_t* find_symbol(file_t& file, const s8* name, s32 len = -1);

        symbol_t* find_symbol(file_t& file, const String_Concept auto& name) {
            return find_symbol(file, (const s8*) name.data, name.length);
        }

        result_t make_section(file_t& file, section::type_t type, symbol_id symbol);

        result_t make_symbol(file_t& file, symbol::type_t type, const s8* name, s32 len = -1);

        result_t make_symbol(file_t& file, const String_Concept auto& name, symbol::type_t type = {}) {
            return make_symbol(file, type, (const s8*) name.data, name.length);
        }
    }
}
