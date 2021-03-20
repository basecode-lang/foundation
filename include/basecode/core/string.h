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

#include <basecode/core/slice.h>
#include <basecode/core/intern.h>
#include <basecode/core/format.h>

namespace basecode::string {
    enum class status_t : u8 {
        ok,
        localized_not_found,
        localized_duplicate_key
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

    namespace localized {
        status_t find(u32 id, str::slice_t** value, const s8* locale = {}, s32 len = -1);

        status_t find(u32 id, str::slice_t** value, const String_Concept auto& locale = {}) {
            if (locale.length == 0)
                return find(id, value);
            else
                return find(id, value, (const s8*) locale.data, locale.length);
        }

        str::slice_t status_name(Status_Concept auto status) {
            str::slice_t* s{};
            return OK(find(u32(status), &s)) ? *s : interned::fold(format::format("[error] invalid status name id: {}", u32(status)));
        }

        status_t add(u32 id, str::slice_t locale, const s8* value, s32 len = -1);

        status_t add(u32 id, const String_Concept auto& locale, const String_Concept auto& value) {
            auto lc = slice::make(locale.data, locale.length);
            return add(id, lc, (const s8*) value.data, value.length);
        }
    }
}
