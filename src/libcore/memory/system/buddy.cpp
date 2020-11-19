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

namespace basecode::memory::buddy {
    [[maybe_unused]] constexpr u32 min_alloc_log2   = 4;
    [[maybe_unused]] constexpr u32 min_alloc_size   = (1U << min_alloc_log2);

    struct list_t final {
        list_t*                 prev;
        list_t*                 next;
    };

    static u0* sbrk(alloc_t* alloc, intptr_t size) {
        const auto page_size = memory::system::os_page_size();
        auto sc = &alloc->subclass.buddy;
        if (size < 0) {
            // XXX: VirtualFree?
            assert(false);
        } else if (size > 0) {
            assert(sc->reserved >= size);

            if (size > sc->to_commit) {
                auto bytes_to_commit = u32(size - sc->to_commit + page_size - 1) & ~(page_size - 1);
                auto p = VirtualAlloc((LPVOID) sc->next_page,
                                      bytes_to_commit,
                                      MEM_COMMIT,
                                      PAGE_EXECUTE_READWRITE);
                if (!p)
                    return nullptr;

                sc->next_page += bytes_to_commit;
                sc->to_commit += bytes_to_commit;
            }

            auto p = sc->curr_ptr;
            sc->curr_ptr += size;
            sc->to_commit -= size;
            sc->reserved -= size;
            return p;
        }
        return sc->curr_ptr;
    }

    static s32 brk(alloc_t* alloc, u0* addr) {
        auto sc = &alloc->subclass.buddy;
        auto p = sbrk(alloc, ((u8*) addr - sc->max_ptr));
        return p ? 0 : - 1;
    }

    static u0 list_init(list_t* list) {
        list->prev = list;
        list->next = list;
    }

    static u0 list_remove(list_t* entry) {
        auto prev = entry->prev;
        auto next = entry->next;
        prev->next = next;
        next->prev = prev;
    }

    static list_t* list_pop(list_t* list) {
        auto back = list->prev;
        if (back == list) return nullptr;
        list_remove(back);
        return back;
    }

    static u0 list_push(list_t* list, list_t* entry) {
        auto prev = list->prev;
        entry->prev = prev;
        entry->next = list;
        prev->next = entry;
        list->prev = entry;
    }

    static b8 parent_is_split(alloc_t* alloc, u32 idx) {
        auto sc = &alloc->subclass.buddy;
        idx = (idx - 1) / 2;
        return u32(sc->node_state[idx / 8] >> (idx % 8)) & 1U;
    }

    static b8 update_max_ptr(alloc_t* alloc, u8* new_value) {
        auto sc = &alloc->subclass.buddy;
        if (new_value > sc->max_ptr) {
            if (brk(alloc, new_value))
                return false;
            sc->max_ptr = new_value;
        }
        return true;
    }

    static u0 flip_parent_is_split(alloc_t* alloc, u32 idx) {
        auto sc = &alloc->subclass.buddy;
        idx = (idx - 1) / 2;
        sc->node_state[idx / 8] ^= 1U << (idx % 8);
    }

    static u32 order_for_size(u32 size) {
        u32 order = 0;
        while ((1U << order) < size) ++order;
        return order;
    }

    static u32 bucket_for_request(alloc_t* alloc, u32 request) {
        auto sc = &alloc->subclass.buddy;
        u32 bucket = sc->num_buckets - 1;
        u32 size = min_alloc_size;

        while (size < request) {
            --bucket;
            size *= 2;
        }

        return bucket;
    }

    static u8* ptr_for_node(alloc_t* alloc, u32 idx, u32 bucket) {
        auto sc = &alloc->subclass.buddy;
        return sc->base_ptr + ((idx - (1U << bucket) + 1) << (sc->heap_order - bucket));
    }

    static u32 node_for_ptr(alloc_t* alloc, const u8* ptr, u32 bucket) {
        auto sc = &alloc->subclass.buddy;
        return (u64(ptr - sc->base_ptr) >> (sc->heap_order - bucket)) + (1U << bucket) - 1;
    }

