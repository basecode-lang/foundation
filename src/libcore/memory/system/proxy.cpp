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

#include <basecode/core/memory/system/proxy.h>

namespace basecode::memory::proxy {
    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.proxy;
        if (sc->owner) {
            system::free(alloc->backing);
        }
        return alloc->total_allocated;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        return memory::internal::free(alloc->backing, mem);
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        return memory::internal::size(alloc->backing, mem);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto sc           = &alloc->subclass.proxy;
        auto proxy_config = (proxy_config_t*) config;
        alloc->backing = proxy_config->backing.alloc;
        sc->owner      = proxy_config->owner;
        BC_ASSERT_NOT_NULL(alloc->backing);
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        return memory::internal::alloc(alloc->backing, size, align);
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        return memory::internal::realloc(alloc->backing, mem, size, align);
    }

    struct system_t final {
        alloc_t*                        alloc;
        array_t<alloc_t*>               proxies;
    };

    alloc_system_t                      g_alloc_system{
        .size                           = size,
        .init                           = init,
        .fini                           = fini,
        .free                           = free,
        .alloc                          = alloc,
        .realloc                        = realloc,
        .type                           = alloc_type_t::proxy,
    };

    thread_local system_t               t_proxy_system{};

    u0 fini() {
        for (auto proxy : t_proxy_system.proxies)
            system::free(proxy);
        array::free(t_proxy_system.proxies);
    }

    u0 reset() {
        for (auto proxy : t_proxy_system.proxies)
            system::free(proxy);
        array::reset(t_proxy_system.proxies);
    }

    u0 free(alloc_t* alloc) {
        if (remove(alloc))
            system::free(alloc);
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }

    b8 remove(alloc_t* alloc) {
        if (!IS_PROXY(alloc))
            return false;
        array::erase(t_proxy_system.proxies, alloc);
        return true;
    }

    status_t init(alloc_t* alloc) {
        t_proxy_system.alloc = alloc;
        array::init(t_proxy_system.proxies, alloc);
        return status_t::ok;
    }

    const array_t<alloc_t*>& active() {
        return t_proxy_system.proxies;
    }

    alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner) {
        auto rc = string::interned::fold_for_result(name);
        if (!OK(rc.status))
            return nullptr;

        proxy_config_t config{};
        config.owner         = owner;
        config.backing.alloc = backing;
        auto proxy = system::make(&config);
        auto psc = &proxy->subclass.proxy;
        psc->name_id = rc.id;
        array::append(t_proxy_system.proxies, proxy);

        return proxy;
    }
}
