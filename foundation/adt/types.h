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

#include <foundation/types.h>
#include <foundation/memory/system.h>

namespace basecode::adt {

    struct slice_t final {
        u8* data{};
        u32 length{};
    };

    template <typename T>
    struct array_t final {
        T* data{};
        u32 size{};
        u32 capacity{};
        memory::allocator_t* allocator{};
    };

    using string_t = array_t<u8>;

}