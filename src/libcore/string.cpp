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

#include <basecode/core/mutex.h>
#include <basecode/core/locale.h>
#include <basecode/core/string.h>
#ifdef _WIN32
#   include <win32_locale.h>
#endif
#include <basecode/core/hashtab.h>

namespace basecode::string {
    struct system_t final {
        alloc_t*                alloc;
        intern_t                pool;
        interned_array_t        interned;
        localized_strtab_t      localized;
        mutex_t                 lock;
    };

    system_t                    g_str_sys;

    namespace system {
        u0 fini() {
            intern::free(g_str_sys.pool);
            mutex::free(g_str_sys.lock);
            array::free(g_str_sys.interned);
            hashtab::free(g_str_sys.localized);
        }

        status_t init(alloc_t* alloc) {
            g_str_sys.alloc = alloc;
            mutex::init(g_str_sys.lock);
            intern::init(g_str_sys.pool, g_str_sys.alloc);
            array::init(g_str_sys.interned, g_str_sys.alloc);
            hashtab::init(g_str_sys.localized, g_str_sys.alloc);
            return status_t::ok;
        }

        const interned_array_t& interned() {
            return g_str_sys.interned;
        }

        const localized_strtab_t& localized() {
            return g_str_sys.localized;
        }
    }

    namespace interned {
        u32 scope_id() {
            scoped_lock_t lock(&g_str_sys.lock);
            return g_str_sys.interned.size;
        }

        b8 remove(u32 id) {
            scoped_lock_t lock(&g_str_sys.lock);
            auto result = intern::get(g_str_sys.pool, id);
            if (OK(result.status)) {
                array::erase(g_str_sys.interned, result);
                return intern::remove(g_str_sys.pool, id);
            }
            return false;
        }

        u0 scope_id(u32 value) {
            if (value == 0) return;
            scoped_lock_t lock(&g_str_sys.lock);
            for (u32 i = value - 1; i < g_str_sys.interned.size; i++) {
                const auto id = g_str_sys.interned[i].id;
                auto result = intern::get(g_str_sys.pool, id);
                if (OK(result.status))
                    intern::remove(g_str_sys.pool, id);
            }
            array::shrink(g_str_sys.interned, value);
        }

        intern::result_t get(u32 id) {
            scoped_lock_t lock(&g_str_sys.lock);
            return intern::get(g_str_sys.pool, id);
        }

        str::slice_t* get_slice(u32 id) {
            scoped_lock_t lock(&g_str_sys.lock);
            return intern::get_slice(g_str_sys.pool, id);
        }

        str::slice_t fold(const s8* value, s32 len) {
            scoped_lock_t lock(&g_str_sys.lock);
            len = len == -1 ? strlen(value) : len;
            auto result = intern::fold(g_str_sys.pool, value, len);
            if (result.new_value)
                array::append(g_str_sys.interned, result);
            return result.slice;
        }

        intern::result_t fold_for_result(const s8* value, s32 len) {
            scoped_lock_t lock(&g_str_sys.lock);
            len = len == -1 ? strlen(value) : len;
            auto result = intern::fold(g_str_sys.pool, value, len);
            if (result.new_value)
                array::append(g_str_sys.interned, result);
            return result;
        }
    }

    namespace localized {
        status_t add(u32 id, str::slice_t locale, const s8* value, s32 len) {
            auto key   = locale::make_key(id, locale);
            auto slice = hashtab::find(g_str_sys.localized, key);
            if (slice)
                return status_t::localized_duplicate_key;
            auto r = intern::fold(g_str_sys.pool, value, len);
            if (!OK(r.status))
                return status_t::localized_intern_failure;
            hashtab::insert(g_str_sys.localized, key, r);
            return status_t::ok;
        }

        status_t find(u32 id, str::slice_t& value, const s8* locale, s32 len) {
            intern::result_t* r{};
            if (!locale) {
                r = hashtab::find(g_str_sys.localized, locale::make_key(id));
            } else {
                const auto lc = slice::make(locale,
                                            len == -1 ? strlen(locale) : len);
                r = hashtab::find(g_str_sys.localized, locale::make_key(id, lc));
            }
            if (r && OK(r->status)) {
                value = r->slice;
                return status_t::ok;
            }
            return status_t::localized_not_found;
        }
    }
}
