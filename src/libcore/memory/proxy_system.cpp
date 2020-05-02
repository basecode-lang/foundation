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

#include "../defer.h"
#include "../intern.h"
#include "memory/memory.h"
#include "proxy_system.h"

namespace basecode::memory::proxy {
    static u0 release(alloc_t* alloc) {
        alloc->total_allocated = {};
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto proxy_config = (proxy_config_t*) config;
        alloc->backing = proxy_config->backing;
        assert(alloc->backing);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto backing = alloc->backing->system;
        backing->free(alloc->backing, mem, freed_size);
        alloc->total_allocated -= freed_size;
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto backing = alloc->backing->system;
        auto mem = backing->alloc(alloc->backing, size, align, allocated_size);
        alloc->total_allocated += allocated_size;
        return mem;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size) {
        auto backing = alloc->backing->system;
        auto new_mem = backing->realloc(alloc->backing, mem, size, align, old_size, new_size);
        alloc->total_allocated += (s32) new_size - (s32) old_size;
        return new_mem;
    }

    struct system_t final {
        alloc_t*                    alloc;
        intern_t                    intern;
        proxy_map_t                 proxies;
    };

    alloc_system_t          g_alloc_system{
        .init                       = init,
        .type                       = alloc_type_t::proxy,
        .free                       = free,
        .alloc                      = alloc,
        .release                    = release,
        .realloc                    = realloc,
    };

    thread_local system_t   g_proxy_system{};

    u0 shutdown() {
        auto proxies = hashtable::values(g_proxy_system.proxies);
        defer(array::free(proxies));
        for (auto alloc : proxies)
            system::free(alloc);
        hashtable::free(g_proxy_system.proxies);
        intern::free(g_proxy_system.intern);
    }

    u0 reset(b8 enforce) {
        auto proxies = hashtable::values(g_proxy_system.proxies);
        defer(array::free(proxies));
        for (auto alloc : proxies)
            system::free(alloc, enforce);
        hashtable::reset(g_proxy_system.proxies);
        intern::reset(g_proxy_system.intern);
    }

    proxy_list_t active() {
        return hashtable::values(g_proxy_system.proxies);
    }

    u32 id(alloc_t* alloc) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        return alloc->subclass.proxy.id;
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }

    u0 id(alloc_t* alloc, u32 id) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        alloc->subclass.proxy.id = id;
    }

    u0 free(alloc_t* proxy, b8 enforce) {
        if (hashtable::remove(g_proxy_system.proxies, proxy::id(proxy))) {
            system::free(proxy, enforce);
            // XXX: need to add remove to intern pool which removes the entry
            //      from the internal hashtable *and* makes the block of memory
            //      for the byte data available again.  need a free list inside
            //      of the intern pool to do this.
        }
    }

    string::slice_t name(alloc_t* alloc) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        return alloc->subclass.proxy.name;
    }

    status_t initialize(alloc_t* alloc) {
        g_proxy_system.alloc = alloc;
        hashtable::init(g_proxy_system.proxies, alloc);
        intern::init(g_proxy_system.intern, alloc);
        return status_t::ok;
    }

    u0 name(alloc_t* alloc, string::slice_t name) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        alloc->subclass.proxy.name = name;
    }

    alloc_t* make(alloc_t* backing, string::slice_t name) {
        auto interned = intern::intern(g_proxy_system.intern, name);
        if (!OK(interned.status))
            return {};
        auto proxy = hashtable::find(g_proxy_system.proxies, interned.id);
        if (proxy)
            return proxy;
        proxy_config_t config{};
        config.backing = backing;
        proxy = system::make(alloc_type_t::proxy, &config);
        proxy::name(proxy, interned.slice);
        proxy::id(proxy, interned.id);
        hashtable::insert(g_proxy_system.proxies, interned.id, proxy);
        return proxy;
    }
}
