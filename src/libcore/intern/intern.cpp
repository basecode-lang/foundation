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

#include <basecode/core/format/system.h>
#include <basecode/core/hashing/murmur.h>
#include <basecode/core/memory/bump_system.h>
#include <basecode/core/memory/page_system.h>
#include "intern.h"

namespace basecode::intern::pool {
    static inline u32 index_size(u32);
    static index_t get_index(u8*, u32);
    static u8* reserve_index(pool_t&, u32);
    static b8 find_bucket(const index_t&, u32, u32&);
    static b8 find_key(const index_t&, u32, u64, const string::slice_t&, u32&);

    static b8 find_key(
            const index_t& index,
            u32 capacity,
            u64 hash,
            const string::slice_t& key,
            u32& bucket_index) {
        for (u32 i = bucket_index; i < capacity; ++i) {
            if (index.hashes[i] == 0) break;
            if (index.hashes[i] == hash && index.slices[i] == key) {
                bucket_index = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            if (index.hashes[i] == 0) break;
            if (index.hashes[i] == hash && index.slices[i] == key) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    static inline u32 index_size(u32 capacity) {
        return (capacity * sizeof(u32))
               + alignof(u64) + (capacity * sizeof(u64))
               + alignof(string::slice_t) + (capacity * sizeof(string::slice_t));
    }

    static index_t get_index(u8* data, u32 capacity) {
        index_t index{};
        index.ids    = (u32*)               data;
        index.hashes = (u64*)               memory::align_forward(index.ids    + capacity, alignof(u64));
        index.slices = (string::slice_t*)   memory::align_forward(index.hashes + capacity, alignof(string::slice_t));
        return index;
    }

    static u8* reserve_index(pool_t& pool, u32 new_capacity) {
        auto new_data = (u8*) memory::alloc(pool.alloc, index_size(new_capacity), alignof(u32));
        auto index = get_index(new_data, new_capacity);
        std::memset(index.hashes, 0, new_capacity * sizeof(u64));
        return new_data;
    }

    static b8 find_bucket(const index_t& index, u32 capacity, u32& bucket_index) {
        for(u32 i = bucket_index; i < capacity; ++i) {
            if (index.hashes[i] == 0) {
                bucket_index = i;
                return true;
            }
        }
        for(u32 i = 0; i < bucket_index; ++i) {
            if (index.hashes[i] == 0) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    u0 free(pool_t& pool) {
        memory::release(&pool.bump_alloc);
        memory::release(&pool.page_alloc);
        if (pool.index) {
            memory::free(pool.alloc, pool.index);
            pool.index = {};
        }
    }

    u0 reset(pool_t& pool) {
        memory::page::reset(&pool.page_alloc);
        memory::bump::reset(&pool.bump_alloc);
        pool.size = {};
        std::memset(pool.index, 0, index_size(pool.capacity));
    }

    pool_t make(alloc_t* alloc) {
        pool_t pool{};
        init(pool, alloc);
        return pool;
    }

    result_t get(pool_t& pool, u32 id) {
        auto index = get_index(pool.index, pool.capacity);
        for (u32 i = 0; i < pool.capacity; ++i) {
            if (index.ids[i] == id) {
                return result_t{
                    .hash = index.hashes[i],
                    .slice = index.slices[i],
                    .id = index.ids[i],
                    .status = status_t::ok
                };
            }
        }
        return result_t{
            .status = status_t::not_found
        };
    }

    u0 init(pool_t& pool, alloc_t* alloc) {
        pool.id = 1;
        pool.alloc = alloc;

        page_config_t page_config{};
        page_config.backing = pool.alloc;
        page_config.page_size = memory::os_page_size() * 16;
        memory::init(&pool.page_alloc, alloc_type_t::page, &page_config);

        bump_config_t bump_config{};
        bump_config.backing = &pool.page_alloc;
        memory::init(&pool.bump_alloc, alloc_type_t::bump, &bump_config);
    }

    static status_t rehash(pool_t& pool, u32 new_capacity) {
        new_capacity = std::max<u32>(new_capacity, 16);

        auto new_index_data = reserve_index(pool, new_capacity);
        auto new_index = get_index(new_index_data, new_capacity);
        if (pool.index) {
            auto old_index = get_index(pool.index, pool.capacity);

            u32 count{};
            u32 old_idx{};
            while (old_idx < pool.capacity) {
                if (old_index.hashes[old_idx] == 0) {
                    ++old_idx;
                    continue;
                }

                u32 bucket_index = old_index.hashes[old_idx] * pool.capacity >> (u32) 32;
                if (!find_bucket(new_index, new_capacity, bucket_index))
                    return status_t::no_bucket;

                new_index.ids[bucket_index] = old_index.ids[old_idx];
                new_index.hashes[bucket_index] = old_index.hashes[old_idx];
                new_index.slices[bucket_index] = old_index.slices[old_idx];

                ++count;
                ++old_idx;
            }

            memory::free(pool.alloc, pool.index);
        }
        pool.index = new_index_data;
        pool.capacity = new_capacity;

        return status_t::ok;
    }

    result_t intern(pool_t& pool, string::slice_t value) {
        if (!pool.index || pool.size * 3 > pool.capacity * 2) {
            auto status = rehash(pool, pool.size * 3);
            if (!OK(status))
                return result_t{.status = status};
        }

        const auto hash = hashing::hash64(value);
        u32 bucket_index = hash * pool.capacity >> (u32) 32;
        auto index = get_index(pool.index, pool.capacity);

        if (find_key(index, pool.capacity, hash, value, bucket_index)) {
            return result_t{
                .hash = index.hashes[bucket_index],
                .slice = index.slices[bucket_index],
                .id = index.ids[bucket_index],
                .status = status_t::ok
            };
        }

        if (!find_bucket(index, pool.capacity, bucket_index)) {
            return result_t{.status = status_t::no_bucket};
        }

        auto data_ptr = (u8*) memory::alloc(&pool.bump_alloc, value.length + 1);
        std::memcpy(data_ptr, value.data, value.length);
        data_ptr[value.length] = '\0';

        index.ids[bucket_index] = pool.id++;
        index.hashes[bucket_index] = hash;

        auto& slice = index.slices[bucket_index];
        slice.data = data_ptr;
        slice.length = value.length;

        ++pool.size;

        return result_t{
            .hash = index.hashes[bucket_index],
            .slice = index.slices[bucket_index],
            .id = index.ids[bucket_index],
            .status = status_t::ok
        };
    }
}