    static b8 lower_bucket_limit(alloc_t* alloc, u32 bucket) {
        auto sc = &alloc->subclass.buddy;
        auto buckets = (list_t*) sc->buckets;
        while (bucket < sc->bucket_limit) {
            auto root = node_for_ptr(alloc, sc->base_ptr, sc->bucket_limit);

            if (!parent_is_split(alloc, root)) {
                list_remove((list_t*) sc->base_ptr);
                list_init(&buckets[--sc->bucket_limit]);
                list_push(&buckets[sc->bucket_limit], (list_t*) sc->base_ptr);
                continue;
            }

            auto right_child = ptr_for_node(alloc, root + 1, sc->bucket_limit);
            if (!update_max_ptr(alloc, right_child + sizeof(list_t)))
                return false;

            list_push(&buckets[sc->bucket_limit], (list_t*) right_child);
            list_init(&buckets[--sc->bucket_limit]);

            root = (root - 1) / 2;
            if (root != 0)
                flip_parent_is_split(alloc, root);
        }
        return true;
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        const auto alloc_granularity = memory::system::os_alloc_granularity();
        auto sc  = &alloc->subclass.buddy;
        auto cfg = (buddy_config_t*) config;
        alloc->backing = cfg->backing;
        sc->heap_size = align(cfg->heap_size, alloc_granularity);
        sc->reserved  = sc->heap_size;
        sc->heap_order = order_for_size(sc->heap_size);
        sc->to_commit = 0;
        sc->base_ptr = (u8*) VirtualAlloc(nullptr,
                                          sc->reserved,
                                          MEM_RESERVE,
                                          PAGE_NOACCESS);
        sc->curr_ptr = sc->max_ptr = sc->next_page = sc->base_ptr;

        sc->num_buckets = (sc->heap_order - min_alloc_log2) + 1;
        const auto node_state_size = (1U << (sc->num_buckets - 1)) / 8;
        sc->node_state = (u8*) memory::alloc(alloc->backing, node_state_size);
        sc->node_order = (u8*) memory::alloc(alloc->backing, (1U << (sc->num_buckets - 1)));
        sc->buckets = memory::alloc(alloc->backing,
                                    sc->num_buckets * sizeof(list_t),
                                    alignof(list_t));
        auto buckets = (list_t*) sc->buckets;
        sc->bucket_limit = sc->num_buckets - 1;
        assert(update_max_ptr(alloc, sc->base_ptr + sizeof(list_t)));
        list_init(&buckets[sc->num_buckets - 1]);
        list_push(&buckets[sc->num_buckets - 1], (list_t*) sc->base_ptr);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto sc  = &alloc->subclass.buddy;
        auto buckets = (list_t*) sc->buckets;

        if (!mem) {
            freed_size = 0;
            return;
        }

        auto bucket = bucket_for_request(alloc, freed_size);
        auto i = node_for_ptr(alloc, (u8*) mem, bucket);

        while (i != 0) {
            flip_parent_is_split(alloc, i);

            if (parent_is_split(alloc, i) || bucket == sc->bucket_limit)
                break;

            list_remove((list_t*) ptr_for_node(alloc, ((i - 1) ^ 1U) + 1, bucket));
            i = (i - 1) / 2;
            --bucket;
        }

        alloc->total_allocated -= freed_size;
        list_push(&buckets[bucket], (list_t*) ptr_for_node(alloc, i, bucket));
    }

    static u0 fini(alloc_t* alloc, b8 enforce, u32* freed_size) {
        u32 temp_freed{};
        u32 total_freed{};
        auto sc  = &alloc->subclass.buddy;
        assert(VirtualFree(sc->base_ptr, 0, MEM_RELEASE));
        sc->base_ptr = sc->curr_ptr = sc->next_page = sc->max_ptr = {};
        sc->reserved = sc->to_commit = {};
        memory::free(alloc->backing, sc->node_state, &temp_freed);
        total_freed += temp_freed;
        memory::free(alloc->backing, sc->buckets, &temp_freed);
        total_freed += temp_freed;
        memory::free(alloc->backing, sc->node_order, &temp_freed);
        total_freed += temp_freed;
        if (freed_size) *freed_size = total_freed;
        if (enforce) assert(alloc->total_allocated == 0);
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& alloc_size) {
        auto sc  = &alloc->subclass.buddy;
        auto buckets = (list_t*) sc->buckets;

        if (size > sc->heap_size) {
            alloc_size = 0;
            return nullptr;
        }

        auto bucket = bucket_for_request(alloc, size);
        auto original_bucket = bucket;

        while (bucket + 1 != 0) {
            if (!lower_bucket_limit(alloc, bucket)) {
                alloc_size = 0;
                return nullptr;
            }

            auto ptr = (u8*) list_pop(&buckets[bucket]);
            if (!ptr) {
                if (bucket != sc->bucket_limit) {
                    --bucket;
                    continue;
                }

                if (!lower_bucket_limit(alloc, bucket - 1)) {
                    alloc_size = 0;
                    return nullptr;
                }

                ptr = (u8*) list_pop(&buckets[bucket]);
                if (!ptr) {
                    alloc_size = 0;
                    return nullptr;
                }
            }

            auto bucket_size  = 1U << (sc->heap_order - bucket);
            auto bytes_needed = bucket < original_bucket ? bucket_size / 2 + sizeof(list_t) : bucket_size;
            if (!update_max_ptr(alloc, ptr + bytes_needed)) {
                list_push(&buckets[bucket], (list_t*) ptr);
                alloc_size = 0;
                return nullptr;
            }

            auto i = node_for_ptr(alloc, ptr, bucket);
            if (i != 0) {
                flip_parent_is_split(alloc, i);
            }

            while (bucket < original_bucket) {
                i = i * 2 + 1;
                ++bucket;
                flip_parent_is_split(alloc, i);
                list_push(&buckets[bucket],
                          (list_t*) ptr_for_node(alloc, i + 1, bucket));
            }

            alloc_size = size;
            alloc->total_allocated += size;
            return ptr;
        }

        return nullptr;
    }

    static u0* realloc(alloc_t* alloc, u0* mem, u32 size, u32 align, u32& old_size) {
        return nullptr;
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
}
