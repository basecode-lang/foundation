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

#include <basecode/core/types.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::variant {
    u0 free(variant_array_t& array);

    template <typename T>
    inline variant_type_t* register_type(variant_array_t& array) {
        auto type_info = &typeid(T);
        auto [type, is_new] = hashtab::emplace2(array.values,
                                                type_info->hash_code());
        if (is_new) {
            array::init(type->variants, array.values.alloc);
            type->type_info = type_info;
            type->destroyer = {};
            auto name = type_info->name();
            auto slab_name = format::format("variant_array<{}>::slab", name);

            slab_config_t cfg{};
            cfg.name          = str::c_str(slab_name);
            cfg.buf_size      = sizeof(T);
            cfg.buf_align     = alignof(T);
            cfg.num_pages     = DEFAULT_NUM_PAGES;
            cfg.backing.alloc = array.values.alloc;
            type->slab_alloc  = memory::system::make(&cfg);

            if constexpr (std::is_destructible_v<T>) {
                type->destroyer = [](const u0* x) -> u0 {
                    static_cast<const T*>(x)->~T();
                };
            }
        }
        return type;
    }

    template <typename T>
    inline u0 append(variant_array_t& array, T&& value) {
        auto type = register_type<T>(array);
        u0* mem = memory::alloc(type->slab_alloc);
        auto variant = new (mem) T(value);
        array::append(type->variants, variant);
        ++array.size;
    }

    template <typename T>
    inline u0 visit(variant_array_t& array, T&& visitor) {
        for (const auto& pair : array.values) {
            const variant_type_t* type = &pair.value;
            const auto& type_info = *type->type_info;
            if (typeid(u8)  == type_info) {
                for (auto x : type->variants)
                    visitor(*((u8*) x));
            } else if (typeid(s8) == type_info) {
                for (auto x : type->variants)
                    visitor(*((s8*) x));
            } else if (typeid(u32) == type_info) {
                for (auto x : type->variants)
                    visitor(*((u32*) x));
            } else if (typeid(s32) == type_info) {
                for (auto x : type->variants)
                    visitor(*((s32*) x));
            } else if (typeid(f32) == type_info) {
                for (auto x : type->variants)
                    visitor(*((f32*) x));
            } else if (typeid(f64) == type_info) {
                for (auto x : type->variants)
                    visitor(*((f64*) x));
            }
        }
    }

    u0 init(variant_array_t& array, alloc_t* alloc = context::top()->alloc.main);
}
