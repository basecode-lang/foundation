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
#include <basecode/core/memory/system/buddy.h>

#define BIT_ARRAY_NUM_BITS                              (u32(8 * sizeof(u32)))
#define BIT_ARRAY_INDEX_SHIFT                           (u32(__builtin_ctzl(BIT_ARRAY_NUM_BITS)))
#define BIT_ARRAY_INDEX_MASK                            (BIT_ARRAY_NUM_BITS - 1UL)

#define BUDDY_ILOG2(value)                              (u32(BUDDY_NUM_BITS - 1UL) - __builtin_clz(value))
#define BUDDY_NUM_BITS                                  (u32(8 * sizeof(u32)))
#define BUDDY_MIN_LEAF_SIZE                             (sizeof(u0*) << 1UL)
#define BUDDY_LEAF_LEVEL_OFFSET                         (BUDDY_ILOG2(BUDDY_MIN_LEAF_SIZE))
#define BUDDY_MAX_LEVELS(total_size, min_size)          (BUDDY_ILOG2(total_size) - BUDDY_ILOG2(min_size))
#define BUDDY_MAX_INDEXES(total_size, min_size)         (1UL << (BUDDY_MAX_LEVELS(total_size, min_size) + 1))
#define BUDDY_BLOCK_INDEX_SIZE(total_size, min_size)    ((BUDDY_MAX_INDEXES(total_size, min_size) + (BUDDY_NUM_BITS - 1)) / BUDDY_NUM_BITS)
#define BUDDY_SIZEOF_METADATA(total_size, min_size)     ((sizeof(buddy_block_t) * (BUDDY_MAX_LEVELS(total_size, min_size) + 1)) + \
                                                        BUDDY_BLOCK_INDEX_SIZE(total_size, min_size) * (BUDDY_NUM_BITS >> 3U))

namespace basecode::memory::buddy {
    static inline u0 bit_array_set(u32* bit_array, u32 index) {
        bit_array[index >> BIT_ARRAY_INDEX_SHIFT] |= (1UL << (index & BIT_ARRAY_INDEX_MASK));
    }

    static inline u0 bit_array_not(u32* bit_array, u32 index) {
        u32 array_index = index >> BIT_ARRAY_INDEX_SHIFT;
        u32 bit_value   = (1UL << (index & BIT_ARRAY_INDEX_MASK));

        if (bit_array[array_index] & bit_value)
            bit_array[array_index] &= ~bit_value;
        else
            bit_array[array_index] |= bit_value;
    }

    static inline u0 bit_array_clear(u32* bit_array, u32 index) {
        bit_array[index >> BIT_ARRAY_INDEX_SHIFT] &= ~(1UL << (index & BIT_ARRAY_INDEX_MASK));
    }

    static inline b8 bit_array_is_set(const u32* bit_array, u32 index) {
        return (bit_array[index >> BIT_ARRAY_INDEX_SHIFT] & (1UL << (index & BIT_ARRAY_INDEX_MASK))) != 0;
    }

    static inline u0 list_init(buddy_block_t* list) {
        list->next = list;
        list->prev = list;
    }

    static inline b8 list_empty(buddy_block_t* list) {
        return list->next == list;
    }

    static inline u0 list_remove(buddy_block_t* list) {
        list->next->prev = list->prev;
        list->prev->next = list->next;
        list->next       = list;
        list->prev       = list;
    }

    static inline buddy_block_t* list_pop(buddy_block_t* list) {
        buddy_block_t* front = nullptr;
        if (!list_empty(list)) {
            front = list->next;
            list_remove(front);
        }
        return front;
    }

    static inline u32 free_index(const alloc_t* alloc, u32 index) {
        UNUSED(alloc);
        return (index - 1) >> 1U;
    }

    static inline u32 size_to_level(const alloc_t* alloc, size_t size) {
        auto sc  = &alloc->subclass.buddy;

        if (size < sc->min_allocation)
            return sc->max_level;

        size = 1U << (BUDDY_NUM_BITS - __builtin_clzl(size - 1));

        return sc->total_levels - BUDDY_ILOG2(size);
    }

    static inline u0 list_add(buddy_block_t* list, buddy_block_t* node) {
        node->next       = list;
        node->prev       = list->prev;
        list->prev->next = node;
        list->prev       = node;
    }

