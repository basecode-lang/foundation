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

#include <basecode/core/macro.h>

namespace basecode::macro {
    struct macro_system_t final {
        alloc_t*                alloc;
    };

    thread_local macro_system_t t_macro_sys{};

    namespace system {
        u0 fini() {
        }

        status_t init(alloc_t* alloc) {
            t_macro_sys.alloc = alloc;
            return status_t::ok;
        }
    }
}
