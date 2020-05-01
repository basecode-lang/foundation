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

        array.h

        bass.cpp bass.h

        bitset.cpp bitset.h

        context.cpp context.h

        cxx/scope.cpp
        cxx/module.cpp
        cxx/program.cpp
        cxx/serializer.cpp
        cxx/cxx.cpp cxx/cxx.h

        error.cpp error.h

        format.cpp format.h

        gfx.h

        hashing/hashable.h
        hashing/crc.cpp hashing/crc.h
        hashing/adler.cpp hashing/adler.h
        hashing/murmur.cpp hashing/murmur.h
        hashing/blake3.cpp hashing/blake3.h
        hashing/symbol.cpp hashing/symbol.h

        hashtable.h

        intern.cpp intern.h

        memory/std_allocator.h
        memory/dlmalloc_config.h
        memory/memory.cpp memory/memory.h
        memory/dlmalloc.c memory/dlmalloc.h
        memory/page_system.cpp memory/page_system.h
        memory/slab_system.cpp memory/slab_system.h
        memory/bump_system.cpp memory/bump_system.h
        memory/trace_system.cpp memory/trace_system.h
        memory/proxy_system.cpp memory/proxy_system.h
        memory/default_system.cpp memory/default_system.h
        memory/dlmalloc_system.cpp memory/dlmalloc_system.h

        profiler.cpp profiler.h
        stopwatch.cpp stopwatch.h

        slice.h

        source/buffer.cpp source/buffer.h

        stack.h

        str.cpp str.h

        timer.cpp timer.h
)
target_link_libraries(
        ${PROJECT_NAME}

        utf8proc
        blake3
        fmt-header-only
)
