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

using namespace basecode;

extern "C" u0 scm_free(u0* alloc, u0* mem) {
    memory::free((alloc_t*) alloc, mem);
}

extern "C" u0* scm_alloc(u0* alloc, usize size) {
    return memory::alloc((alloc_t*) alloc, size, sizeof(u0*));
}

extern "C" u0* scm_calloc(u0* alloc, usize count, usize size) {
    auto mem = memory::alloc((alloc_t*) alloc, count * size, size);
    std::memset(mem, 0, count * size);
    return mem;
}

