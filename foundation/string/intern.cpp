// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <foundation/hashing/blake3.h>
#include "intern.h"

namespace basecode::intern {
    pool_t::pool_t(memory::allocator_t* allocator) : hashes(allocator),
                                                     interned(allocator),
                                                     allocator(allocator) {
        assert(allocator);
    }

    u0 free(pool_t& intern) {
        array::free(intern.hashes);
        array::free(intern.interned);
        memory::deallocate(intern.allocator, intern.buf);
    }

    u0 init(pool_t& intern, u32 buf_size) {
        intern.cursor = intern.buf = (u8*) memory::allocate(
            intern.allocator,
            buf_size);
    }

    pool_t make(u32 buf_size, memory::allocator_t* allocator) {
        pool_t intern(allocator);
        init(intern, buf_size);
        return intern;
    }

    string::slice_t intern(pool_t& intern, string::slice_t value) {
        hash_t hash{};
        hashing::blake3::hash256(value.data, value.length, hash.data);

        for (u32 i = 0; i < intern.hashes.size; ++i) {
            if (std::memcmp(hash.data, intern.hashes[i].data, sizeof(hash_t)) == 0)
                return intern.interned[i];
        }

        string::slice_t slice;
        slice.length = value.length;
        slice.data = intern.cursor;
        std::memcpy(intern.cursor, value.data, value.length);
        intern.cursor[value.length] = '\0';
        intern.cursor += value.length + 1;
        array::append(intern.hashes, hash);
        array::append(intern.interned, slice);

        return slice;
    }
}