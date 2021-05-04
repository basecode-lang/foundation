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

#include <basecode/core/variant.h>

namespace basecode::variant {
    u0 free(variant_array_t& array) {
        array.size = {};
        for (const auto& pair : array.values) {
            auto type = const_cast<variant_type_t*>(&pair.value);
            if (type->destroyer) {
                for (auto v : type->variants)
                    type->destroyer(v);
            }
            array::free(type->variants);
            memory::system::free(type->slab_alloc);
        }
        hashtab::free(array.values);
    }

    u0 init(variant_array_t& array, alloc_t* alloc) {
        array.size = {};
        hashtab::init(array.values, alloc);
    }
}
