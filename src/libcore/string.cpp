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

#include <basecode/core/mutex.h>
#include <basecode/core/string.h>

namespace basecode::string {
    using interned_list_t       = array_t<intern::result_t>;

    struct system_t final {
        alloc_t*                alloc;
        intern_t                pool;
        interned_list_t         interned;
        mutex_t                 lock;
    };

    system_t                    g_system;

    namespace system {
        u0 fini() {
            intern::free(g_system.pool);
            mutex::free(g_system.lock);
            array::free(g_system.interned);
        }

        status_t init(alloc_t* alloc) {
            g_system.alloc = alloc;
            mutex::init(g_system.lock);
            intern::init(g_system.pool, g_system.alloc);
            array::init(g_system.interned, g_system.alloc);
            return status_t::ok;
        }
    }

    namespace interned {
        u32 scope_id() {
            scoped_lock_t lock(g_system.lock);
            return g_system.interned.size;
        }

        b8 remove(u32 id) {
            scoped_lock_t lock(g_system.lock);
            auto result = intern::get(g_system.pool, id);
            if (OK(result.status)) {
                array::erase(g_system.interned, result);
                return intern::remove(g_system.pool, id);
            }
            return false;
        }

        u0 scope_id(u32 value) {
            if (value == 0) return;
            scoped_lock_t lock(g_system.lock);
            for (u32 i = value - 1; i < g_system.interned.size; i++) {
                const auto id = g_system.interned[i].id;
                auto result = intern::get(g_system.pool, id);
                if (OK(result.status))
                    intern::remove(g_system.pool, id);
            }
            array::shrink(g_system.interned, value);
        }

        intern::result_t get(u32 id) {
            scoped_lock_t lock(g_system.lock);
            return intern::get(g_system.pool, id);
        }

        str::slice_t fold(const s8* value, s32 len) {
            scoped_lock_t lock(g_system.lock);
            len = len == -1 ? strlen(value) : len;
            auto result = intern::fold(g_system.pool, value, len);
            if (result.new_value)
                array::append(g_system.interned, result);
            return result.slice;
        }

        intern::result_t fold_for_result(const s8* value, s32 len) {
            scoped_lock_t lock(g_system.lock);
            len = len == -1 ? strlen(value) : len;
            auto result = intern::fold(g_system.pool, value, len);
            if (result.new_value)
                array::append(g_system.interned, result);
            return result;
        }
    }
}
