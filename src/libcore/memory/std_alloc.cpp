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

#include <basecode/core/memory.h>

namespace basecode::memory::std_alloc {
    u0 free(alloc_t* alloc, u0* mem, u32 size) {
        UNUSED(size);
        memory::free(alloc, mem);
    }

    u0* alloc(alloc_t* alloc, u32 size, u32 align) {
        return memory::alloc(alloc, size, align);
    }
}
