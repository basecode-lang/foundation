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

#pragma once

#include <basecode/core/family.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    struct obj_type_t;
    struct obj_destroyer_t;

    using obj_array_t           = array_t<u0*>;
    using slab_table_t          = hashtab_t<u32, obj_type_t>;
    using storage_table_t       = hashtab_t<u0*, obj_destroyer_t>;
    using destroy_callback_t    = u0 (*)(const u0*);

    struct obj_destroyer_t final {
        u0*                     obj;
        alloc_t*                alloc;
        destroy_callback_t      destroy;
    };

    struct obj_type_t final {
        alloc_t*                alloc;
        u32                     type_id;
        const s8*               type_name;
        obj_array_t             objects;
    };

    struct obj_pool_t final {
        alloc_t*                alloc;
        slab_table_t            slabs;
        storage_table_t         storage;
        u32                     size;
    };
    static_assert(sizeof(obj_pool_t) <= 128, "obj_pool_t is now larger than 128 bytes!");

    namespace obj_pool {
        enum class status_t : u32 {
            ok                  = 0,
        };

        u0 reset(obj_pool_t& pool);

        template <typename T>
        u0 destroy(obj_pool_t& pool, T* obj) {
            u0* o = obj;
            auto d = hashtab::find(pool.storage, o);
            if (d) {
                const auto type_id = family_t<>::template type<T>;
                auto type = hashtab::find(pool.slabs, type_id);
                if (d->destroy)
                    d->destroy(o);
                hashtab::remove(pool.storage, o);
                array::erase(type->objects, o);
                memory::free(d->alloc, o);
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
                slab_config_t cfg{};
                cfg.backing     = pool.alloc;
                cfg.buf_size    = sizeof(T);
                cfg.buf_align   = alignof(T);
                type->alloc     = memory::system::make(alloc_type_t::slab, &cfg);
                type->type_id   = type_id;
                type->type_name = typeid(T).name();
                array::init(type->objects, pool.alloc);
            }
            return type;
        }

        template <typename T, typename... Args>
        T* make(obj_pool_t& pool, Args&&... args) {
            auto type = register_type<T>(pool);
            u0* mem = memory::alloc(type->alloc);
            array::append(type->objects, mem);
            auto d = hashtab::emplace(pool.storage, mem);
            d->obj     = mem;
            d->alloc   = type->alloc;
            if constexpr (std::is_destructible_v<T>) {
                d->destroy = [](const u0* x) -> u0 {
                    static_cast<const T*>(x)->~T();
                };
            } else {
                d->destroy = nullptr;
            }
            ++pool.size;
            return new (mem) T(std::forward<Args>(args)...);
        }

        u0 free(obj_pool_t& pool, b8 skip_storage = true);

        status_t init(obj_pool_t& pool, alloc_t* alloc = context::top()->alloc);
    }
}
