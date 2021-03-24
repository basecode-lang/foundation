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
    using destroy_callback_t    = u0 (*)(const u0*);

    struct obj_destroyer_t final {
        alloc_t*                alloc;
        u0*                     obj;
        destroy_callback_t      destroy;

        auto operator<=>(const obj_destroyer_t& rhs) const {
            return obj <=> rhs.obj;
        }
    };

    using slab_table_t          = hashtab_t<u32, alloc_t*>;
    using storage_table_t       = hashtab_t<u0*, obj_destroyer_t>;

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
            auto d = hashtab::find(pool.storage, (u0*) obj);
            if (d) {
                if (d->destroy)
                    d->destroy(d->obj);
                hashtab::remove(pool.storage, d->obj);
                memory::free(d->alloc, d->obj);
                --pool.size;
            }
        }

        inline b8 empty(const obj_pool_t& pool) {
            return pool.size == 0;
        }

        template <typename T, typename... Args>
        T* make(obj_pool_t& pool, Args&&... args) {
            const auto type_id = family_t<>::template type<T>;
            auto slab = hashtab::find(pool.slabs, type_id);
            if (!slab) {
                slab_config_t cfg{};
                cfg.backing   = pool.alloc;
                cfg.buf_size  = sizeof(T);
                cfg.buf_align = alignof(T);
                slab = memory::system::make(alloc_type_t::slab, &cfg);
                hashtab::insert(pool.slabs, type_id, slab);
            }
            u0* mem = memory::alloc(slab);
            auto d = hashtab::emplace(pool.storage, mem);
            d->obj     = mem;
            d->alloc   = slab;
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
