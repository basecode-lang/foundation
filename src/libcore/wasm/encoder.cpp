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

#include <basecode/core/wasm/encoder.h>

namespace basecode::wasm {
    namespace module {
        status_t encode(module_t& module) {
            UNUSED(module);
            return status_t::not_implemented;
        }
    }

    namespace section {
        status_t encode(section_t* sect) {
            UNUSED(sect);
            return status_t::not_implemented;
        }
    }

    namespace instruction {
        status_t encode(module_t& module, instruction_array_t& list) {
            UNUSED(module);
            UNUSED(list);
            return status_t::not_implemented;
        }
    }
}
