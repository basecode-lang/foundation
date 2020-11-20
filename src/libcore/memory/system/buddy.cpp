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

#define HEAP_GROW(h)                    ((h) << 1U)
#define HEAP_SHRINK(h)                  ((h) >> 1U)
#define LEFT_CHILD(idx)                 ((u32(idx) << 1U) + 1)
#define RIGHT_CHILD(idx)                ((u32(idx) << 1U) + 2)
#define BUDDY_NODE(idx)                 ((idx) - 1 + ((u32(idx) & 1U) << 1U))
#define PARENT_NODE(idx)                ((u32((idx) + 1) >> 1U) - 1)        // N.B. (idx + 1) / 2 - 1
#define INDEX_OFFSET(idx, lvl, mlvl)    ((((idx) + 1) - (1U << u32(lvl))) << ((mlvl) - (lvl)))

namespace basecode::memory::buddy {
    static constexpr u8 node_unused = 0;
    static constexpr u8 node_used   = 1;
    static constexpr u8 node_split  = 2;
    static constexpr u8 node_full   = 3;

    static u0 dump_node(alloc_t* alloc, s32 index, s32 level) {
        auto sc = &alloc->subclass.buddy;
        switch (sc->heap[index]) {
            case node_unused:
                format::print("<{}:{}>",
                              INDEX_OFFSET(index, level, sc->max_level),
                              1U << (sc->max_level - level));
                break;
            case node_used:
                format::print("[{}:{}]",
                              INDEX_OFFSET(index, level, sc->max_level),
                              1U << (sc->max_level - level));
                break;
            case node_full:
                format::print("{{");
                dump_node(alloc, LEFT_CHILD(index), level + 1);
                dump_node(alloc, RIGHT_CHILD(index), level + 1);
                format::print("}}");
                break;
            default:
                format::print("(");
                dump_node(alloc, LEFT_CHILD(index), level + 1);
                dump_node(alloc, RIGHT_CHILD(index), level + 1);
                format::print(")");
                break;
        }
    }

    u0 dump(alloc_t* alloc) {
        dump_node(alloc, 0, 0);
        format::print("\n");
    }

    static u32 level_for_size(u32 size) {
        u32 level = 0;
        while ((1U << level) < size) ++level;
        return level;
    }

    static u0 combine(alloc_t* alloc, s32 index) {
        auto sc = &alloc->subclass.buddy;
        while (true) {
            s32 buddy = BUDDY_NODE(index);
            if (buddy < 0
            ||  sc->heap[buddy] != node_unused) {
                sc->heap[index] = node_unused;
                while (((index = PARENT_NODE(index)) >= 0)
                       && sc->heap[index] == node_full) {
                    sc->heap[index] = node_split;
                }
                return;
            }
            index = PARENT_NODE(index);
        }
    }

    static u0 mark_parent(alloc_t* alloc, s32 index) {
        auto sc = &alloc->subclass.buddy;
        while (true) {
            s32 buddy = BUDDY_NODE(index);
            if (buddy > 0
            && (sc->heap[buddy] == node_used || sc->heap[buddy] == node_full)) {
                index = PARENT_NODE(index);
                sc->heap[index] = node_full;
            } else {
                return;
            }
        }
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (buddy_config_t*) config;
        alloc->backing = cfg->backing;

        auto sc  = &alloc->subclass.buddy;
        sc->heap_size = is_power_of_two(cfg->heap_size) ? cfg->heap_size :
                        next_power_of_two(cfg->heap_size);
        sc->heap = (u8*) memory::alloc(alloc->backing, sc->heap_size << 1U);
        sc->max_level = level_for_size(sc->heap_size);
        std::memset(sc->heap, 0, sc->heap_size << 1U);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto sc = &alloc->subclass.buddy;

        auto offset = (u8*) mem - (sc->heap + sc->heap_size);
        u32 left      = 0;
        u32 heap_size = sc->heap_size;
        u32 index     = 0;

        while (true) {
            switch (sc->heap[index]) {
                case node_used:
                    assert(offset == left);
                    combine(alloc, index);
                    freed_size = heap_size;
                    alloc->total_allocated -= heap_size;
                    return;
                case node_unused:
                    assert(false);
                    return;
                default:
                    heap_size = HEAP_SHRINK(heap_size);
                    if (offset < left + heap_size) {
                        index = LEFT_CHILD(index);
                    } else {
                        left += heap_size;
                        index = RIGHT_CHILD(index);
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

        auto sc = &alloc->subclass.buddy;
        u32 heap_size = sc->heap_size;
        alloc_size = 0;

        if (size == 0)
            size = 1;
        else {
            size = is_power_of_two(size) ? size : next_power_of_two(size);
        }

        if (size > heap_size)
            return nullptr;

        s32 index = 0;
        u32 level = 0;

        while (index >= 0) {
            if (size == heap_size) {
                if (sc->heap[index] == node_unused) {
                    sc->heap[index] = node_used;
                    mark_parent(alloc, index);
                    alloc_size = size;
                    alloc->total_allocated += size;
                    return (sc->heap + sc->heap_size) + INDEX_OFFSET(index, level, sc->max_level);
                }
            } else {
                switch (sc->heap[index]) {
                    case node_used:
                    case node_full:
                        break;
                    case node_unused:
                        sc->heap[index]              = node_split;
                        sc->heap[LEFT_CHILD(index)]  = node_unused;
                        sc->heap[RIGHT_CHILD(index)] = node_unused;
                    default:
                        index = LEFT_CHILD(index);
                        heap_size = HEAP_SHRINK(heap_size);
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
                heap_size = HEAP_GROW(heap_size);
                index = PARENT_NODE(index);
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
        auto sc     = &alloc->subclass.buddy;
        u32  left   = 0;
        u32 index     = 0;
        u32 heap_size = sc->heap_size;
        u32 offset    = (u8*) mem - (sc->heap + sc->heap_size);

        while (true) {
            switch (sc->heap[index]) {
                case node_used:
                    assert(offset == left);
                    return heap_size;
                case node_unused:
                    assert(false);
                    return heap_size;
                default:
                    heap_size = HEAP_SHRINK(heap_size);
                    if (offset < left + heap_size) {
                        index = LEFT_CHILD(index);
                    } else {
                        left += heap_size;
                        index = RIGHT_CHILD(index);
                    }
                    break;
            }
        }
    }
}
