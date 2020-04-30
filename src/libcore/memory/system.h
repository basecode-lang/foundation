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

#include <basecode/core/types.h>
#include <basecode/core/slice/slice.h>
#include "dlmalloc_config.h"

namespace basecode {
    static constexpr u32 header_pad_value   = 0xffffffffu;

    enum class alloc_type_t : u8 {
        system,
        bump,
        page,
        slab,
        proxy,
        trace,
        dlmalloc,
    };

    struct alloc_system_t;
    struct alloc_config_t {};

    struct alloc_header_t final {
        u32                     size;
    };

    struct alloc_t final {
        alloc_system_t*         system;
        alloc_t*                backing;
        union {
            mspace              heap;
            struct {
                u0*             buf;
                u32             offset;
                u32             end_offset;
            }                   bump;
            struct {
                u0*             tail;
                u0*             head;
                u32             count;
                u32             buf_size;
                u32             buf_align;
                u32             buf_max_count;
            }                   slab;
            struct {
                u0*             cursor;
                u0*             tail;
                u0*             head;
                u32             size;
                u32             count;
            }                   page;
            struct {
                string::slice_t name;
                u32             id;
            }                   proxy;
        }                       subclass;
        u32                     total_allocated;
    };

    using init_callback_t       = u0  (*)(alloc_t*, alloc_config_t*);
    using release_callback_t    = u0  (*)(alloc_t*);
    using free_callback_t       = u0  (*)(alloc_t*, u0* mem, u32& freed_size);
    using alloc_callback_t      = u0* (*)(alloc_t*, u32 size, u32 align, u32& allocated_size);
    using realloc_callback_t    = u0* (*)(alloc_t*, u0* mem, u32 size, u32 align, u32& old_size, u32& new_size);

    struct alloc_system_t final {
        init_callback_t         init{};
        alloc_type_t            type{};
        free_callback_t         free{};
        alloc_callback_t        alloc{};
        release_callback_t      release{};
        realloc_callback_t      realloc{};
    };

    namespace memory {
        enum class status_t : u8 {
            ok,
            invalid_allocator,
            invalid_allocation_system,
        };

        namespace system {
            u0 shutdown();

            usize os_page_size();

            alloc_t* default_alloc();

            inline alloc_header_t* header(u0* data) {
                auto p = static_cast<u32*>(data);
                while (p[-1] == header_pad_value)
                    --p;
                return reinterpret_cast<alloc_header_t*>(p - 1);
            }

            u0 free(alloc_t* alloc, b8 enforce = true);

            b8 set_page_executable(u0* ptr, usize size);

            inline u32 size_with_padding(u32 size, u32 align) {
                return size + align + sizeof(alloc_header_t);
            }

            inline u0* align_forward(u0* p, u32 align, u32& adjust) {
                auto pi = uintptr_t(p);
                const u32 mod = pi % align;
                if (mod) {
                    adjust = align - mod;
                    pi += adjust;
                }
                return (u0*) pi;
            }

            inline u0 fill(alloc_header_t* header, u0* data, u32 size) {
                header->size = size;
                auto p = reinterpret_cast<u32*>(header + 1);
                while (p < data)
                    *p++ = header_pad_value;
            }

            inline u0* data_pointer(alloc_header_t* header, u32 align) {
                u0* p = header + 1;
                u32 adjust{};
                return align_forward(p, align, adjust);
            }

            u0 initialize(u32 heap_size = 32 * 1024 * 1024, u0* base = {});

            alloc_t* make(alloc_type_t type, alloc_config_t* config = {});
        }

        u0 release(alloc_t* alloc, b8 enforce = true);

        u0* alloc(alloc_t* alloc, u32* alloc_size = {});

        u0 free(alloc_t* alloc, u0* mem, u32* freed_size = {});

        u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align = sizeof(u32));

        status_t init(alloc_t* alloc, alloc_type_t type, alloc_config_t* config = {});

        u0* alloc(alloc_t* alloc, u32 size, u32 align = sizeof(u32), u32* alloc_size = {});
    }
}

