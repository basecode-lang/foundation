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

extern "C" {
    #include <fe.h>
}
#include <basecode/core/path.h>
#include <basecode/core/types.h>
#include <basecode/core/context.h>

namespace basecode {
    namespace config {
        enum class status_t : u8 {
            ok          = 0,
            bad_input   = 22
        };

        namespace system {
            u0 fini();

            fe_Context* context();

            status_t init(u32 heap_size = 64 * 1024, alloc_t* alloc = context::top()->alloc);
        }

        status_t eval(const path_t& path, fe_Object** obj);

        status_t eval(const u8* source, u32 len, fe_Object** obj);

        status_t eval(const String_Concept auto& source, fe_Object** obj) {
            return eval(source.data, source.length, obj);
        }
    }
}
