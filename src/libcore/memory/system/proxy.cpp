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
        proxy_map_t                     proxies;
        stable_array_t<proxy_pair_t>    proxy_pairs;
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
        for (auto pair : t_proxy_system.proxy_pairs)
            system::free(pair->alloc);
        symtab::free(t_proxy_system.proxies);
        stable_array::free(t_proxy_system.proxy_pairs);
    }

    u0 reset(b8 enforce) {
        for (auto pair : t_proxy_system.proxy_pairs)
            system::free(pair->alloc, enforce);
        symtab::reset(t_proxy_system.proxies);
        stable_array::reset(t_proxy_system.proxy_pairs);
    }

    proxy_list_t active() {
        proxy_list_t list{};
        array::init(list, t_proxy_system.alloc);
        array::reserve(list, t_proxy_system.proxy_pairs.size);
        for (auto pair : t_proxy_system.proxy_pairs)
            array::append(list, pair);
        return list;
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }

    str::slice_t name(alloc_t* alloc) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        const auto pair = alloc->subclass.proxy.pair;
        return slice::make(pair->name, pair->length);
    }

    u0 free(alloc_t* proxy, b8 enforce) {
        // XXX: need to erase the proxy_pair_t entry in the stable_array. use
        //      the proxy_pair_t::id for index to stable_array::erase call.

        // XXX: the api supports mutating the name on the proxy_pair_t. if we do
        //      this though, the symtab won't be able to find it again.  i'm not
        //      sure if changing the name makes any sense.
        if (symtab::remove(t_proxy_system.proxies, proxy::name(proxy))) {
            system::free(proxy, enforce);
            // XXX: need to add remove to intern pool which removes the entry
            //      from the internal hashtable *and* makes the block of memory
            //      for the byte data available again.  need a free list inside
            //      of the intern pool to do this.
        }
    }

    status_t init(alloc_t* alloc) {
        t_proxy_system.alloc = alloc;
        symtab::init(t_proxy_system.proxies, alloc);
        stable_array::init(t_proxy_system.proxy_pairs, alloc);
        return status_t::ok;
    }

    u0 name(alloc_t* alloc, str::slice_t name) {
        assert(alloc && alloc->system->type == alloc_type_t::proxy);
        auto       pair = alloc->subclass.proxy.pair;
        const auto len  = std::min<u32>(max_proxy_name_len - 1, name.length);
        std::memcpy(pair->name, name.data, len);
        pair->name[len] = '\0';
        pair->length = len;
    }

    alloc_t* make(alloc_t* backing, str::slice_t name, b8 owner) {
        proxy_pair_t* pair{};
        if (symtab::find(t_proxy_system.proxies, name, pair))
            return pair->alloc;

        proxy_config_t config{};
        config.owner   = owner;
        config.backing = backing;
        auto proxy = system::make(alloc_type_t::proxy, &config);

        const auto len = std::min<u32>(max_proxy_name_len - 1, name.length);
        auto new_pair = &stable_array::append(t_proxy_system.proxy_pairs);
        if (!symtab::insert(t_proxy_system.proxies, name, new_pair)) {
            return nullptr;
        }
        new_pair->alloc = proxy;
        new_pair->id = t_proxy_system.proxy_pairs.size;
        std::memcpy(new_pair->name, name.data, len);
        new_pair->name[len] = '\0';
        new_pair->length           = name.length;
        proxy->subclass.proxy.pair = new_pair;

        return proxy;
    }
}
