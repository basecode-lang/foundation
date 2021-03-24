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
    u0 reset(obj_pool_t& pool) {
        hashtab::reset(pool.storage);
        for (const auto& pair : pool.slabs)
            array::reset(const_cast<obj_array_t&>(pair.value.objects));
        hashtab::reset(pool.slabs);
    }

    u0 free(obj_pool_t& pool, b8 skip_storage) {
        if (!skip_storage) {
            for (const auto& pair : pool.storage) {
                if (pair.value.destroy)
                    pair.value.destroy(pair.value.obj);
            }
        }
        for (const auto& pair : pool.slabs) {
            auto& type = pair.value;
            array::free(const_cast<obj_array_t&>(type.objects));
            memory::system::free(type.alloc);
        }
        hashtab::free(pool.slabs);
        hashtab::free(pool.storage);
    }

    status_t init(obj_pool_t& pool, alloc_t* alloc) {
        pool.alloc = alloc;
        hashtab::init(pool.slabs, pool.alloc);
        hashtab::init(pool.storage, pool.alloc);
        return status_t::ok;
    }
}
