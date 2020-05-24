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

#include <basecode/core/str_array.h>
#include <basecode/core/stable_array.h>
#include <basecode/core/memory/system/proxy.h>

namespace basecode::memory::proxy {
    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto proxy_config = (proxy_config_t*) config;
        alloc->backing = proxy_config->backing;
        alloc->subclass.proxy.owner = proxy_config->owner;
        assert(alloc->backing);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto backing = alloc->backing->system;
        backing->free(alloc->backing, mem, freed_size);
        assert(freed_size <= alloc->total_allocated);
        alloc->total_allocated -= freed_size;
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        auto sc = &alloc->subclass.proxy;
        if (sc->owner) {
            u32 temp_freed{};
            system::free(alloc->backing, enforce, &temp_freed);
            assert(temp_freed <= alloc->total_allocated);
            alloc->total_allocated -= temp_freed;
            if (freed_size) *freed_size = temp_freed;
            if (enforce) assert(alloc->total_allocated == 0);
        }
        alloc->total_allocated = {};
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        auto backing = alloc->backing->system;
        auto mem = backing->alloc(alloc->backing, size, align, alloc_size);
        alloc->total_allocated += alloc_size;
        return mem;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size) {
        auto backing = alloc->backing->system;
        auto new_mem = backing->realloc(alloc->backing, mem, size, align, old_size);
        alloc->total_allocated += (s32) (system::size_with_padding(size, align) - old_size);
        return new_mem;
    }

    struct system_t final {
        alloc_t*                        alloc;
        proxy_symtab_t                  proxies;
        str_array_t                     names;
        stable_array_t<proxy_pair_t>    pairs;
        u16                             count;
    };

    alloc_system_t                      g_alloc_system{
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

    u0 reset(b8 enforce) {
        for (auto pair : t_proxy_system.pairs)
            system::free(pair->alloc, enforce);
        symtab::reset(t_proxy_system.proxies);
        str_array::reset(t_proxy_system.names);
        stable_array::reset(t_proxy_system.pairs);
        t_proxy_system.count = {};
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }

    u0 active(proxy_array_t& list) {
        array::reserve(list, t_proxy_system.pairs.size);
        for (auto pair : t_proxy_system.pairs)
            array::append(list, pair);
    }

    str::slice_t name(alloc_t* alloc) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        return t_proxy_system.names[alloc->subclass.proxy.pair->name_id - 1];
    }

    u0 free(alloc_t* proxy, b8 enforce) {
        assert(proxy && proxy->system->type == alloc_type_t::proxy);
        if (symtab::remove(t_proxy_system.proxies, proxy::name(proxy))) {
            stable_array::erase(t_proxy_system.pairs, proxy->subclass.proxy.pair->id - 1);
            str_array::erase(t_proxy_system.names, proxy->subclass.proxy.pair->name_id - 1);
            system::free(proxy, enforce);
        }
    }

    status_t init(alloc_t* alloc) {
        t_proxy_system.count = {};
        t_proxy_system.alloc = alloc;
        symtab::init(t_proxy_system.proxies, alloc);
        str_array::init(t_proxy_system.names, alloc);
        stable_array::init(t_proxy_system.pairs, alloc);
        return status_t::ok;
    }

    alloc_t* find(str::slice_t name) {
        proxy_pair_t* pair{};
        return symtab::find(t_proxy_system.proxies, name, pair) ? pair->alloc : nullptr;
    }

    alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner) {
        t_proxy_system.count++;

        str_t temp_name{};
        str::init(temp_name, t_proxy_system.alloc);
        str::reserve(temp_name, name.length + 12);
        defer(str::free(temp_name));
        {
            str_buf_t buf(&temp_name);
            format::format_to(buf, "[proxy:{:04x}] {}", t_proxy_system.count, name);
        }

        str_array::append(t_proxy_system.names, temp_name);
        auto unique_name = t_proxy_system.names[t_proxy_system.names.size - 1];

        proxy_config_t config{};
        config.owner   = owner;
        config.backing = backing;
        auto proxy = system::make(alloc_type_t::proxy, &config);
        auto new_pair = &stable_array::append(t_proxy_system.pairs);
        if (!symtab::insert(t_proxy_system.proxies, unique_name, new_pair))
            return nullptr;
        new_pair->alloc            = proxy;
        new_pair->name_id          = t_proxy_system.names.size;
        new_pair->id               = t_proxy_system.pairs.size;
        proxy->subclass.proxy.pair = new_pair;

        return proxy;
    }
}
