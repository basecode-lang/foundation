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

namespace basecode::memory {
    struct allocator_t;
}

namespace basecode::context {

    struct context_t;

    ///////////////////////////////////////////////////////////////////////////

    void pop();

    context_t* current();

    void push(context_t* ctx);

    ///////////////////////////////////////////////////////////////////////////

    struct context_t {
        u0* user{};
        memory::allocator_t* allocator{};
    };

}
