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

#include <basecode/core/array.h>

namespace basecode {
    template <typename T>
    struct queue_t final {
        using Value_Type        = T;

        array_t<T>              items;
        u32                     size;
        u32                     offset;

        T& operator[](u32 index)                { return items[(index + offset) % items.size]; }
        const T& operator[](u32 index) const    { return items[(index + offset) % items.size]; }
    };
    static_assert(sizeof(queue_t<s32>) <= 32, "queue_t<T> is now larger than 32 bytes!");

    namespace queue {
        namespace internal {
            template <typename T>
            u0 increase_capacity(queue_t<T>& queue, u32 new_capacity) {
                u32 end = queue.items.size;
                array::resize(queue.items, new_capacity);
                if (queue.offset + queue.size > end) {
                    u32 end_items = end - queue.offset;
                    std::memmove(queue.items.begin() + new_capacity - end_items,
                                 queue.items.begin() + queue.offset,
                                 end_items * sizeof(T));
                    queue.offset += new_capacity - end;
                }
            }

            template <typename T>
            u0 grow(queue_t<T>& queue, u32 min_capacity = {}) {
                const auto new_capacity = std::max<u32>(queue.items.size * 2 + 8,
                                                        min_capacity);
                increase_capacity(queue, new_capacity);
            }
        }

        template <typename T>
        u0 free(queue_t<T>& queue) {
            array::free(queue.items);
            queue.size = queue.offset = {};
        }

        template <typename T>
        u0 clear(queue_t<T>& queue) {
            free(queue);
        }

        template <typename T>
        T* front(queue_t<T>& queue) {
            return queue.items.data + queue.offset;
        }

        template <typename T>
        u0 reset(queue_t<T>& queue) {
            array::reset(queue.items);
            queue.size = queue.offset = {};
        }

        template <typename T>
        u0 pop_back(queue_t<T>& queue) {
            if (queue.size == 0) return;
            --queue.size;
        }

        template <typename T>
        auto pop_front(queue_t<T>& queue) {
            if (queue.size == 0) return (T) {};
            auto value = (T) queue.items[queue.offset];
            queue.offset = (queue.offset + 1) % queue.items.size;
            --queue.size;
            return value;
        }

        template <typename T>
        u0 reserve(queue_t<T>& queue, u32 size) {
            if (size > queue.size)
                internal::increase_capacity(queue, size);
        }

        template <typename T>
        inline b8 empty(const queue_t<T>& queue) {
            return queue.size == 0;
        }

        template <typename T>
        u0 consume(queue_t<T>& queue, u32 count) {
            if (queue.size == 0 || queue.offset < count) return;
            queue.offset = (queue.offset + count) % queue.items.size;
            queue.offset -= count;
        }

        template <typename T>
        inline u32 space(const queue_t<T>& queue) {
            return queue.items.size - queue.size;
        }

        template <typename T>
        u0 push_back(queue_t<T>& queue, const T& value) {
            if (!space(queue))
                internal::grow(queue);
            queue[queue.size++] = value;
        }

        template <typename T>
        u0 push_front(queue_t<T>& queue, const T& value) {
            if (!space(queue))
                internal::grow(queue);
            queue.offset = (queue.offset - 1 + queue.items.size)
                           % queue.items.size;
            ++queue.size;
            queue[0] = value;
        }

        template <typename T>
        u0 push(queue_t<T>& queue, const T* items, u32 count) {
            if (space(queue) < count)
                internal::grow(queue, queue.size + count);
            const u32 size = queue.items.size;
            const u32 insert = (queue.offset + queue.size) % size;
            u32 to_insert = count;
            if (insert + to_insert > size)
                to_insert = size - insert;
            std::memcpy(queue.items.begin() + insert, items, to_insert * sizeof(T));
            queue.size += to_insert;
            items += to_insert;
            count -= to_insert;
            std::memcpy(queue.items.begin(), items, count * sizeof(T));
            queue.size += count;
        }

        template <typename T>
        u0 init(queue_t<T>& queue, alloc_t* alloc = context::top()->alloc.main) {
            array::init(queue.items, alloc);
            queue.size = queue.offset = {};
        }
    }
}
