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

#include <cassert>
#include <basecode/core/types.h>
#include <basecode/core/iterator.h>
#include <basecode/core/array_common.h>

#define POOL_ARRAY_INIT(storage, type, array)        SAFE_SCOPE(                \
     auto ot = obj_pool::register_type<type>(storage);                          \
     proxy_array::init((array), ot->objects, ot->objects.size);)
#define POOL_ARRAY_APPEND(storage, type, array)                                 \
     ((array).size++, obj_pool::make<type>(storage))

namespace basecode {
    template<typename T, typename Backing_Type = T>
    struct proxy_array_t final {
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, true>;
        using Value_Type        = T;
        using Size_Per_16       [[maybe_unused]] = std::integral_constant<u32, 16 / sizeof(T)>;
        using Backing_Array     = const array_t<Backing_Type>*;

        Backing_Array           backing;
        u32                     start;
        u32                     size;

        const Value_Type& operator[](u32 index) const               { return (const Value_Type&) backing->data[start + index];  }

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const proxy_array_t* ref)                 { pos = ref->size;      }
            inline u0 next(const proxy_array_t* ref)                { UNUSED(ref); ++pos;   }
            inline Value_Type get(proxy_array_t* ref)               { return (*ref)[pos];   }
            inline u0 begin(const proxy_array_t* ref)               { UNUSED(ref); pos = 0; }
            inline b8 cmp(const iterator_state_t& s) const          { return pos != s.pos;  }
            inline const Value_Type get(const proxy_array_t* ref)   { return (*ref)[pos];   }
        };
        DECL_ITERS(proxy_array_t, Value_Type, iterator_state_t);
    };
    static_assert(sizeof(proxy_array_t<s32>) <= 16, "proxy_array_t<T> is now larger than 16 bytes!");

    namespace proxy_array {
        template <Proxy_Array T,
                  typename Value_Type = typename T::Value_Type>
        u0 init(T& array, const array_t<Value_Type>& backing, u32 start, u32 size = 0) {
            array.backing = &backing;
            array.start   = start;
            array.size    = size;
        }
    }
}
