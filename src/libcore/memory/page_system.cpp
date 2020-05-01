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

#include <cassert>
#include "page_system.h"

namespace basecode::memory::page {
    static u0 release(alloc_t* alloc) {
        u32 freed_size{};
        auto backing = alloc->backing->system;
        auto subclass = &alloc->subclass.page;
        auto curr_page = (u0*) subclass->head;
        while (curr_page) {
            auto prev_page = ((page_header_t*) curr_page)->prev;
            backing->free(alloc->backing, curr_page, freed_size);
            alloc->total_allocated -= freed_size;
            curr_page = prev_page;
        }
        assert(alloc->total_allocated == 0);
    }

    static u0 init(alloc_t* alloc, alloc_config_t* config) {
        auto page_config = (page_config_t*) config;
        auto subclass = &alloc->subclass.page;
        subclass->count = {};
        alloc->backing = page_config->backing;
        subclass->cursor = subclass->tail = subclass->head = {};
        if (page_config->page_size < memory::system::os_page_size()) {
            page_config->page_size = memory::system::os_page_size();
        } else if (!is_power_of_two(page_config->page_size)) {
            page_config->page_size = next_power_of_two(page_config->page_size);
        }
        subclass->size = page_config->page_size - (alignof(page_header_t) + 4);
        assert(alloc->backing);
    }

    static u0 free(alloc_t* alloc, u0* mem, u32& freed_size) {
        auto subclass = &alloc->subclass.page;
        auto buf = (u8*) mem - sizeof(page_header_t);
        auto page = (page_header_t*) buf;
        if (page->next) ((page_header_t*) page->next)->prev = page->prev;
        if (page->prev) ((page_header_t*) page->prev)->next = page->next;
        if (subclass->head == buf) subclass->head = page->next;
        if (subclass->tail == buf) subclass->tail = page->prev;
        alloc->backing->system->free(alloc->backing, buf, freed_size);
        alloc->total_allocated -= freed_size;
    }

    static u0* alloc(alloc_t* alloc, u32 size, u32 align, u32& allocated_size) {
        auto subclass = &alloc->subclass.page;
        auto backing = alloc->backing;
        u8* page{};
        if (subclass->cursor) {
            page = (u8*) subclass->cursor;
            subclass->cursor = ((page_header_t*) subclass->cursor)->next;
        }
        if (!page) {
            page = (u8*) backing->system->alloc(backing, subclass->size, alignof(page_header_t), allocated_size);
            subclass->count++;
            alloc->total_allocated += allocated_size;
            if (!subclass->tail)
                subclass->tail = page;
            auto page_header = (page_header_t*) page;
            if (subclass->head) {
                page_header->prev = subclass->head;
                ((page_header_t*) subclass->head)->next = page;
            } else {
                page_header->prev = page_header->next = {};
            }
            subclass->head = page;
        }
        allocated_size = subclass->size;
        return page + sizeof(page_header_t);
    }

    alloc_system_t g_system{
        .init       = init,
        .type       = alloc_type_t::page,
        .free       = free,
        .alloc      = alloc,
        .release    = release,
    };

    u0 reset(alloc_t* alloc) {
        auto a = unwrap(alloc);
        assert(a && a->system->type == alloc_type_t::page);
        auto subclass = &a->subclass.page;
        subclass->cursor = subclass->tail;
    }

    alloc_system_t* system() {
        return &g_system;
    }
}
