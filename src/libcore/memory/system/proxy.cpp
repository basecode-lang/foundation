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

#include <basecode/core/str_array.h>
#include <basecode/core/stable_array.h>
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
        alloc->backing = proxy_config->backing;
        sc->owner      = proxy_config->owner;
        assert(alloc->backing);
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        return memory::internal::alloc(alloc->backing, size, align);
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        return memory::internal::realloc(alloc->backing, mem, size, align);
    }

    struct system_t final {
        alloc_t*                        alloc;
        proxy_symtab_t                  proxies;
        str_array_t                     names;
        stable_array_t<proxy_pair_t>    pairs;
        u16                             count;
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
        for (auto pair : t_proxy_system.pairs)
            system::free(pair->alloc);
        symtab::free(t_proxy_system.proxies);
        str_array::free(t_proxy_system.names);
        stable_array::free(t_proxy_system.pairs);
        t_proxy_system.count = {};
    }

    u0 reset() {
        for (auto pair : t_proxy_system.pairs)
            system::free(pair->alloc);
        symtab::reset(t_proxy_system.proxies);
        str_array::reset(t_proxy_system.names);
        stable_array::reset(t_proxy_system.pairs);
        t_proxy_system.count = {};
    }

    u0 free(alloc_t* proxy) {
        if (remove(proxy))
            system::free(proxy);
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }

    b8 remove(alloc_t* proxy) {
        if (!proxy || proxy->system->type != alloc_type_t::proxy)
            return false;
        auto pair = proxy->subclass.proxy.pair;
        const auto name = proxy::name(proxy);
        if (symtab::remove(t_proxy_system.proxies, name)) {
            stable_array::erase(t_proxy_system.pairs, pair->pair_id - 1);
            str_array::erase(t_proxy_system.names, pair->name_id - 1);
            --t_proxy_system.count;
            return true;
        }
        return false;
    }

    status_t init(alloc_t* alloc) {
        t_proxy_system.count = {};
        t_proxy_system.alloc = alloc;
        symtab::init(t_proxy_system.proxies, alloc);
        str_array::init(t_proxy_system.names, alloc);
        stable_array::init(t_proxy_system.pairs, alloc);
        return status_t::ok;
    }

    u0 active(proxy_array_t& list) {
        array::reserve(list, t_proxy_system.pairs.size);
        for (auto pair : t_proxy_system.pairs)
            array::append(list, pair);
    }

    alloc_t* find(str::slice_t name) {
        proxy_pair_t* pair{};
        return symtab::find(t_proxy_system.proxies, name, pair) ? pair->alloc : nullptr;
    }

    str::slice_t name(alloc_t* alloc) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        return t_proxy_system.names[alloc->subclass.proxy.pair->name_id - 1];
    }

    alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner) {
        ++t_proxy_system.count;

        str_t temp_name{};
        str::init(temp_name, t_proxy_system.alloc);
        str::reserve(temp_name, name.length + 12);
        defer(str::free(temp_name));
        {
            str_buf_t buf(&temp_name);
            format::format_to(buf,
                              "[proxy:{:04x}] {}",
                              t_proxy_system.count,
                              name);
        }

        str_array::append(t_proxy_system.names, temp_name);
        auto unique_name = t_proxy_system.names[t_proxy_system.names.size - 1];

        proxy_config_t config{};
        config.owner   = owner;
        config.backing = backing;
        auto proxy    = system::make(alloc_type_t::proxy, &config);
        auto psc      = &proxy->subclass.proxy;
        auto new_pair = &stable_array::append(t_proxy_system.pairs);
        new_pair->alloc   = proxy;
        new_pair->name_id = t_proxy_system.names.size;
        new_pair->pair_id = t_proxy_system.pairs.size;
        psc->pair         = new_pair;
        if (!symtab::insert(t_proxy_system.proxies, unique_name, new_pair))
            return nullptr;
        return proxy;
    }
}
