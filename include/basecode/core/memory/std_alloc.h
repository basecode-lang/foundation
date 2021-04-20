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

namespace basecode {
    struct alloc_t;

    namespace memory::std_alloc {
        u0  free(alloc_t* alloc, u0* mem, u32 size);

        u0* alloc(alloc_t* alloc, u32 size, u32 align);
    }

    template<class T> class std_alloc_t {
    public:
        using value_type        = T;

        alloc_t*                backing{};

        std_alloc_t(const std_alloc_t<T>& other) : backing(other.backing) {
            BC_ASSERT_NOT_NULL(backing);
        }

        template<class U>
        explicit std_alloc_t(std_alloc_t<U> const& other) noexcept
            : backing(other.backing) {
            BC_ASSERT_NOT_NULL(backing);
        }

        explicit std_alloc_t(alloc_t* alloc = context::top()->alloc.main) noexcept
            : backing(alloc) {
            BC_ASSERT_NOT_NULL(backing);
        }

        value_type* allocate(std::size_t n) {
            return (value_type*) memory::std_alloc::alloc(backing,
                                                          n * sizeof(value_type),
                                                          alignof(value_type));
        }

        u0 deallocate(value_type* mem, std::size_t size) noexcept {
            memory::std_alloc::free(backing, mem, size);
        }
    };

    template<class T, class U>
    b8 operator==(std_alloc_t<T> const& x, std_alloc_t<U> const& y) noexcept {
        return x.backing == y.backing;
    }

    template<class T, class U>
    b8 operator!=(std_alloc_t<T> const& x, std_alloc_t<U> const& y) noexcept {
        return !(x == y);
    }
}
