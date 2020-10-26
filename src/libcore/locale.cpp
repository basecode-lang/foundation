// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#ifdef _WIN32
#   include <win32_locale.h>
#endif
#include <basecode/core/log.h>
#include <basecode/core/defer.h>
#include <basecode/core/locale.h>
#include <basecode/core/slice_utils.h>

namespace basecode::locale {
    struct system_t final {
        alloc_t*                alloc;
        str_t                   lc_all;
        str_t                   locale;
    };

    system_t                    g_lc_sys;

    namespace system {
        u0 fini() {
            str::free(g_lc_sys.lc_all);
            str::free(g_lc_sys.locale);
        }

        status_t init(alloc_t* alloc) {
            g_lc_sys.alloc = alloc;
            str::init(g_lc_sys.lc_all, g_lc_sys.alloc);
            str::init(g_lc_sys.locale, g_lc_sys.alloc);
#ifdef _WIN32
            win32_get_locale(g_lc_sys.lc_all);
#else
            str::append(g_lc_sys.lc_all, setlocale(LC_ALL, ""));
#endif
            array_t<str::slice_t> fields{};
            array::init(fields, g_lc_sys.alloc);
            defer(array::free(fields));
            slice::to_fields(g_lc_sys.lc_all, fields, '.');
            str::append(g_lc_sys.locale, fields[0]);
            log::info("system locale: {}", g_lc_sys.locale);
            return status_t::ok;
        }
    }

    str::slice_t locale() {
        return slice::make(g_lc_sys.locale);
    }

    locale_key_t make_key(u32 id) {
        locale_key_t key{};
        ZERO_MEM(&key, locale_key_t);
        key.id = id;
        std::memcpy(key.locale, g_lc_sys.locale.data, g_lc_sys.locale.length);
        return key;
    }

    locale_key_t make_key(u32 id, str::slice_t locale) {
        locale_key_t key{};
        ZERO_MEM(&key, locale_key_t);
        key.id = id;
        std::memcpy(key.locale, locale.data, locale.length);
        return key;
    }
}
