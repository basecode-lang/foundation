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

#include <basecode/core/bits.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::buf_pool {
    constexpr u32 max_pool_count = 7;

    static u32 s_pool_sizes[max_pool_count] = {
        32,
        128,
        512,
        2048,
        4096,
        8192,
        16384,
    };

    using lease_map_t           = hashtab_t<u64, lease_t>;

    struct system_t final {
        alloc_t*                alloc;
        alloc_t*                pools[max_pool_count];
        lease_map_t             leases;
    };

    system_t                    g_system;

    namespace system {
        u0 fini() {
            for (auto& pool : g_system.pools)
                memory::system::free(pool);
            hashtab::free(g_system.leases);
        }

        status_t init(alloc_t* alloc) {
            g_system.alloc = alloc;
            hashtab::init(g_system.leases, g_system.alloc);

            slab_config_t slab_config{};
            slab_config.backing   = g_system.alloc;
            slab_config.num_pages = DEFAULT_NUM_PAGES;
            slab_config.buf_align = alignof(u0*);

            for (u32 i = 0; i < max_pool_count; ++i) {
                slab_config.buf_size = s_pool_sizes[i];
                g_system.pools[i] = memory::system::make(alloc_type_t::slab,
                                                         &slab_config);
            }

            return status_t::ok;
        }
    }

    u0 release(u8* buf) {
        const auto key = (u64) buf;
        auto lease = hashtab::find(g_system.leases, key);
        if (!lease)
            return;
        memory::free(lease->alloc, buf);
        hashtab::remove(g_system.leases, key);
    }

    u8* retain(u32 size) {
        for (u32 i = 0 ; i < max_pool_count; ++i) {
            if (size > s_pool_sizes[i])
                continue;
            auto alloc = g_system.pools[i];
            auto buf   = (u8*) memory::alloc(alloc);
            const auto key = (u64) buf;
            auto lease = hashtab::emplace(g_system.leases, key);
            lease->buf   = buf;
            lease->size  = size;
            lease->alloc = alloc;
            return buf;
        }
        return nullptr;
    }

    const lease_t* lease_for(const u8* buf) {
        const auto key = (u64) buf;
        return hashtab::find(g_system.leases, key);
    }
}
