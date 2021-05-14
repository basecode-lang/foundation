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

#include <basecode/core/utf.h>

namespace basecode::utf {
    b8 isalnum(s32 cp) {
        return isalpha(cp) || isdigit(cp);
    }

    b8 isdigit(s32 cp) {
        return cp < 0x80 ? ::isdigit(cp) :
               utf8proc_category(cp) == UTF8PROC_CATEGORY_ND;
    }

    b8 isalpha(s32 cp) {
        if (cp < 0x80)
            return cp == '_' || ::isalpha(cp);
        switch (utf8proc_category(cp)) {
            case UTF8PROC_CATEGORY_LU:
            case UTF8PROC_CATEGORY_LL:
            case UTF8PROC_CATEGORY_LT:
            case UTF8PROC_CATEGORY_LM:
            case UTF8PROC_CATEGORY_LO:
                return true;
            default:
                break;
        }
        return false;
    }

    b8 islower(s32 cp) {
        return utf8proc_category(cp) == UTF8PROC_CATEGORY_LL;
    }

    b8 isupper(s32 cp) {
        return utf8proc_category(cp) == UTF8PROC_CATEGORY_LU;
    }

    b8 isxdigit(s32 cp) {
        if (cp < 0x80)
            return ::isxdigit(cp);
        const auto cat = utf8proc_category(cp);
        return cat == UTF8PROC_CATEGORY_ND || cat == UTF8PROC_CATEGORY_NL;
    }

    utf8_str_t utf16_to_utf8(const wchar_t* pwstr, alloc_t* alloc) {
        utf8_str_t str{};
        utf::init(str, alloc);
        auto p = pwstr;
        for (; *p; ++p) {
            utf16_codepoint_t cp16{};
            cp16.low  = *p;
            cp16.high = cp16.low == 0xffff ? *++p : 0;
            utf::append(str, utf16_to_utf8(cp16));
        }
        return str;
    }

    utf8_str_t utf32_to_utf8(const wchar_t* pwstr, alloc_t* alloc) {
        utf8_str_t str{};
        utf::init(str, alloc);
        auto p = pwstr;
        for (; *p; ++p)
            append(str, utf32_to_utf8(*p));
        return str;
    }
}
