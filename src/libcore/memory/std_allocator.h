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

#include <cassert>
#include <basecode/core/context.h>
#include "memory.h"

namespace basecode::memory {
    template<class T>
    class std_allocator_t {
    public:
        using value_type = T;

        alloc_t* backing{};

        explicit std_allocator_t(alloc_t* allocator = context::top()->alloc) noexcept : backing(allocator) {
            assert(backing);
        }

        std_allocator_t(const std_allocator_t<T>& other) : backing(other.backing) {
            assert(backing);
        }

        template<class U>
        explicit std_allocator_t(std_allocator_t<U> const& other) noexcept : backing(other.backing) {
            assert(backing);
        }

        value_type* allocate(std::size_t n) {
            return (value_type*) memory::alloc(backing, n * sizeof(value_type), alignof(value_type));
        }

        void deallocate(value_type* mem, std::size_t) noexcept {
            memory::free(backing, mem);
        }
    };

    template<class T, class U>
    bool operator==(std_allocator_t<T> const& x, std_allocator_t<U> const& y) noexcept {
        return x.backing == y.backing;
    }

    template<class T, class U>
    bool operator!=(std_allocator_t<T> const& x, std_allocator_t<U> const& y) noexcept {
        return !(x == y);
    }

}
