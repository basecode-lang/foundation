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

#pragma once

#include <basecode/core/slice.h>

#define IS_PROXY(a)     (a->system && a->system->type == alloc_type_t::proxy)

namespace basecode {
    static constexpr u32 header_pad_value   = 0xffffffffu;
    using mspace                            = u0*;

    struct slab_t;
    struct alloc_t;
    struct proxy_pair_t;
    struct page_header_t;
    struct alloc_config_t {};

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

    using alloc_init_callback_t     = u0  (*)(alloc_t*, alloc_config_t*);
    using alloc_free_callback_t     = u0  (*)(alloc_t*, u0* mem, u32& freed_size);
    using alloc_fini_callback_t     = u0  (*)(alloc_t*, b8 enforce, u32* freed_size);
    using alloc_alloc_callback_t    = u0* (*)(alloc_t*, u32 size, u32 align, u32& allocated_size);
    using alloc_realloc_callback_t  = u0* (*)(alloc_t*, u0* mem, u32 size, u32 align, u32& old_size);

    struct alloc_system_t final {
        alloc_init_callback_t       init;
        alloc_fini_callback_t       fini;
        alloc_free_callback_t       free;
        alloc_alloc_callback_t      alloc;
        alloc_realloc_callback_t    realloc;
        alloc_type_t                type;
    };

    struct alloc_header_t final {
        u32                         size;
    };

    union alloc_subclass_t final {
        struct {
            mspace                  heap;
            u0*                     base;
            u32                     size;
        }                           dl;
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
        struct {
            u8*                     base_ptr;
            u8*                     curr_ptr;
            u8*                     next_page;
            u8*                     max_ptr;
            u8*                     node_state;
            u8*                     node_order;
            u0*                     buckets;
            u32                     num_buckets;
            u32                     bucket_limit;
            u32                     heap_size;
            u32                     heap_order;
            u32                     reserved;
            u32                     to_commit;
        }                           buddy;
    };

    struct alloc_t final {
        alloc_system_t*             system;
        alloc_t*                    backing;
        alloc_subclass_t            subclass;
        u32                         total_allocated;
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

            usize os_alloc_granularity();

            inline alloc_header_t* header(u0* data) {
                auto p = static_cast<u32*>(data);
                while (p[-1] == header_pad_value)
                    --p;
                return reinterpret_cast<alloc_header_t*>(p - 1);
            }

            b8 set_page_executable(u0* ptr, usize size);

            inline u32 size_with_padding(u32 size, u32 align) {
                return size + align + sizeof(alloc_header_t);
            }

            inline u0* align_forward(u0* p, u32 align, u32& adjust) {
                const auto pi = uintptr_t(p);
                const auto aligned = pi + (-pi & (align - 1));
                adjust = aligned - pi;
                return (u0*) aligned;
            }

            inline u0 fill(alloc_header_t* header, u0* data, u32 size) {
                header->size = size;
                auto p = (u32*) header + 1;
                while (p < data)
                    *p++ = header_pad_value;
            }

            inline u0* data_pointer(alloc_header_t* header, u32 align) {
                u0* p = header + 1;
                u32 adjust{};
                return align_forward(p, align, adjust);
            }

            alloc_t* make(alloc_type_t type, alloc_config_t* config = {});

            u0 free(alloc_t* alloc, b8 enforce = true, u32* freed_size = {});

            status_t init(alloc_type_t type, u32 heap_size = 32 * 1024 * 1024, u0* base = {});
        }

        alloc_t* unwrap(alloc_t* alloc);

        str::slice_t type_name(alloc_type_t type);

        str::slice_t status_name(status_t status);

        u0* alloc(alloc_t* alloc, u32* alloc_size = {});

        u0 free(alloc_t* alloc, u0* mem, u32* freed_size = {});

        u0 fini(alloc_t* alloc, b8 enforce = true, u32* freed_size = {});

        u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align = sizeof(u32));

        status_t init(alloc_t* alloc, alloc_type_t type, alloc_config_t* config = {});

        u0* alloc(alloc_t* alloc, u32 size, u32 align = sizeof(u32), u32* alloc_size = {});
    }
}

