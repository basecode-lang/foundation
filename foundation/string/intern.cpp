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

#include <foundation/format/system.h>
#include <foundation/hashing/murmur.h>
#include <foundation/string/ascii_string_formatters.h>
#include "intern.h"

namespace basecode::hashing {
    u64 hash64(const string::slice_t& key) {
        return murmur::hash64(key.data, key.length);
    }
}

namespace basecode::intern {
    [[maybe_unused]] static u0 print_pool(pool_t& pool) {
        for (u32 i = 0; i < pool.hashes.capacity; ++i) {
            format::print("i = {}, hash = {}:\n", i, pool.hashes[i]);
            for (const auto& entry : pool.interned[i].entries)
                format::print("\thash = {}, slice = {}\n", entry.hash, entry.slice);
        }
    }

    [[maybe_unused]] static u0 rehash(pool_t& pool, u32 new_capacity) {
        auto new_hashes = (u64*) memory::allocate(
            pool.allocator,
            sizeof(u64) * new_capacity,
            alignof(u64));
        auto new_interned = (interned_t*) memory::allocate(
            pool.allocator,
            sizeof(interned_t) * new_capacity,
            alignof(interned_t));
        std::memset(new_hashes, 0, sizeof(u64) * new_capacity);
        for (u32 i = 0; i < new_capacity; ++i)
            array::init(new_interned[i].entries, pool.allocator);

        for (u32 i = 0; i < pool.hashes.capacity; ++i) {
            if (pool.hashes[i] == 0)
                continue;
            const auto& original = pool.interned[i];
            for (const auto& entry : original.entries) {
                auto bucket_index = entry.hash % new_capacity;
                if (new_hashes[bucket_index] == 0)
                    new_hashes[bucket_index] = entry.hash;
                auto& interned = new_interned[bucket_index];
                array::append(interned.entries, entry);
            }
        }

        for (u32 i = 0; i < pool.interned.capacity; ++i)
            array::free(pool.interned[i].entries);
        memory::deallocate(pool.allocator, pool.hashes.data);
        memory::deallocate(pool.allocator, pool.interned.data);

        pool.hashes.data = new_hashes;
        pool.hashes.capacity = new_capacity;

        pool.interned.data = new_interned;
        pool.interned.capacity = new_capacity;
    }

    pool_t::pool_t(memory::allocator_t* allocator) : hashes(allocator),
                                                     interned(allocator),
                                                     allocator(allocator) {
        assert(allocator);
    }

    u0 free(pool_t& pool) {
        for (u32 i = 0; i < pool.interned.capacity; ++i)
            array::free(pool.interned[i].entries);
        array::free(pool.hashes);
        array::free(pool.interned);
        memory::deallocate(pool.allocator, pool.buf);
    }

    u0 init(pool_t& pool, u32 buf_size) {
        pool.cursor = pool.buf = (u8*) memory::allocate(
            pool.allocator,
            buf_size);
    }

    pool_t make(u32 buf_size, memory::allocator_t* allocator) {
        pool_t pool(allocator);
        init(pool, buf_size);
        return pool;
    }

    string::slice_t intern(pool_t& pool, string::slice_t value) {
        if (array::empty(pool.hashes))
            rehash(pool, 16);
        else if (pool.hashes.size * 3 > pool.hashes.capacity * 2)
            rehash(pool, pool.hashes.size * 2);

        auto hash = hashing::hash64(value);
        auto bucket_index = hash % pool.hashes.capacity;

        if (pool.hashes[bucket_index] != 0) {
            auto& current = pool.interned[bucket_index];
            for (const auto& entry : current.entries) {
                if (std::memcmp(value.data, entry.slice.data, entry.slice.length) == 0)
                    return entry.slice;
            }
        } else {
            pool.hashes[bucket_index] = hash;
            ++pool.hashes.size;
            ++pool.interned.size;
        }

        auto& interned = pool.interned[bucket_index];
        auto& entry = array::append(interned.entries);
        entry.hash = hash;
        entry.slice.length = value.length;
        entry.slice.data = pool.cursor;
        std::memcpy(pool.cursor, value.data, value.length);
        pool.cursor[value.length] = '\0';
        pool.cursor += value.length + 1;

        return entry.slice;
    }
}
