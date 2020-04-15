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

        context/system.cpp context/system.h

        error/system.cpp error/system.h

        format/system.cpp format/system.h

        graphics/types.h

        hashing/hashable.h
        hashing/crc.cpp hashing/crc.h
        hashing/adler.cpp hashing/adler.h
        hashing/murmur.cpp hashing/murmur.h
        hashing/blake3.cpp hashing/blake3.h

        hashtable/hashtable.h

        memory/std_allocator.h
        memory/dlmalloc_config.h
        memory/system.cpp memory/system.h
        memory/dlmalloc.c memory/dlmalloc.h

        profiler/system.cpp profiler/system.h
        profiler/stopwatch.cpp profiler/stopwatch.h

        slice/slice.h

        source/buffer.cpp source/buffer.h

        stack/stack.h

        string/formatters.h
        string/utf8_string.h
        string/intern.cpp string/intern.h
        string/ascii_string.cpp string/ascii_string.h

        timers/system.cpp timers/system.h
)
target_link_libraries(
        ${PROJECT_NAME}

        utf8proc
        blake3
        fmt-header-only
)