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
#include <basecode/core/array_common.h>

#define POOL_ARRAY_INIT(storage, type, array)        SAFE_SCOPE(                \
     auto ot = obj_pool::register_type<type>(storage);                          \
     proxy_array::init((array), ot->objects, ot->objects.size);)
#define POOL_ARRAY_APPEND(storage, type, array)                                 \
     ((array).size++, obj_pool::make<type>(storage))

namespace basecode::proxy_array {
    template <Proxy_Array T,
              typename Value_Type = typename T::Value_Type>
    u0 init(T& array,
            const array_t<Value_Type>& backing,
            u32 start,
            u32 size = 0) {
        array.backing = &backing;
        array.start   = start;
        array.size    = size;
    }
}
