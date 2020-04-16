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

#include <basecode/core/intern/intern.h>
#include <basecode/core/format/system.h>
#include "proxy.h"
#include "page_system.h"
#include "bump_system.h"
#include "proxy_system.h"

namespace basecode::memory::proxy {
    struct system_t final {
        alloc_t*            alloc;
        alloc_t             page_alloc;
        alloc_t             bump_alloc;
        intern::pool_t      intern_pool;
        proxy_list_t        proxies;
    };

    thread_local system_t g_system{};

    u0 reset(b8 enforce) {
        for (auto proxy : g_system.proxies)
            free(proxy, enforce);
        array::reset(g_system.proxies);
        memory::bump::reset(&g_system.bump_alloc);
        memory::page::reset(&g_system.page_alloc);
        intern::pool::reset(g_system.intern_pool);
    }

    u0 shutdown() {
        for (auto proxy : g_system.proxies)
            free(proxy);
        array::free(g_system.proxies);
        memory::release(&g_system.bump_alloc);
        memory::release(&g_system.page_alloc);
        intern::pool::free(g_system.intern_pool);
    }

    const proxy_list_t& active() {
        return g_system.proxies;
    }

    u0 free(alloc_t* proxy, b8 enforce) {
        release(proxy, enforce);
    }

    status_t initialize(alloc_t* alloc) {
        g_system.alloc = alloc;

        page_config_t page_config{};
        page_config.backing = g_system.alloc;
        page_config.page_size = memory::os_page_size() * 16;
        memory::init(&g_system.page_alloc, alloc_type_t::page, &page_config);

        bump_config_t bump_config{};
        bump_config.backing = &g_system.page_alloc;
        memory::init(&g_system.bump_alloc, alloc_type_t::bump, &bump_config);

        array::init(g_system.proxies, alloc);
        intern::pool::init(g_system.intern_pool, alloc);
        return status_t::ok;
    }

    alloc_t* make(alloc_t* backing, string::slice_t name) {
        auto interned = intern::pool::intern(g_system.intern_pool, name);
        if (interned.id > g_system.proxies.size) {
            proxy_config_t config{};
            config.backing = backing;
            auto proxy = (alloc_t*) memory::alloc(&g_system.bump_alloc, sizeof(alloc_t), alignof(alloc_t));
            array::append(g_system.proxies, proxy);
            init(proxy, alloc_type_t::proxy, &config);
            proxy->name = interned.slice;
            return proxy;
        }
        return g_system.proxies[interned.id - 1];
    }
}