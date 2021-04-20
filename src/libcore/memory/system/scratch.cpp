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

#include <basecode/core/assert.h>
#include <basecode/core/memory/system/scratch.h>

namespace basecode::memory::scratch {
    constexpr u32 pad_value     = 0x7FFF0000u;

    struct alloc_header_t final {
        u32                     free:   1;
        u32                     size:   31;
    };

    inline static alloc_header_t* header(u0* data) {
        auto p = (alloc_header_t*) data;
        while (true) {
            auto t = p - 1;
            if (t->size != pad_value)
                break;
            --p;
        }
        return p - 1;
    }

    inline static u32 size_with_padding(u32 size, u32 align) {
        return size + align + sizeof(alloc_header_t);
    }

    inline static u0 fill(alloc_header_t* header, u0* data, u32 size) {
        header->size = size;
        auto p = (alloc_header_t*) header + 1;
        while (p < data) {
            p->free = false;
            p->size = pad_value;
            ++p;
        }
    }

    inline static u0* data_pointer(alloc_header_t* header, u32 align) {
        u0* p = header + 1;
        u32 adjust{};
        return memory::system::align_forward(p, align, adjust);
    }

    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.scratch;
        memory::internal::free(alloc->backing, sc->begin);
        return alloc->total_allocated;
    }

    static b8 in_use(alloc_t* alloc, u0* p) {
        auto sc = &alloc->subclass.scratch;
        if (sc->free == sc->alloc)
            return false;
        if (sc->alloc > sc->free)
            return p >= sc->free && p < sc->alloc;
        return p >= sc->free || p < sc->alloc;
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        auto h = header(mem);
        return h->size - ((u8*) mem - (u8*) h);
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        auto sc = &alloc->subclass.scratch;
        auto h = header(mem);
        BC_ASSERT_MSG(!h->free, "slot is already free");
        auto freed_size = h->size;
        h->free = true;
        while (sc->free != sc->alloc) {
            h = (alloc_header_t*) sc->free;
            if (!h->free)
                break;
            sc->free += h->size;
            if (sc->free == sc->end)
                sc->free = sc->begin;
        }
        return freed_size;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto sc  = &alloc->subclass.scratch;
        auto cfg = (scratch_config_t*) config;
        alloc->backing = cfg->backing;
        sc->size = cfg->buf_size;
        const auto r  = memory::internal::alloc(alloc->backing,
                                                sc->size,
                                                sizeof(u64));
        sc->begin = (u8*) r.mem;
        sc->end   = sc->begin + sc->size;
        sc->alloc = sc->begin;
        sc->free  = sc->begin;
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        auto sc = &alloc->subclass.scratch;
        mem_result_t    r       {};
        u32             adjust  {};

        size = ((size + (align - 1)) / align) * align;

        auto p = sc->alloc;
        auto h = (alloc_header_t*) p;
        auto data = (u8*) data_pointer(h, align);
        p = data + size;

        if (p > sc->end) {
            h->free = true;
            h->size = (sc->end - (u8*)h);

            p = sc->begin;
            h = (alloc_header_t*) p;
            data = (u8*) data_pointer(h, align);
            p = data + size;
        }

        BC_ASSERT_MSG(!in_use(alloc, p), "scratch allocator exhausted");

        fill(h, data, p - (u8*) h);
        h->free   = false;
        sc->alloc = p;
        r.mem     = data;
        r.size    = h->size;
        return r;
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        mem_result_t r          {};
        u32          old_size   {};
        if (mem) {
            old_size = scratch::free(alloc, mem);
            r = scratch::alloc(alloc, size, align);
            r.size = s32(r.size - old_size);
        } else {
            r = scratch::alloc(alloc, size, align);
        }
        return r;
    }

    alloc_system_t g_alloc_system{
        .size       = size,
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = realloc,
        .type       = alloc_type_t::scratch,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        BC_ASSERT_MSG(a && a->system->type == alloc_type_t::scratch,
                      "expected a non-null scratch allocator");
        auto sc   = &a->subclass.scratch;
        sc->end   = sc->begin + sc->size;
        sc->alloc = sc->begin;
        sc->free  = sc->begin;
    }

    alloc_system_t* system() {
        return &g_alloc_system;
    }
}
