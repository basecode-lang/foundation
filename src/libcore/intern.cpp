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

#include <basecode/core/intern.h>
#include <basecode/core/buf_pool.h>
#include <basecode/core/hash_common.h>

namespace basecode::intern {
    static status_t rehash(intern_t& pool, s32 new_capacity = -1);

    static b8 find_key(const intern_t& pool,
                       u64 hash,
                       str::slice_t key,
                       u32& bucket_index) {
        for (u32 i = bucket_index; i < pool.capacity; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) return false;
            if (pool.hashes[i] == hash
            &&  pool.strings[id - 1].value == key) {
                bucket_index = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) return false;
            if (pool.hashes[i] == hash
            &&  pool.strings[id - 1].value == key) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    static u32 buffer_size(const intern_t& pool, u32 capacity = 0) {
        if (!capacity)
            capacity = pool.capacity;
        const auto ids_size    = capacity * sizeof(intern_id);
        const auto hashes_size = capacity * sizeof(u64);
        return ids_size + alignof(u64) + hashes_size;
    }

    u0 free(intern_t& pool) {
        array::free(pool.strings);
        memory::free(pool.alloc, pool.ids);
        pool.ids     = {};
        pool.size    = pool.capacity = {};
        pool.hashes  = {};
        pool.cap_idx = {};
    }

    u0 reset(intern_t& pool) {
        for (auto& str : pool.strings)
            buf_pool::release((u8*) str.value.data);
        array::reset(pool.strings);
        pool.size   = {};
        std::memset(pool.ids, 0, buffer_size(pool));
    }

    intern_t make(alloc_t* alloc) {
        intern_t pool{};
        init(pool, alloc);
        return pool;
    }

    b8 remove(intern_t& pool, intern_id id) {
        if (id == 0 || id > pool.strings.size)
            return false;
        const auto& str = pool.strings[id - 1];
        buf_pool::release((u8*) str.value.data);
        pool.hashes[str.bucket_index] = pool.ids[str.bucket_index] = {};
        --pool.size;
        array::erase(pool.strings, id - 1);
        return true;
    }

    u0 reserve(intern_t& pool, u32 capacity) {
        array::reserve(pool.strings, capacity);
    }

    result_t get(intern_t& pool, intern_id id) {
        if (id == 0 || id > pool.strings.size)
            return result_t{.status = status_t::not_found};
        const auto& str = pool.strings[id - 1];
        return result_t{
            pool.hashes[str.bucket_index],
            str.value,
            id,
            status_t::ok,
            false
        };
    }

    str::slice_t* get_slice(intern_t& pool, intern_id id) {
        if (id == 0 || id > pool.strings.size)
            return nullptr;
        return &pool.strings[id - 1].value;
    }

    result_t fold(intern_t& pool, const s8* data, s32 len) {
        if (hash_common::requires_rehash(pool.size, pool.capacity, pool.load_factor)) {
            auto status = rehash(pool);
            if (!OK(status))
                return result_t{.status = status, .new_value = false};
        }

        const auto value = slice::make(data, len == -1 ? strlen(data) : len);
        u64 hash         = hash::hash64(value);
        u32 bucket_index = hash_common::range_reduction(hash, pool.capacity);
        if (find_key(pool, hash, value, bucket_index)) {
            const auto id = pool.ids[bucket_index];
            return result_t{
                hash,
                pool.strings[id - 1].value,
                id,
                status_t::ok,
                false
            };
        }

        if (!hash_common::find_free_bucket(pool.hashes, pool.capacity, bucket_index))
            return result_t{.status = status_t::no_bucket, .new_value = false};

        auto& str = array::append(pool.strings);
        auto buf = buf_pool::retain(value.length + 1);
        std::memcpy(buf, value.data, value.length);
        buf[value.length] = '\0';
        str.value        = slice::make(buf, value.length);
        str.bucket_index = bucket_index;

        const auto id = pool.strings.size;
        pool.ids[bucket_index]    = id;
        pool.hashes[bucket_index] = hash;

        ++pool.size;

        return result_t{hash, str.value, id, status_t::ok, true};
    }

    u0 init(intern_t& pool, alloc_t* alloc, f32 load_factor) {
        pool.ids         = {};
        pool.size        = pool.capacity = {};
        pool.alloc       = alloc;
        pool.hashes      = {};
        pool.cap_idx     = {};
        pool.load_factor = load_factor;
        array::init(pool.strings, pool.alloc);
    }

    static status_t rehash(intern_t& pool, s32 new_capacity) {
        s32 idx = s32(new_capacity == -1 ? pool.cap_idx :
                      hash_common::find_nearest_prime_capacity(new_capacity));
        f32 lf;
        do {
            new_capacity = s32(hash_common::prime_capacity(idx++));
            lf = f32(pool.size) / f32(new_capacity);
        } while (lf > pool.load_factor);
        pool.cap_idx = idx;

        const auto ids_size = new_capacity * sizeof(intern_id);
        const auto buf_size = buffer_size(pool, new_capacity);
        auto       buf      = (u8*) memory::alloc(pool.alloc, buf_size, alignof(intern_id));
        std::memset(buf, 0, buf_size);

        u32  hashes_align{};
        auto ids    = (intern_id*) buf;
        auto hashes = (u64*) memory::system::align_forward(buf + ids_size,
                                                           alignof(u64),
                                                           hashes_align);

        for (u32 i = 0; i < pool.capacity; ++i) {
            const auto id = pool.ids[i];
            if (id == 0)
                continue;

            const u64 hash = pool.hashes[i];
            u32 bucket_index = hash_common::range_reduction(hash, new_capacity);
            if (!hash_common::find_free_bucket(hashes, new_capacity, bucket_index))
                return status_t::no_bucket;

            ids[bucket_index]    = id;
            hashes[bucket_index] = hash;

            auto& s = pool.strings[id - 1];
            s.bucket_index = bucket_index;
        }

        memory::free(pool.alloc, pool.ids);
        pool.ids      = ids;
        pool.hashes   = hashes;
        pool.capacity = new_capacity;

        return status_t::ok;
    }
}
