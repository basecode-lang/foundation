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

#include <basecode/core/obj_pool.h>

namespace basecode::obj_pool {
    static u0 free_slabs(obj_pool_t& pool) {
        hashtab::for_each_pair(
            pool.slabs,
            [](auto idx, const auto& key, auto& value, auto* user) -> u32 {
                memory::system::free(value);
                return 0;
            },
            nullptr);
    }

    static u0 free_storage(obj_pool_t& pool) {
        hashtab::for_each_pair(
            pool.storage,
            [](auto idx, const auto& key, auto& value, auto* user) -> u32 {
                if (value.destroy)
                    value.destroy(value.obj);
                return 0;
            },
            nullptr);
    }

    u0 free(obj_pool_t& pool) {
        free_storage(pool);
        free_slabs(pool);
        hashtab::free(pool.slabs);
        hashtab::free(pool.storage);
    }

    u0 reset(obj_pool_t& pool) {
        free_storage(pool);
        free_slabs(pool);
        hashtab::reset(pool.slabs);
        hashtab::reset(pool.storage);
    }

    status_t init(obj_pool_t& pool, alloc_t* alloc) {
        pool.alloc = alloc;
        hashtab::init(pool.slabs, pool.alloc);
        hashtab::init(pool.storage, pool.alloc);
        return status_t::ok;
    }
}
