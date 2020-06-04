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

#include <basecode/core/intern.h>

namespace basecode {
    namespace string {
        enum class status_t : u8 {
            ok,
        };

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        namespace interned {
            u32 scope_id();

            b8 remove(u32 id);

            u0 scope_id(u32 value);

            intern::result_t get(u32 id);

            str::slice_t fold(const s8* value, s32 len = -1);

            str::slice_t fold(const String_Concept auto& value) {
                return fold((const s8*) value.data, value.length);
            }

            intern::result_t fold_for_result(const s8* value, s32 len = -1);

            intern::result_t fold_for_result(const String_Concept auto& value) {
                return fold_for_result((const s8*) value.data, value.length);
            }
        }
    }
}
