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
#include <basecode/core/memory/system/bump.h>
#include <basecode/core/memory/system/page.h>

namespace basecode::intern {
    static str::slice_t s_status_names[] = {
        "ok"_ss,
        "no bucket"_ss,
        "not found"_ss,
    };

    static b8 requires_rehash(const intern_t& pool) {
        return pool.capacity == 0 || pool.size + 1 > (pool.capacity - 1) * pool.load_factor;
    }

    static b8 find_bucket(const u32* ids, u32 capacity, u32& bucket_index) {
        for(u32 i = bucket_index; i < capacity; ++i) {
            if (ids[i] == 0) {
                bucket_index = i;
                return true;
            }
        }
        for(u32 i = 0; i < bucket_index; ++i) {
            if (ids[i] == 0) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    static b8 find_key(const intern_t& pool, u64 hash, const str::slice_t& key, u32& bucket_index) {
        for (u32 i = bucket_index; i < pool.capacity; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) return false;
            if (pool.hashes[i] == hash && pool.buf[id - 1].key == key) {
                bucket_index = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) return false;
            if (pool.hashes[i] == hash && pool.buf[id - 1].key == key) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    u0 free(intern_t& pool) {
        assoc_array::free(pool.buf);
        memory::free(pool.alloc, pool.ids);
        pool.ids    = {};
        pool.hashes = {};
        pool.size   = pool.capacity = {};
    }

    u0 reset(intern_t& pool) {
        assoc_array::reset(pool.buf);
        pool.size   = {};
        std::memset(pool.ids, 0, pool.capacity * sizeof(u32));
    }

    intern_t make(alloc_t* alloc) {
        intern_t pool{};
        init(pool, alloc);
        return pool;
    }

    result_t get(intern_t& pool, u32 id) {
        if (id == 0 || id > pool.buf.size)
            return result_t{.status = status_t::not_found};
        auto pair = pool.buf[id - 1];
        u32 bucket_index = *pair.value;
        return result_t{pool.hashes[bucket_index], pair.key, id, status_t::ok, false};
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    static status_t rehash(intern_t& pool, u32 new_capacity) {
        new_capacity = std::max<u32>(new_capacity, std::ceil(std::max<u32>(16, new_capacity) / pool.load_factor));

        const auto ids_size    = new_capacity * sizeof(u32);
        const auto hashes_size = new_capacity * sizeof(u64);
        const auto buf_size    = ids_size + alignof(u64) + hashes_size;

        auto buf    = (u8*) memory::alloc(pool.alloc, buf_size, alignof(u32));
        std::memset(buf, 0, ids_size);

        u32  hashes_align{};
        auto ids    = (u32*) buf;
        auto hashes = (u64*) memory::system::align_forward(buf + ids_size, alignof(u64), hashes_align);

        for (u32 i = 0; i < pool.capacity; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) continue;

            const u64 hash = pool.hashes[i];
            u32 bucket_index = ((u128) hash * (u128) new_capacity) >> 64;
            if (!find_bucket(ids, new_capacity, bucket_index))
                return status_t::no_bucket;

            ids[bucket_index]       = id;
            hashes[bucket_index]    = hash;
            pool.buf.values[id - 1] = bucket_index;
        }

        memory::free(pool.alloc, pool.ids);
        pool.ids      = ids;
        pool.hashes   = hashes;
        pool.capacity = new_capacity;

        return status_t::ok;
    }

    u0 init(intern_t& pool, alloc_t* alloc, f32 load_factor) {
        pool.hashes      = {};
        pool.ids         = {};
        pool.alloc       = alloc;
        pool.load_factor = load_factor;
        pool.size        = pool.capacity = {};
        assoc_array::init(pool.buf, pool.alloc);
    }

    result_t intern(intern_t& pool, const str::slice_t& value) {
        if (requires_rehash(pool)) {
            auto status = rehash(pool, pool.size * 2);
            if (!OK(status))
                return result_t{.status = status, .new_value = false};
        }

        u64 hash         = hash::hash64(value);
        u32 bucket_index = ((u128) hash * (u128) pool.capacity) >> 64;

        if (find_key(pool, hash, value, bucket_index)) {
            const auto id = pool.ids[bucket_index];
            return result_t{hash, pool.buf[id - 1].key, id, status_t::ok, false};
        }

        if (!find_bucket(pool.ids, pool.capacity, bucket_index))
            return result_t{.status = status_t::no_bucket, .new_value = false};

        assoc_array::append(pool.buf, value, bucket_index);

        const auto id = pool.buf.size;
        pool.ids[bucket_index]    = id;
        pool.hashes[bucket_index] = hash;

        ++pool.size;

        return result_t{hash, pool.buf[id - 1].key, id, status_t::ok, true};
    }

    u0 reserve(intern_t& pool, u32 key_capacity, u32 value_capacity) {
        assoc_array::reserve_keys(pool.buf, key_capacity);
        assoc_array::reserve_values(pool.buf, value_capacity);
    }
}
