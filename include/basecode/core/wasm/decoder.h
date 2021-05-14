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

#include <basecode/core/wasm/wasm.h>

namespace basecode::wasm {
    namespace module {
        status_t decode(module_t& module);
    }

    namespace section {
        status_t decode(section_t* sect);

        status_t read_code(section_t* sect);

        status_t read_data(section_t* sect);

        status_t read_start(section_t* sect);

        status_t read_types(section_t* sect);

        status_t read_table(section_t* sect);

        status_t read_custom(section_t* sect);

        status_t read_memory(section_t* sect);

        status_t read_global(section_t* sect);

        status_t read_exports(section_t* sect);

        status_t read_imports(section_t* sect);

        status_t read_elements(section_t* sect);

        status_t read_functions(section_t* sect);

        status_t read_data_count(section_t* sect);
    }

    namespace instruction {
        status_t read_body(module_t& module, instruction_array_t& list);
    }
}