    static inline u32 index_of(const alloc_t* alloc, const u0* ptr, u32 level) {
        auto sc = &alloc->subclass.buddy;
        const usize offset = (u8*) ptr - (u8*) sc->heap;
        return (1UL << level) + (offset >> u32(sc->total_levels - level)) - 1UL;
    }

    static inline u32 split_index(const alloc_t* alloc, u32 index) {
        auto sc = &alloc->subclass.buddy;
        return index + (sc->max_indexes >> 1U);
    }

    static inline u0* to_buddy(const alloc_t* alloc, const u0* ptr, u32 level) {
        auto sc = &alloc->subclass.buddy;
        const size_t offset = (const u8*) ptr - (const u8*) sc->heap;
        return (offset ^ (sc->size >> level)) + (u8*) sc->heap;
    }

    static mem_result_t buddy_alloc_from_level(alloc_t* alloc, u32 level) {
        auto sc = &alloc->subclass.buddy;
        buddy_block_t* block_ptr        {};
        buddy_block_t* buddy_block_ptr;
        s32            block_at_level;
        s32            index;

        for (block_at_level = level; block_at_level >= 0; --block_at_level) {
            if (!list_empty(&sc->free_blocks[block_at_level])) {
                block_ptr = list_pop(&sc->free_blocks[block_at_level]);
                break;
            }
        }

        if (!block_ptr)
            return mem_result_t{};

        index = index_of(alloc, block_ptr, block_at_level);

        if (block_at_level != level) {
            while (block_at_level < level) {
                bit_array_set(sc->block_index, split_index(alloc, index));
                if (block_at_level > 0)
                    bit_array_not(sc->block_index, free_index(alloc, index));
                buddy_block_ptr = (buddy_block_t*) to_buddy(alloc, block_ptr, block_at_level + 1);
                list_add(&sc->free_blocks[block_at_level + 1], buddy_block_ptr);
                index = (u32(index) << 1U) + 1;
                ++block_at_level;
            }
        }

        mem_result_t r{};
        r.mem = block_ptr;

        if (level > 0) {
            bit_array_not(sc->block_index, free_index(alloc, index));
            r.size = 1U << (sc->total_levels - level);
        } else {
            r.size = sc->size;
        }

        return r;
    }

    static u32 buddy_release_at_level(alloc_t* alloc, u0* ptr, u32 level) {
        u0* buddy_ptr = to_buddy(alloc, ptr, level);
        auto sc    = &alloc->subclass.buddy;
        u32  index = index_of(alloc, ptr, level);
        u32  size;

        if (level > 0) {
            bit_array_not(sc->block_index, free_index(alloc, index));
            size = 1U << (sc->total_levels - level);
            assert(size <= alloc->total_allocated);
        } else {
            size = sc->size - sc->metadata_size;
        }

        while (level > 0
              && !bit_array_is_set(sc->block_index, free_index(alloc, index))) {
            bit_array_clear(sc->block_index, split_index(alloc, index));
            list_remove((buddy_block_t*) buddy_ptr);
            index = (index - 1) >> 1U;
            --level;
            if (buddy_ptr < ptr)
                ptr = buddy_ptr;
            buddy_ptr = to_buddy(alloc, ptr, level);
            if (level > 0)
                bit_array_not(sc->block_index, free_index(alloc, index));
        }

        bit_array_clear(sc->block_index, split_index(alloc, index));
        list_add(&sc->free_blocks[level], (buddy_block_t*) ptr);

        return size;
    }

    static u32 fini(alloc_t* alloc) {
        auto sc = &alloc->subclass.buddy;
        memory::free(alloc->backing, sc->heap);
        return sc->metadata_size;
    }

    static u32 free(alloc_t* alloc, u0* mem) {
        if (!mem)
            return {};
        auto sc = &alloc->subclass.buddy;
        auto index = index_of(alloc, mem, sc->max_level);
        for (u32 level = sc->max_level; level > 0; --level) {
            index = (index - 1) >> 1U;
            if (bit_array_is_set(sc->block_index, split_index(alloc, index)))
                return buddy_release_at_level(alloc, mem, level);
        }
        return buddy_release_at_level(alloc, mem, 0);
    }

