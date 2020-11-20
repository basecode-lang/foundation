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

#include <basecode/core/bits.h>
#include <basecode/core/format.h>
#include <basecode/core/memory/system/buddy.h>

namespace basecode::memory::buddy {
    struct buddy_t final {
        u32                     level;
        u8                      tree[1];
    };

    static constexpr u8 node_unused = 0;
    static constexpr u8 node_used   = 1;
    static constexpr u8 node_split  = 2;
    static constexpr u8 node_full   = 3;

    static inline u32 index_offset(u32 index, u32 level, u32 max_level) {
        return ((index + 1) - (1U << level)) << (max_level - level);
    }

    static u0 dump_node(buddy_t* b, s32 index, s32 level) {
        switch (b->tree[index]) {
            case node_unused:
                format::print("<{}:{}>",
                              index_offset(index, level, b->level),
                              1U << (b->level - level));
                break;
            case node_used:
                format::print("[{}:{}]",
                              index_offset(index, level, b->level),
                              1U << (b->level - level));
                break;
            case node_full:
                format::print("{{");
                dump_node(b, index * 2 + 1, level + 1);
                dump_node(b, index * 2 + 2, level + 1);
                format::print("}}");
                break;
            default:
                format::print("(");
                dump_node(b, index * 2 + 1, level + 1);
                dump_node(b, index * 2 + 2, level + 1);
                format::print(")");
                break;
        }
    }

    u0 dump(alloc_t* alloc) {
        auto sc = &alloc->subclass.buddy;
        dump_node((buddy_t*) sc->heap, 0, 0);
        format::print("\n");
    }

    static u32 level_for_size(u32 size) {
        u32 level = 0;
        while ((1U << level) < size) ++level;
        return level;
    }

    static u0 combine(buddy_t* b, s32 index) {
        while (true) {
            s32 buddy = index - 1 + (u32(index) & 1U) * 2;
            if (buddy < 0
            ||  b->tree[buddy] != node_unused) {
                b->tree[index] = node_unused;
                while (((index = (index + 1) / 2 - 1) >= 0)
                       && b->tree[index] == node_full) {
                    b->tree[index] = node_split;
                }
                return;
            }
            index = (index + 1) / 2 - 1;
        }
    }

    static u0 mark_parent(buddy_t* b, s32 index) {
        while (true) {
            s32 buddy = index - 1 + (u32(index) & 1U) * 2;
            if (buddy > 0
            && (b->tree[buddy] == node_used || b->tree[buddy] == node_full)) {
                index = (index + 1) / 2 - 1;
                b->tree[index] = node_full;
            } else {
                return;
            }
        }
    }

    static u32 buddy_size(buddy_t* b, u32 offset) {
        u32 left   = 0;
        u32 index  = 0;
        u32 length = 1U << b->level;

        while (true) {
            switch (b->tree[index]) {
                case node_used:
                    assert(offset == left);
                    return length;
                case node_unused:
                    assert(false);
                    return length;
                default:
                    length /= 2;
                    if (offset < left + length) {
                        index = index * 2 + 1;
                    } else {
                        left += length;
                        index = index * 2 + 2;
                    }
                    break;
            }
        }
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (buddy_config_t*) config;
        alloc->backing = cfg->backing;

        auto sc  = &alloc->subclass.buddy;
        sc->heap_size = is_power_of_two(cfg->heap_size) ? cfg->heap_size :
                        next_power_of_two(cfg->heap_size);
        const auto heap_size = sizeof(buddy_t) + (sc->heap_size * 2 - 2);
        sc->heap = (u8*) memory::alloc(alloc->backing,
                                       heap_size,
                                       alignof(buddy_t));

        std::memset(sc->heap, 0, heap_size);
        auto buddy = (buddy_t*) sc->heap;
        buddy->level = level_for_size(sc->heap_size);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto sc = &alloc->subclass.buddy;
        auto b  = (buddy_t*) sc->heap;

        auto offset = (u8*) mem - (b->tree + sc->heap_size);
        u32 left = 0;
        u32 length = 1U << b->level;
        u32 index = 0;

        while (true) {
            switch (b->tree[index]) {
                case node_used:
                    assert(offset == left);
                    combine(b, index);
                    freed_size = length;
                    alloc->total_allocated -= length;
                    return;
                case node_unused:
                    assert(false);
                    return;
                default:
                    length /= 2;
                    if (offset < left + length) {
                        index = index * 2 + 1;
                    } else {
                        left += length;
                        index = index * 2 + 2;
                    }
                    break;
            }
        }
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        u32 temp_freed{};
        u32 total_freed{};

        auto sc  = &alloc->subclass.buddy;
        memory::free(alloc->backing, sc->heap, &temp_freed);
        total_freed += temp_freed;

        if (freed_size) *freed_size = total_freed;
        if (enforce) assert(alloc->total_allocated == 0);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        UNUSED(align);

        auto sc     = &alloc->subclass.buddy;
        auto b      = (buddy_t*) sc->heap;
        u32  length = 1U << b->level;
        alloc_size = 0;

        if (size == 0)
            size = 1;
        else {
            size = is_power_of_two(size) ? size : next_power_of_two(size);
        }

        if (size > length)
            return nullptr;

        s32 index = 0;
        u32 level = 0;

        while (index >= 0) {
            if (size == length) {
                if (b->tree[index] == node_unused) {
                    b->tree[index] = node_used;
                    mark_parent(b, index);
                    alloc_size = size;
                    alloc->total_allocated += size;
                    return (b->tree + sc->heap_size) + index_offset(index, level, b->level);
                }
            } else {
                switch (b->tree[index]) {
                    case node_used:
                    case node_full:
                        break;
                    case node_unused:
                        b->tree[index]         = node_split;
                        b->tree[index * 2 + 1] = node_unused;
                        b->tree[index * 2 + 2] = node_unused;
                    default:
                        index = index * 2 + 1;
                        length /= 2;
                        level++;
                        continue;
                }
            }
            if (u32(index) & 1U) {
                ++index;
                continue;
            }
            while (true) {
                level--;
                length *= 2;
                index = (index + 1) / 2 - 1;
                if (index < 0)
                    return nullptr;
                if (u32(index) & 1U) {
                    ++index;
                    break;
                }
            }
        }

        return nullptr;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size) {
        buddy::free(alloc, mem, old_size);
        u32 alloc_size{};
        return buddy::alloc(alloc, size, align, alloc_size);
    }

    alloc_system_t g_system{
        .init       = init,
        .fini       = fini,
        .free       = free,
        .alloc      = alloc,
        .realloc    = realloc,
        .type       = alloc_type_t::buddy,
    };

    alloc_system_t* system() {
        return &g_system;
    }

    u32 allocated_size(alloc_t* alloc, u0* mem) {
        auto sc  = &alloc->subclass.buddy;
        auto b   = (buddy_t*) sc->heap;
        return buddy_size(b, (u8*) mem - (b->tree + sc->heap_size));
    }
}
