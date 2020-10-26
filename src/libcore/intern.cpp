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
#include <basecode/core/string.h>
#include <basecode/core/buf_pool.h>

namespace basecode::intern {
    static b8 requires_rehash(const intern_t& pool) {
        return pool.capacity == 0 || pool.size + 1 > u32(f32(pool.capacity - 1) * pool.load_factor);
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
            if (pool.hashes[i] == hash && pool.strings[id - 1].value == key) {
                bucket_index = i;
                return true;
            }
        }
        for (u32 i = 0; i < bucket_index; ++i) {
            const auto id = pool.ids[i];
            if (id == 0) return false;
            if (pool.hashes[i] == hash && pool.strings[id - 1].value == key) {
                bucket_index = i;
                return true;
            }
        }
        return false;
    }

    u0 free(intern_t& pool) {
        array::free(pool.strings);
        memory::free(pool.alloc, pool.ids);
        pool.ids    = {};
        pool.hashes = {};
        pool.size   = pool.capacity = {};
    }

    u0 reset(intern_t& pool) {
        for (auto& str : pool.strings)
            buf_pool::release((u8*) str.value.data);
        array::reset(pool.strings);
        pool.size   = {};
        std::memset(pool.ids, 0, pool.capacity * sizeof(u32));
    }

    intern_t make(alloc_t* alloc) {
        intern_t pool{};
        init(pool, alloc);
        return pool;
    }

    b8 remove(intern_t& pool, u32 id) {
        if (id == 0 || id > pool.strings.size)
            return false;
        const auto& str = pool.strings[id - 1];
        buf_pool::release((u8*) str.value.data);
        pool.hashes[str.bucket_index] = pool.ids[str.bucket_index] = {};
        --pool.size;
        array::erase(pool.strings, id - 1);
        return true;
    }

    result_t get(intern_t& pool, u32 id) {
        if (id == 0 || id > pool.strings.size)
            return result_t{.status = status_t::not_found};
        const auto& str = pool.strings[id - 1];
        return result_t{pool.hashes[str.bucket_index], str.value, id, status_t::ok, false};
    }

    u0 reserve(intern_t& pool, u32 capacity) {
        array::reserve(pool.strings, capacity);
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
            u32 bucket_index = (u128(hash) * u128(new_capacity)) >> u128(64);
            if (!find_bucket(ids, new_capacity, bucket_index))
                return status_t::no_bucket;

            ids[bucket_index]    = id;
            hashes[bucket_index] = hash;
            pool.strings[id - 1].bucket_index = bucket_index;
        }

        memory::free(pool.alloc, pool.ids);
        pool.ids      = ids;
        pool.hashes   = hashes;
        pool.capacity = new_capacity;

        return status_t::ok;
    }

    result_t fold(intern_t& pool, const s8* data, s32 len) {
        if (requires_rehash(pool)) {
            auto status = rehash(pool, pool.size * 2);
            if (!OK(status))
                return result_t{.status = status, .new_value = false};
        }

        const auto value = slice::make(data, len == -1 ? strlen(data) : len);
        u64 hash         = hash::hash64(value);
        u32 bucket_index = (u128(hash) * u128(pool.capacity)) >> u128(64);

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

        if (!find_bucket(pool.ids, pool.capacity, bucket_index))
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
        pool.hashes      = {};
        pool.ids         = {};
        pool.alloc       = alloc;
        pool.load_factor = load_factor;
        pool.size        = pool.capacity = {};
        array::init(pool.strings, pool.alloc);
    }
}
