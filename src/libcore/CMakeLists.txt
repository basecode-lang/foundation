cmake_minimum_required(VERSION 3.15)
project(core)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PRODUCT_NAME  "Basecode Foundation Core Library")

set(INC_DIR "${INC_DIR_BASE}/basecode/core")

configure_file (
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/core/configure.h"
)

set(
        SOURCES

        ${INC_DIR}/assoc_array.h

        ${INC_DIR}/defer.h
        ${INC_DIR}/types.h

        ${INC_DIR}/array.h

        base64.cpp ${INC_DIR}/base64.h

        bass.cpp ${INC_DIR}/bass.h

        ${INC_DIR}/bintree.h

        bits.cpp ${INC_DIR}/bits.h

        bitset.cpp ${INC_DIR}/bitset.h

        buf.cpp ${INC_DIR}/buf.h

        context.cpp ${INC_DIR}/context.h

        cxx/scope.cpp
        cxx/module.cpp
        cxx/program.cpp
        cxx/serializer.cpp
        cxx/cxx.cpp ${INC_DIR}/cxx/cxx.h

        ${INC_DIR}/digraph.h

        error.cpp ${INC_DIR}/error.h

        filesys.cpp ${INC_DIR}/filesys.h

        ffi.cpp ${INC_DIR}/ffi.h

        format.cpp ${INC_DIR}/format.h ${INC_DIR}/format_types.h

        gap_buf.cpp ${INC_DIR}/gap_buf.h

        ${INC_DIR}/gfx.h

        ${INC_DIR}/hashable.h

        hash/crc.cpp ${INC_DIR}/hash/crc.h
        hash/adler.cpp ${INC_DIR}/hash/adler.h
        hash/murmur.cpp ${INC_DIR}/hash/murmur.h
        hash/blake3.cpp ${INC_DIR}/hash/blake3.h
        hash/symbol.cpp ${INC_DIR}/hash/symbol.h

        ${INC_DIR}/hashtab.h

        intern.cpp ${INC_DIR}/intern.h

        ${INC_DIR}/iterator.h

        ${INC_DIR}/list.h

        log.cpp ${INC_DIR}/log.h

        log/system/syslog.cpp ${INC_DIR}/log/system/syslog.h
        log/system/spdlog.cpp ${INC_DIR}/log/system/spdlog.h
        log/system/default.cpp ${INC_DIR}/log/system/default.h

        memory.cpp ${INC_DIR}/memory.h

        memory/dyn_alloc.cpp
        memory/meta.cpp ${INC_DIR}/memory/meta.h
        memory/std_alloc.cpp ${INC_DIR}/memory/std_alloc.h
        memory/system/dl.cpp ${INC_DIR}/memory/system/dl.h
        memory/system/page.cpp ${INC_DIR}/memory/system/page.h
        memory/system/slab.cpp ${INC_DIR}/memory/system/slab.h
        memory/system/bump.cpp ${INC_DIR}/memory/system/bump.h
        memory/system/trace.cpp ${INC_DIR}/memory/system/trace.h
        memory/system/proxy.cpp ${INC_DIR}/memory/system/proxy.h
        memory/system/stack.cpp ${INC_DIR}/memory/system/stack.h
        memory/system/default.cpp ${INC_DIR}/memory/system/default.h
        memory/system/dlmalloc.c ${INC_DIR}/memory/system/dlmalloc.h ${INC_DIR}/memory/system/dlmalloc_config.h

        network.cpp ${INC_DIR}/network.h

        numbers.cpp ${INC_DIR}/numbers.h

        path.cpp ${INC_DIR}/path.h

        plot.cpp ${INC_DIR}/plot.h

        profiler.cpp ${INC_DIR}/profiler.h

        ${INC_DIR}/queue.h

        ${INC_DIR}/set.h

        ${INC_DIR}/slice.h
        slice_utils.cpp ${INC_DIR}/slice_utils.h

        ${INC_DIR}/stack.h

        ${INC_DIR}/stable_array.h

        stopwatch.cpp ${INC_DIR}/stopwatch.h

        str.cpp ${INC_DIR}/str.h
        str_array.cpp ${INC_DIR}/str_array.h

        ${INC_DIR}/symtab.h

        timer.cpp ${INC_DIR}/timer.h
)
add_library(${PROJECT_NAME} ${SOURCES})
set_source_files_properties(buf.cpp PROPERTIES COMPILE_FLAGS "-march=native")
target_link_libraries(
        ${PROJECT_NAME}

        utf8proc
        blake3
        fmt-header-only
        spdlog::spdlog
        dyncall_s
        dynload_s
        ${DL_LIBRARY}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIR_BASE})
