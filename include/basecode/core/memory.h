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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/types.h>

#define IS_PROXY(a)                 ((a)->system                                \
                                     && (a)->system->type == alloc_type_t::proxy)

namespace basecode {
    struct slab_t;
    struct alloc_t;
    struct proxy_pair_t;
    struct buddy_block_t;
    struct page_header_t;
    struct alloc_config_t {};

    using mspace                    = u0*;

    enum class alloc_type_t : u8 {
        default_,
        bump,
        page,
        slab,
        proxy,
        trace,
        stack,
        buddy,
        dlmalloc,
    };

    struct mem_result_t final {
        u0*                     mem;
        s32                     size;
    } __attribute__((aligned(16)));

    using mem_fini_callback_t       = u32 (*)(alloc_t*);
    using mem_size_callback_t       = u32 (*)(alloc_t*, u0* mem);
    using mem_free_callback_t       = u32 (*)(alloc_t*, u0* mem);
    using mem_init_callback_t       = u0  (*)(alloc_t*, alloc_config_t*);
    using mem_alloc_callback_t      = mem_result_t (*)(alloc_t*,
                                                       u32 size,
                                                       u32 align);
    using mem_realloc_callback_t    = mem_result_t (*)(alloc_t*,
                                                       u0* mem,
                                                       u32 size,
                                                       u32 align);

    struct alloc_system_t final {
        mem_size_callback_t         size;
        mem_init_callback_t         init;
        mem_fini_callback_t         fini;
        mem_free_callback_t         free;
        mem_alloc_callback_t        alloc;
        mem_realloc_callback_t      realloc;
        alloc_type_t                type;
    };

    union alloc_subclass_t final {
        struct {
            mspace                  heap;
            u0*                     base;
            u32                     size;
        }                           dl;
        struct {
            u0*                     buf;
            u0*                     free;
            u32                     max_size;
        }                           stack;
        struct {
            u0*                     buf;
            u32                     offset;
            u32                     end_offset;
        }                           bump;
        struct {
            slab_t*                 tail;
            slab_t*                 head;
            u32                     count;
            u32                     buf_size;
            u32                     page_size;
            u32                     buf_max_count;
            u8                      num_pages;
            u8                      buf_align;
        }                           slab;
        struct {
            page_header_t*          cursor;
            page_header_t*          tail;
            page_header_t*          head;
            u32                     page_size;
            u32                     count;
            u8                      num_pages;
        }                           page;
        struct {
            proxy_pair_t*           pair;
            b8                      owner;
        }                           proxy;
        struct buddy_t {
            u0*                     heap;
            u0*                     extra_metadata;
            u32*                    block_index;
            buddy_block_t*          free_blocks;
            u32                     size;
            u32                     max_level;
            u32                     max_indexes;
            u32                     total_levels;
            u32                     metadata_size;
            u32                     min_allocation;
        }                           buddy;
    };

    struct alloc_t final {
        alloc_system_t*             system;
        alloc_t*                    backing;
        alloc_subclass_t            subclass;
        u32                         total_allocated;
        u32                         pad;

        std::weak_ordering operator<=>(const alloc_t& other) const {
            return std::weak_ordering::equivalent;
        }
    };

    namespace memory {
        enum class status_t : u8 {
            ok,
            invalid_allocator,
            invalid_default_allocator,
            invalid_allocation_system,
        };

        namespace system {
            u0 fini();

            usize os_page_size();

            u0 print_allocators();

            alloc_t* default_alloc();

            u32 free(alloc_t* alloc);

            usize os_alloc_granularity();

            status_t init(alloc_type_t type,
                          u32 heap_size = 32 * 1024 * 1024,
                          u0* base = {});

            b8 set_page_executable(u0* ptr, usize size);

            inline u0* align_forward(u0* p, u32 align, u32& adjust) {
                const auto pi = uintptr_t(p);
                const auto aligned = pi + (-pi & (align - 1));
                adjust = aligned - pi;
                return (u0*) aligned;
            }

            alloc_t* make(alloc_type_t type, alloc_config_t* config = {});
        }

        namespace internal {
            u32 fini(alloc_t* alloc);

            u32 size(alloc_t* alloc, u0* mem);

            u32 free(alloc_t* alloc, u0* mem);

            mem_result_t alloc(alloc_t* alloc, u32 size, u32 align);

            mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align);
        }

        alloc_t* unwrap(alloc_t* alloc);

        inline u32 fini(alloc_t* alloc) {
            return internal::fini(alloc);
        }

        inline u0* alloc(alloc_t* alloc) {
            return internal::alloc(alloc, 0, 0).mem;
        }

        const s8* type_name(alloc_type_t type);

        const s8* status_name(status_t status);

        inline u32 size(alloc_t* alloc, u0* mem) {
            return internal::size(alloc, mem);
        }

        inline u32 free(alloc_t* alloc, u0* mem) {
            return internal::free(alloc, mem);
        }

        status_t init(alloc_t* alloc,
                      alloc_type_t type,
                      alloc_config_t* config = {});

        inline u0* realloc(alloc_t* alloc,
                           u0* mem,
                           u32 size,
                           u32 align = sizeof(u64)) {
            return internal::realloc(alloc, mem, size, align).mem;
        }

        inline u0* alloc(alloc_t* alloc, u32 size, u32 align = sizeof(u64)) {
            return internal::alloc(alloc, size, align).mem;
        }
    }
}

