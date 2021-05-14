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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <basecode/core/str.h>
#include <basecode/core/string.h>

namespace basecode {
    str::slice_t get_env(const char* name) {
        auto ev = getenv(name);
        if (!ev || ev[0] == '\0')
            return {};
        return slice::make(ev, strlen(ev));
    }

    u0 win32_get_locale(str_t& str) {
        char iso639[10];
        char iso3166[10];
        char code_page[10];
        u16 primary, sub;
        const s8* script = "";

        auto ev = get_env("LC_ALL");
        if (!slice::empty(ev)) {
            str::append(str, ev);
            return;
        }

        ev = get_env("LC_MESSAGES");
        if (!slice::empty(ev)) {
            str::append(str, ev);
            return;
        }

        ev = get_env("LANG");
        if (!slice::empty(ev)) {
            str::append(str, ev);
            return;
        }

        auto lcid = GetThreadLocale();

        if (!GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, iso639, sizeof(iso639))
        ||  !GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, iso3166, sizeof(iso3166))
        ||  !GetLocaleInfo(lcid, LOCALE_IDEFAULTCODEPAGE, code_page, sizeof(code_page))) {
            str::append(str, "C", 1);
            return;
        }

        auto langid = LANGIDFROMLCID(lcid);
        primary = PRIMARYLANGID(langid);
        sub = SUBLANGID(langid);

        switch (primary) {
            case LANG_AZERI:
                switch (sub) {
                    case SUBLANG_AZERI_LATIN:
                        script = "@Latn";
                        break;
                    case SUBLANG_AZERI_CYRILLIC:
                        script = "@Cyrl";
                        break;
                    default:
                        break;
                }
                break;
            case LANG_SERBIAN:
                switch (sub) {
                    case SUBLANG_SERBIAN_LATIN:
                    case 0x06:
                        script = "@Latn";
                        break;
                    default:
                        break;
                }
                break;
            case LANG_UZBEK:
                switch (sub) {
                    case SUBLANG_UZBEK_LATIN:
                        script = "@Latn";
                        break;
                    case SUBLANG_UZBEK_CYRILLIC:
                        script = "@Cyrl";
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        str_buf_t buf(&str);
        format::format_to(buf,
                          "{}_{}{}.{}",
                          iso639,
                          iso3166,
                          script,
                          code_page);
    }
}
