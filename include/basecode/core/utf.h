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

#include <utf8proc.h>
#include <basecode/core/memory.h>

namespace basecode {
    template <typename T>
    struct utf_str_t final {
        alloc_t*                alloc;
        T*                      data;
        u32                     length;
        T& operator[](u32 index)                { return data[index]; }
        const T& operator[](u32 index) const    { return data[index]; }
    };

    template <>
    struct utf_str_t<u16> final {
        using value_type        = u16;
    };

    template <>
    struct utf_str_t<u32> final {
        using value_type        = u32;
    };

    template <typename T> concept Utf_String_Concept = (same_as<typename T::value_type, u32> || same_as<typename T::value_type, u16>) && requires(const T& t) {
        typename                T::value_type;
        {t.alloc}               -> same_as<alloc_t*>;
        {t.data}                -> same_as<typename T::value_type*>;
        {t.length}              -> same_as<u32>;
    };

    namespace utf {
        b8 isalnum(s32 cp);

        b8 islower(s32 cp);

        b8 isupper(s32 cp);

        b8 isdigit(s32 cp);

        b8 isalpha(s32 cp);

        b8 isxdigit(s32 cp);

        u0 free(Utf_String_Concept auto& str) {
            memory::free(str.alloc, str.data);
            str.data   = {};
            str.length = {};
        }

        s32 utf32_to_16(s32 cp, s16& h, s16& l);

        b8 decode(const s8* str, u32 length, s32& cp);

        u0 resize(Utf_String_Concept auto& str, u32 length) {
            str.data   = memory::realloc(str.alloc, str.data, length);
            str.length = length;
        }

        u0 init(Utf_String_Concept auto& str, alloc_t* alloc = context::top()->alloc) {
            str.alloc  = alloc;
            str.data   = {};
            str.length = {};
        }
    }
}
