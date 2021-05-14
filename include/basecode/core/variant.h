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

namespace basecode {
    template <typename T>
    u0 variant_storage_t<T>::free() {
        if constexpr (std::is_destructible_v<T>) {
            for (auto& value : values)
                value.~T();
        }
        array::free(values);
    }

    template <typename T>
    u0 variant_storage_t<T>::init(alloc_t* alloc) {
        array::init(values, alloc);
    }

    template <typename T>
    inline u0 variant_storage_t<T>::visit(variant_visitor_t& visitor,
                                          u32 idx) {
        visitor.accept(values[idx]);
    }
}

namespace basecode::variant {
    template <typename T>
    inline variant_type_t* register_type(variant_array_t& array);

    u0 free(variant_array_t& array);

    u0 init(variant_array_t& array,
            alloc_t* alloc = context::top()->alloc.main);

    template <typename T>
    inline u0 append(variant_array_t& array, const T& value) {
        using Value_Type   = std::decay_t<T>;
        using Storage_Type = variant_storage_t<Value_Type>;

        auto type = register_type<T>(array);
        auto storage = (Storage_Type*) type->storage;

        auto& variant = array::append(storage->values);
        variant = (Value_Type) value;

        auto& seq = array::append(array.sequence);
        seq.storage = storage;
        seq.idx     = storage->values.size - 1;

        ++array.size;
    }

    template <typename T>
    inline u0 visit(const variant_array_t& array, T&& visitor) {
        for (const auto& seq : array.sequence)
            seq.storage->visit(visitor, seq.idx);
    }

    template <typename T>
    inline variant_type_t* register_type(variant_array_t& array) {
        using Value_Type   = std::decay_t<T>;
        using Storage_Type = variant_storage_t<Value_Type>;

        auto type_info = &typeid(T);
        auto [type, is_new] = hashtab::emplace2(array.values,
                                                type_info->hash_code());
        if (is_new) {
            type->type_info = type_info;
            auto mem = memory::alloc(array.values.alloc,
                                     sizeof(Storage_Type),
                                     alignof(Storage_Type));
            type->storage = new (mem) Storage_Type();
            type->storage->init(array.values.alloc);
        }
        return type;
    }
}
