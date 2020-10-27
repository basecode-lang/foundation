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
        status_t add_symbol(import_t* import, const symbol_t* symbol);
    }

    namespace section {
        u0 free(section_t* section);

        u0 reserve(section_t* section, u64 size);

        u0 data(section_t* section, const u8* data, u32 length);

        status_t init(section_t* section, section_type_t type, const symbol_t* symbol);

        status_t import_module(section_t* section, import_t** result, const symbol_t* module);
    }

    namespace file {
        u0 free(file_t& file);

        status_t init(file_t& file);

        status_t find_section(file_t& file, section_t** result, const symbol_t* symbol);

        status_t find_symbol(file_t& file, symbol_t** result, const s8* name, s32 len = -1);

        status_t make_symbol(file_t& file, symbol_t** result, const s8* name, s32 len = -1);

        status_t find_symbol(file_t& file, symbol_t** result, const String_Concept auto& name) {
            return find_symbol(file, result, (const s8*) name.data, name.length);
        }

        status_t make_symbol(file_t& file, symbol_t** result, const String_Concept auto& name) {
            return make_symbol(file, result, (const s8*) name.data, name.length);
        }

        status_t make_section(file_t& file, section_t** result, section_type_t type, const symbol_t* symbol);
    }
}
