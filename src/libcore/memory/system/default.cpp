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

#include <basecode/core/memory/system/default.h>

namespace basecode::memory::default_ {
    constexpr u32 header_pad_value = 0xffffffffu;

    struct alloc_header_t final {
        u32                     size;
    };

    inline static alloc_header_t* header(u0* data) {
        auto p = static_cast<u32*>(data);
        while (p[-1] == header_pad_value)
            --p;
        return reinterpret_cast<alloc_header_t*>(p - 1);
    }

    inline static u32 size_with_padding(u32 size, u32 align) {
        return size + align + sizeof(alloc_header_t);
    }

    inline static u0 fill(alloc_header_t* header, u0* data, u32 size) {
        header->size = size;
        auto p = (u32*) header + 1;
        while (p < data)
            *p++ = header_pad_value;
    }

    inline static u0* data_pointer(alloc_header_t* header, u32 align) {
        u0* p = header + 1;
        u32 adjust{};
        return memory::system::align_forward(p, align, adjust);
    }

    static u32 fini(alloc_t* alloc) {
        return alloc->total_allocated;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        auto h = header(mem);
        return h->size;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        UNUSED(alloc);
        auto h = header(mem);
        const auto freed_size = h->size;
        std::free(h);
        return freed_size;
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        UNUSED(alloc);
        mem_result_t r{};
        r.size = size_with_padding(size, align);
        auto h = (alloc_header_t*) std::malloc(r.size);
        r.mem = data_pointer(h, align);
        fill(h, r.mem, r.size);
        return r;
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        UNUSED(alloc);
        alloc_header_t* h{};
        auto old_size = 0;
        if (mem) {
            h = header(mem);
            old_size = h->size;
        }
        mem_result_t r{};
        r.size = size_with_padding(size, align);
        h = (alloc_header_t*) std::realloc(h, r.size);
        r.mem = data_pointer(h, align);
        fill(h, r.mem, r.size);
        r.size = s32(r.size - old_size);
        return r;
    }

    alloc_system_t g_system{
        .size       = size,
        .init       = {},
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = realloc,
        .type       = alloc_type_t::default_,
    };

    alloc_system_t* system() {
        return &g_system;
    }
}
