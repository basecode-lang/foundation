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
#include <cstring>
#include <algorithm>
#include <foundation/types.h>
#include <foundation/memory/system.h>
#include "slice.h"

namespace basecode::array {
    template<typename T>
    struct array_t final {
        explicit array_t(memory::allocator_t* allocator = context::current()->allocator);

        ~array_t();

        array_t(const array_t<T>& other);

        array_t(array_t<T>&& other) noexcept;

        T* data{};
        u32 size{};
        u32 capacity{};
        memory::allocator_t* allocator{};

        T* end() { return data + size; }

        T* rend() { return data; }

        T* begin() { return data; }

        T* rbegin() { return data + size; }

        const T* end() const { return data + size; }

        const T* rend() const { return data; }

        const T* begin() const { return data; }

        const T* rbegin() const { return data + size; }

        T& operator[](u32 index);

        const T& operator[](u32 index) const;

        array_t<T>& operator=(const array_t<T>& other);

        array_t<T>& operator=(array_t&& other) noexcept;
    };

    template <typename T> u0 clear(array_t<T>&);
    template <typename T> u0 append(array_t<T>&, T&&);
    template <typename T> u0 append(array_t<T>&, const T&);
    template <typename T> u0 reserve(array_t<T>&, u32, b8 copy = true);

    ///////////////////////////////////////////////////////////////////////////

    template <typename T> u0 grow(
            array_t<T>& array,
            u32 new_capacity = 0) {
        new_capacity = std::max(new_capacity, array.capacity);
        reserve(array, new_capacity * 2 + 8);
    }

    template <typename T> u0 reserve(
            array_t<T>& array,
            u32 new_capacity,
            b8 copy) {
        if (array.capacity == new_capacity)
            return;

        if (new_capacity < array.size)
            new_capacity = array.size;

        T* new_data{};
        if (new_capacity > 0) {
            new_data = (T*) memory::allocate(
                array.allocator,
                new_capacity * sizeof(T),
                alignof(T));
            std::memset(new_data, 0, new_capacity * sizeof(T));
            if (array.data && copy) {
                std::memcpy(
                    new_data,
                    array.data,
                    array.size * sizeof(T));
            }
        }

        memory::deallocate(array.allocator, array.data);
        array.data = new_data;
        array.capacity = new_capacity;
    }

    template <typename T> T* prepare_insert(array_t<T>& array, const T* it) {
        const auto offset = it - array.data;
        if (array.size == array.capacity)
            reserve(array, array.size + 1);
        if (offset < array.size) {
            auto count = array.size - offset;
            std::memmove(
                array.data + offset + 1,
                array.data + offset,
                count * sizeof(T));
        }
        return offset;
    }

    template <typename T> u0 init(
            array_t<T>* array,
            memory::allocator_t* allocator = context::current()->allocator) {
        array->data = {};
        array->allocator = allocator;
        array->size = array->capacity = {};
    }

    template <typename T> array_t<T> make(
            std::initializer_list<T> elements,
            memory::allocator_t* allocator = context::current()->allocator) {
            array_t<T> array(allocator);
        for (auto&& e : elements)
            append(array, e);
        return array;
    }

    template <typename T> u0 pop(array_t<T>& array) {
        --array.size;
    }

    template <typename T> u0 free(array_t<T>& array) {
        clear(array);
    }

    template <typename T> u0 trim(array_t<T>& array) {
        reserve(array, array.size);
    }

    template <typename T> T& back(array_t<T>& array) {
        return array.data[array.size - 1];
    }

    template <typename T> u0 reset(array_t<T>& array) {
        array.size = {};
        std::memset(array.data, 0, sizeof(T) * array.capacity);
    }

    template <typename T> u0 clear(array_t<T>& array) {
        memory::deallocate(array.allocator, array.data);
        array.data = {};
        array.size = array.capacity = {};
    }

    template <typename T> b8 empty(array_t<T>& array) {
        return array.size == 0;
    }

    template <typename T> T& front(array_t<T>& array) {
        return array.data[0];
    }

    template <typename T> const T& back(array_t<T>& array) {
        return array.data[array.size - 1];
    }

    template <typename T> const T& front(array_t<T>& array) {
        return array.data[0];
    }

    template <typename T> T& emplace(array_t<T>& array) {
        if (array.size + 1 > array.capacity)
            grow(array);
        new (array.data + array.size) T();
        ++array.size;
        return array.data[array.size - 1];
    }

    template <typename T> u0 append(array_t<T>& array, T&& value) {
        if (array.size + 1 > array.capacity)
            grow(array);
        array.data[array.size++] = value;
    }

    template <typename T> u0 resize(array_t<T>& array, u32 new_size) {
        if (new_size > array.capacity)
            grow(array, new_size);
        array.size = new_size;
    }

    template <typename T> u0 append(array_t<T>& array, const T& value) {
        if (array.size + 1 > array.capacity)
            grow(array);
        array.data[array.size++] = value;
    }

    template <typename T> b8 contains(array_t<T>& array, const T& value) {
        const T* data = array.data;
        const T* data_end = array.data + array.size;
        while (data < data_end) {
            if (*data++ == value)
                return true;
        }
        return false;
    }

    template <typename T> T* insert(array_t<T>& array, const T* it, T&& value) {
        const auto offset = prepare_insert(array, it);
        array.data[offset] = value;
        ++array.size;
        return array.data + offset;
    }

    template <typename T> array_t<T> make(memory::allocator_t* allocator = context::current()->allocator) {
        return array_t<T>(allocator);
    }

    ///////////////////////////////////////////////////////////////////////////

    template<typename T> inline array_t<T>::array_t(memory::allocator_t* allocator) : allocator(allocator) {
        assert(allocator);
    }

    template<typename T> inline array_t<T>::~array_t() {
        array::clear(*this);
    }

    template<typename T> inline array_t<T>::array_t(const array_t<T>& other) {
        operator=(other);
    }

    template<typename T> inline array_t<T>::array_t(array_t<T>&& other) noexcept {
        operator=(other);
    }

    template<typename T> T& array_t<T>::operator[](u32 index) {
        return data[index];
    }

    template<typename T> const T& array_t<T>::operator[](u32 index) const {
        return data[index];
    }

    template<typename T> array_t<T>& array_t<T>::operator=(const array_t<T>& other) {
        if (this == &other)
            return *this;
        if (!allocator)
            allocator = other.allocator;
        reserve(*this, other.size, false);
        std::memcpy(data, other.data, sizeof(T) * other.size);
        size = other.size;
        return *this;
    }

    template<typename T> array_t<T>& array_t<T>::operator=(array_t<T>&& other) noexcept {
        if (this == &other)
            return *this;
        clear(*this);
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = other.size = other.capacity = {};
        return *this;
    }
}

namespace basecode::slice {
    template <typename T> inline slice_t<T> make(
            array::array_t<T>& array,
            u32 start,
            u32 end) {
        return slice_t{.data = array.data + start, .length = end - start};
    }

    template <typename T> inline slice_t<T> make(array::array_t<T>& array) {
        return slice_t{.data = array.data, .length = array.size};
    }
}