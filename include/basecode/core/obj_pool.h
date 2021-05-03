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

#pragma once

#include <basecode/core/format.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::obj_pool {
    u0 reset(obj_pool_t& pool);

    template <typename T>
    u0 destroy(obj_pool_t& pool, T* obj) {
        u0* o = obj;
        const auto type_id = family_t<>::template type<T>;
        auto type = hashtab::find(pool.slabs, type_id);
        if (type) {
            if (type->destroyer)
                type->destroyer(o);
            array::erase(type->objects, o);
            memory::free(type->alloc, o);
            --pool.size;
        }
    }

    inline b8 empty(const obj_pool_t& pool) {
        return pool.size == 0;
    }

    template <typename T, typename... Args>
    obj_type_t* register_type(obj_pool_t& pool) {
        const auto type_id = family_t<>::template type<T>;
        auto [type, is_new] = hashtab::emplace2(pool.slabs, type_id);
        if (is_new) {
            type->type_id   = type_id;
            type->type_name = typeid(T).name();
            auto slab_name  = format::format("obj_pool<{}>::slab",
                                             type->type_name);

            slab_config_t cfg{};
            cfg.name          = str::c_str(slab_name);
            cfg.buf_size      = sizeof(T);
            cfg.buf_align     = alignof(T);
            cfg.num_pages     = DEFAULT_NUM_PAGES;
            cfg.backing.alloc = pool.alloc;
            type->alloc       = memory::system::make(&cfg);

            if constexpr (std::is_destructible_v<T>) {
                type->destroyer = [](const u0* x) -> u0 {
                    static_cast<const T*>(x)->~T();
                };
            } else {
                type->destroyer = nullptr;
            }
            array::init(type->objects, pool.alloc);
        }
        return type;
    }

    template <typename T, typename... Args>
    T* make(obj_pool_t& pool, Args&&... args) {
        auto type = register_type<T>(pool);
        u0* mem = memory::alloc(type->alloc);
        array::append(type->objects, mem);
        ++pool.size;
        return new (mem) T(std::forward<Args>(args)...);
    }

    u0 free(obj_pool_t& pool, b8 skip_storage = true);

    status_t init(obj_pool_t& pool,
                  alloc_t* alloc = context::top()->alloc.main);
}
