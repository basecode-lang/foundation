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
#include <basecode/core/string/formatters.h>
#include "intern.h"

namespace basecode::hashing {
    u64 hash64(const string::slice_t& key) {
        return murmur::hash64(key.data, key.length);
    }
}

namespace basecode::intern::pool {
    static inline u32 page_size();
    static status_t make_page(pool_t&);
    static index_t get_index(u8*, u32);
    static u8* reserve_index(pool_t&, u32);
    static b8 find_free_bucket(const index_t&, u32, u32, u32&);
    static b8 find_key(const index_t&, u32, u32, u64, const slice_t&, u32&);

    static b8 find_key(
            const index_t& index,
            u32 capacity,
            u32 bucket_index,
            u64 hash,
            const slice_t& key,
            u32& key_idx) {
        for (u32 i = bucket_index; i < capacity; ++i) {
            const auto index_hash = index.hashes[i];
            if (index_hash == 0)
                break;
            if (index_hash == hash) {
                if (i < capacity - 1 && index.hashes[i + 1] == 0) {
                    key_idx = i;
                    return true;
                }
                if (index.slices[i] == key) {
                    key_idx = i;
                    return true;
                }
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            const auto index_hash = index.hashes[i];
            if (index_hash == 0)
                break;
            if (index_hash == hash) {
                if (i < bucket_index - 1 && index.hashes[i + 1] == 0) {
                    key_idx = i;
                    return true;
                }
                if (index.slices[i] == key) {
                    key_idx = i;
                    return true;
                }
            }
        }
        return false;
    }

    u0 free(pool_t& pool) {
        auto curr_page = pool.page;
        while (curr_page) {
            u8* prev_page = (u8*) *((u64*) curr_page);
            memory::deallocate(pool.allocator, curr_page);
            curr_page = prev_page;
        }
        memory::deallocate(pool.allocator, pool.index);
    }

    static u8* reserve_index(
            pool_t& pool,
            u32 new_capacity) {
        const auto size =
            (new_capacity * sizeof(u32))
            + alignof(u64)
            + (new_capacity * sizeof(u64))
            + alignof(slice_t)
            + (new_capacity * sizeof(slice_t));
        auto new_data = (u8*) memory::allocate(
            pool.allocator,
            size,
            alignof(u32));
        std::memset(new_data, 0, size);
        return new_data;
    }

    static b8 find_free_bucket(
            const index_t& index,
            u32 capacity,
            u32 bucket_index,
            u32& free_idx) {
        for (u32 i = bucket_index; i < capacity; ++i) {
            if (index.hashes[i] == 0) {
                free_idx = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            if (index.hashes[i] == 0) {
                free_idx = i;
                return true;
            }
        }
        return false;
    }

    static inline u32 page_size() {
        return (memory::os_page_size() * 16) - 16;
    }

    result_t get(pool_t& pool, id_t id) {
        auto index = get_index(pool.index, pool.capacity);
        for (u32 i = 0; i < pool.capacity; ++i) {
            if (index.ids[i] == id) {
                return result_t{
                    .id = index.ids[i],
                    .hash = index.hashes[i],
                    .slice = index.slices[i],
                    .status = status_t::ok
                };
            }
        }
        return result_t{
            .status = status_t::not_found
        };
    }

    static status_t make_page(pool_t& pool) {
        const auto mem_size = page_size();
        auto page = (u8*) memory::allocate(pool.allocator, mem_size);
        std::memset(page, 0, mem_size);
        if (pool.page) {
            u64 prev_ptr = (u64) pool.page;
            std::memcpy(page, &prev_ptr, sizeof(u64));

            u64 next_ptr = (u64) page;
            std::memcpy(pool.page + sizeof(u8*), &next_ptr, sizeof(u64));
        }
        pool.page = page;
        if (!pool.head)
            pool.head = pool.page;
        pool.offset = sizeof(u8*) * 2;
        pool.end_offset = pool.offset + (mem_size - pool.offset);
        return status_t::ok;
    }

    pool_t make(memory::allocator_t* allocator) {
        pool_t pool{};
        init(pool, allocator);
        return pool;
    }

    static index_t get_index(u8* data, u32 capacity) {
        index_t index{};
        index.ids    = (u32*) data;
        index.hashes = (u64*) memory::align_forward(
            data + (capacity * sizeof(u32)),
            alignof(u64));
        index.slices = (slice_t*) memory::align_forward(
            (u8*) index.hashes + (capacity * sizeof(u64)),
            alignof(slice_t));
        return index;
    }

    static status_t rehash(pool_t& pool, u32 new_capacity) {
        new_capacity = std::max<u32>(new_capacity, 8);

        auto new_index_data = reserve_index(pool, new_capacity);
        auto new_index = get_index(new_index_data, new_capacity);
        auto old_index = get_index(pool.index, pool.capacity);

        for (u32 i = 0; i < pool.capacity; ++i) {
            auto hash = old_index.hashes[i];
            if (hash == 0)
                continue;

            const auto bucket_index = hash % new_capacity;
            u32 free_idx{};
            const auto found = find_free_bucket(
                new_index,
                new_capacity,
                bucket_index,
                free_idx);
            if (!found)
                return status_t::no_available_bucket;

            new_index.ids[free_idx]    = old_index.ids[i];
            new_index.hashes[free_idx] = old_index.hashes[i];
            new_index.slices[free_idx] = old_index.slices[i];
        }

        memory::deallocate(pool.allocator, pool.index);

        pool.index = new_index_data;
        pool.capacity = new_capacity;

        return status_t::ok;
    }

    u0 init(pool_t& pool, memory::allocator_t* allocator) {
        pool.id = 1;
        pool.head = pool.page = {};
        pool.allocator = allocator;
        pool.offset = pool.end_offset = {};
    }

    result_t intern(pool_t& pool, string::slice_t value) {
        if (!pool.index || pool.size * 3 > pool.capacity * 2) {
            auto status = rehash(pool, pool.size * 2);
            if (status != status_t::ok) {
                return result_t{
                    .status = status
                };
            }
        }

        const auto hash = hashing::hash64(value);
        const auto bucket_index = hash % pool.capacity;
        auto index = get_index(pool.index, pool.capacity);

        u32 key_idx{};
        if (!find_key(index, pool.capacity, bucket_index, hash, value, key_idx)) {
            const auto found = find_free_bucket(
                index,
                pool.capacity,
                bucket_index,
                key_idx);
            if (!found) {
                return result_t{
                    .status = status_t::no_available_bucket
                };
            }

            if (!pool.page
            ||   pool.offset + value.length > pool.end_offset) {
                auto status = make_page(pool);
                if (status != status_t::ok) {
                    return result_t{
                        .status = status
                    };
                }
            }

            auto data_ptr = pool.page + pool.offset;
            std::memcpy(data_ptr, value.data, value.length);
            data_ptr[value.length] = '\0';
            pool.offset += value.length + 1;

            index.ids[key_idx] = pool.id++;
            index.hashes[key_idx] = hash;
            auto& slice = index.slices[key_idx];
            slice.data = data_ptr;
            slice.length = value.length;

            ++pool.size;
        }

        return result_t{
            .id = index.ids[key_idx],
            .hash = index.hashes[key_idx],
            .slice = index.slices[key_idx],
            .status = status_t::ok
        };
    }
}
