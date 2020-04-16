cmake_minimum_required(VERSION 3.15)
project(core)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PRODUCT_NAME  "Basecode Foundation Core Library")

configure_file (
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/core/configure.h"
)

add_library(
        ${PROJECT_NAME}

        defer.h
        types.h

        array/array.h

        bass/system.cpp bass/system.h

        bitset/bitset.cpp bitset/bitset.h

        context/system.cpp context/system.h

        cxx/scope.cpp
        cxx/module.cpp
        cxx/program.cpp
        cxx/serializer.cpp
        cxx/types.cpp cxx/types.h

        error/system.cpp error/system.h

        format/system.cpp format/system.h

        gfx/types.h

        hashing/hashable.h
        hashing/crc.cpp hashing/crc.h
        hashing/adler.cpp hashing/adler.h
        hashing/murmur.cpp hashing/murmur.h
        hashing/blake3.cpp hashing/blake3.h
        hashing/symbol.cpp hashing/symbol.h

        hashtable/hashtable.h

        intern/intern.cpp intern/intern.h

        memory/std_allocator.h
        memory/dlmalloc_config.h
        memory/proxy.cpp memory/proxy.h
        memory/system.cpp memory/system.h
        memory/dlmalloc.c memory/dlmalloc.h
        memory/page_system.cpp memory/page_system.h
        memory/bump_system.cpp memory/bump_system.h
        memory/proxy_system.cpp memory/proxy_system.h
        memory/default_system.cpp memory/default_system.h
        memory/dlmalloc_system.cpp memory/dlmalloc_system.h

        profiler/system.cpp profiler/system.h
        profiler/stopwatch.cpp profiler/stopwatch.h

        slice/slice.h

        source/buffer.cpp source/buffer.h

        stack/stack.h

        string/str.cpp string/str.h

        timers/system.cpp timers/system.h
)
target_link_libraries(
        ${PROJECT_NAME}

        utf8proc
        blake3
        fmt-header-only
)