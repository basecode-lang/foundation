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

#include "system.h"

namespace basecode {
    struct bump_config_t : alloc_config_t {
        alloc_t*                backing;
    };

    namespace memory::bump {
        u0 reset(alloc_t* alloc);

        alloc_system_t* system();

        force_inline u0* buf(alloc_t* alloc) {
            auto subclass = &alloc->subclass.bump;
            return subclass->buf;
        }

        force_inline u16 offset(alloc_t* alloc) {
            auto subclass = &alloc->subclass.bump;
            return subclass->offset;
        }

        force_inline u16 end_offset(alloc_t* alloc) {
            auto subclass = &alloc->subclass.bump;
            return subclass->end_offset;
        }
    }
}