    static u32 size(alloc_t* alloc, u0* mem) {
        if (!mem)
            return 0;
        auto sc = &alloc->subclass.buddy;
        auto index = index_of(alloc, mem, sc->max_level);
        for (u32 level = sc->max_level; level > 0; --level) {
            index = (index - 1) >> 1U;
            if (bit_array_is_set(sc->block_index, split_index(alloc, index))) {
                return 1U << (sc->total_levels - level);
            }
        }
        return sc->size;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto cfg = (buddy_config_t*) config;
        alloc->backing = cfg->backing;
        auto sc  = &alloc->subclass.buddy;
        sc->size = cfg->heap_size;

        u8* heap = (u8*) memory::alloc(alloc->backing,
                                       sc->size,
                                       alignof(buddy_block_t));
        sc->heap = heap;

        sc->extra_metadata = nullptr;
        sc->min_allocation = BUDDY_MIN_LEAF_SIZE;
        sc->total_levels   = BUDDY_ILOG2(sc->size);
        sc->max_indexes    = BUDDY_MAX_INDEXES(sc->size, sc->min_allocation);
        sc->max_level      = BUDDY_MAX_LEVELS(sc->size, sc->min_allocation);
        sc->metadata_size  = align(BUDDY_SIZEOF_METADATA(sc->size, BUDDY_MIN_LEAF_SIZE),
                                           alignof(buddy_block_t));
        u8* initial_metadata = (heap + sc->size) - sc->metadata_size;
        auto initial_free_blocks = (buddy_block_t*) initial_metadata;
        auto initial_block_index = (u32*) (initial_metadata + (sizeof(buddy_block_t) * (sc->max_level + 1)));
        sc->free_blocks = initial_free_blocks;
        sc->block_index = initial_block_index;

        for (u32 i = 0; i < sc->max_level + 1; ++i) {
            list_init(&sc->free_blocks[i]);
        }

        const auto max_indexes = (sc->max_indexes + (BUDDY_NUM_BITS - 1)) / BUDDY_NUM_BITS;
        for (u32 i = 0; i < max_indexes; ++i) {
            sc->block_index[i] = 0;
        }

        list_add(&sc->free_blocks[0], (buddy_block_t*) heap);

        const auto num_blocks = (sc->metadata_size + (BUDDY_MIN_LEAF_SIZE - 1)) / BUDDY_MIN_LEAF_SIZE;
        for (u32 i = 0; i < num_blocks; ++i) {
            const auto r = buddy_alloc_from_level(alloc, sc->max_level);
            alloc->total_allocated += r.size;
        }

        sc->free_blocks = (buddy_block_t*) heap;
        sc->block_index = (u32*) (heap + (sizeof(buddy_block_t) * (sc->max_level + 1)));

        for (u32 i = 0; i < max_indexes; ++i)
            sc->block_index[i] = initial_block_index[i];

        for (u32 level = 0; level < sc->max_level + 1; ++level) {
            list_init(&sc->free_blocks[level]);
            if (!list_empty(&initial_free_blocks[level])) {
                sc->free_blocks[level].next = initial_free_blocks[level].next;
                sc->free_blocks[level].prev = initial_free_blocks[level].prev;

                sc->free_blocks[level].next->prev = &sc->free_blocks[level];
                sc->free_blocks[level].prev->next = &sc->free_blocks[level];
            }
        }
    }

    static mem_result_t alloc(alloc_t* alloc, u32 size, u32 align) {
        UNUSED(align);
        return buddy_alloc_from_level(alloc, size_to_level(alloc, size));
    }

    static mem_result_t realloc(alloc_t* alloc, u0* mem, u32 size, u32 align) {
        buddy::free(alloc, mem);
        return buddy::alloc(alloc, size, align);
    }

    alloc_system_t g_system{
        .size       = size,
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

    u32 available(alloc_t* alloc) {
        u32 available = 0;
        u32 blocks_available;

        auto sc = &alloc->subclass.buddy;

        for (u32 level = 0; level < sc->max_level + 1; ++level) {
            blocks_available = 0;
            for (buddy_block_t* cursor = sc->free_blocks[level].next;
                    cursor != &sc->free_blocks[level];
                    cursor = cursor->next) {
                ++blocks_available;
            }
            available += blocks_available << (sc->total_levels - level);
        }

        return available;
    }

    u32 largest_available(alloc_t* alloc) {
        auto sc = &alloc->subclass.buddy;
        for (u32 level = 0; level < sc->max_level + 1; ++level)
            if (!list_empty(&sc->free_blocks[level]))
                return sc->size >> level;

        return 0;
    }
}
