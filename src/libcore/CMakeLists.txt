cmake_minimum_required(VERSION 3.16)
project(basecode-core C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Core Library")
set(LIBRARY_NAME        "basecode-core")
set(LIBRARY_TARGET      "${PROJECT_NAME}-lib")
set(CORE_INC_DIR        "${INC_DIR_BASE}/basecode/core")
if (WIN32)
    set(PLATFORM_INC_DIR    "${INC_DIR_BASE}/platform/win32")
    set(PLATFORM_SRC_DIR    "${PROJECT_DIR_BASE}/src/platform/win32")
endif ()

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/core/configure.h"
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/core.fe
    ${CMAKE_BINARY_DIR}/etc/core.fe COPYONLY
)

# -----------------------------------------------------------------------------
#
# libbasecode-core static library
#
# -----------------------------------------------------------------------------

set(
    CORE_SOURCES

    ${CORE_INC_DIR}/src_loc.h

    ast.cpp ${CORE_INC_DIR}/ast.h

    ${CORE_INC_DIR}/assoc_array.h

    ${CORE_INC_DIR}/defer.h
    ${CORE_INC_DIR}/types.h

    ${CORE_INC_DIR}/array.h

    base64.cpp ${CORE_INC_DIR}/base64.h

    bass.cpp ${CORE_INC_DIR}/bass.h

    ${CORE_INC_DIR}/bintree.h

    bits.cpp ${CORE_INC_DIR}/bits.h

    bitset.cpp ${CORE_INC_DIR}/bitset.h

    buf.cpp ${CORE_INC_DIR}/buf.h

    buf_pool.cpp ${CORE_INC_DIR}/buf_pool.h

    config.cpp ${CORE_INC_DIR}/config.h

    context.cpp ${CORE_INC_DIR}/context.h

    cxx/scope.cpp
    cxx/module.cpp
    cxx/program.cpp
    cxx/serializer.cpp
    cxx/cxx.cpp ${CORE_INC_DIR}/cxx/cxx.h

    ${CORE_INC_DIR}/digraph.h

    eav.cpp ${CORE_INC_DIR}/eav.h

    error.cpp ${CORE_INC_DIR}/error.h

    event.cpp ${CORE_INC_DIR}/event.h

    fe/src/fe.c fe/src/fe.h

    filesys.cpp ${CORE_INC_DIR}/filesys.h

    ffi.cpp ${CORE_INC_DIR}/ffi.h

    format.cpp ${CORE_INC_DIR}/format.h ${CORE_INC_DIR}/format_types.h

    gap_buf.cpp ${CORE_INC_DIR}/gap_buf.h

    ${CORE_INC_DIR}/hashable.h

    hash/crc.cpp ${CORE_INC_DIR}/hash/crc.h
    hash/adler.cpp ${CORE_INC_DIR}/hash/adler.h
    hash/murmur.cpp ${CORE_INC_DIR}/hash/murmur.h
    hash/blake3.cpp ${CORE_INC_DIR}/hash/blake3.h
    hash/symbol.cpp ${CORE_INC_DIR}/hash/symbol.h

    ${CORE_INC_DIR}/hashtab.h

    intern.cpp ${CORE_INC_DIR}/intern.h

    ipc.cpp ${CORE_INC_DIR}/ipc.h

    ${CORE_INC_DIR}/iterator.h

    job.cpp ${CORE_INC_DIR}/job.h

    ${CORE_INC_DIR}/list.h

    locale.cpp ${CORE_INC_DIR}/locale.h

    log.cpp ${CORE_INC_DIR}/log.h

    log/system/syslog.cpp ${CORE_INC_DIR}/log/system/syslog.h
    log/system/spdlog.cpp ${CORE_INC_DIR}/log/system/spdlog.h
    log/system/default.cpp ${CORE_INC_DIR}/log/system/default.h

    memory.cpp ${CORE_INC_DIR}/memory.h

    memory/meta.cpp ${CORE_INC_DIR}/memory/meta.h
    memory/dyn_alloc.cpp ${CORE_INC_DIR}/memory/dyn_alloc.h
    memory/std_alloc.cpp ${CORE_INC_DIR}/memory/std_alloc.h
    memory/system/dl.cpp ${CORE_INC_DIR}/memory/system/dl.h
    memory/system/page.cpp ${CORE_INC_DIR}/memory/system/page.h
    memory/system/slab.cpp ${CORE_INC_DIR}/memory/system/slab.h
    memory/system/bump.cpp ${CORE_INC_DIR}/memory/system/bump.h
    memory/system/trace.cpp ${CORE_INC_DIR}/memory/system/trace.h
    memory/system/proxy.cpp ${CORE_INC_DIR}/memory/system/proxy.h
    memory/system/stack.cpp ${CORE_INC_DIR}/memory/system/stack.h
    memory/system/default.cpp ${CORE_INC_DIR}/memory/system/default.h
    memory/system/dlmalloc.c ${CORE_INC_DIR}/memory/system/dlmalloc.h ${CORE_INC_DIR}/memory/system/dlmalloc_config.h

    mutex.cpp ${CORE_INC_DIR}/mutex.h

    network.cpp ${CORE_INC_DIR}/network.h

    numbers.cpp ${CORE_INC_DIR}/numbers.h

    path.cpp ${CORE_INC_DIR}/path.h

    plot.cpp ${CORE_INC_DIR}/plot.h

    pratt.cpp ${CORE_INC_DIR}/pratt.h

    profiler.cpp ${CORE_INC_DIR}/profiler.h

    ${CORE_INC_DIR}/queue.h

    rpn.cpp ${CORE_INC_DIR}/rpn.h

    ${CORE_INC_DIR}/set.h

    ${CORE_INC_DIR}/slice.h
    slice_utils.cpp ${CORE_INC_DIR}/slice_utils.h
    ${CORE_INC_DIR}/sql.h

    ${CORE_INC_DIR}/stack.h

    ${CORE_INC_DIR}/stable_array.h

    stopwatch.cpp ${CORE_INC_DIR}/stopwatch.h

    str.cpp ${CORE_INC_DIR}/str.h
    str_array.cpp ${CORE_INC_DIR}/str_array.h

    string.cpp ${CORE_INC_DIR}/string.h

    ${CORE_INC_DIR}/symtab.h

    timer.cpp ${CORE_INC_DIR}/timer.h

    term.cpp ${CORE_INC_DIR}/term.h

    thread.cpp ${CORE_INC_DIR}/thread.h

    token.cpp ${CORE_INC_DIR}/token.h
)
if (WIN32)
    set (
        PLATFORM_SOURCES

        ${PLATFORM_INC_DIR}/arpa/inet.h

        ${PLATFORM_INC_DIR}/sys/time.h
        ${PLATFORM_INC_DIR}/sys/socket.h
        ${PLATFORM_INC_DIR}/sys/sys_types.h
        ${PLATFORM_SRC_DIR}/mman.cpp ${PLATFORM_INC_DIR}/sys/mman.h

        ${PLATFORM_INC_DIR}/netdb.h
        ${PLATFORM_SRC_DIR}/glob.cpp ${PLATFORM_INC_DIR}/glob.h
        ${PLATFORM_SRC_DIR}/poll.cpp ${PLATFORM_INC_DIR}/poll.h
        ${PLATFORM_SRC_DIR}/unistd.cpp ${PLATFORM_INC_DIR}/unistd.h
        ${PLATFORM_SRC_DIR}/dirent.cpp ${PLATFORM_INC_DIR}/dirent.h
        ${PLATFORM_SRC_DIR}/syslog.cpp ${PLATFORM_INC_DIR}/syslog.h
        ${PLATFORM_SRC_DIR}/win32_locale.cpp ${PLATFORM_INC_DIR}/win32_locale.h
        #        ${PLATFORM_SRC_DIR}/pthread.cpp ${PLATFORM_INC_DIR}/pthread.h
    )
    if (MSVC)
        set (
            PLATFORM_SOURCES

            ${PLATFORM_SOURCES}
            ${PLATFORM_SRC_DIR}/pthread.cpp ${PLATFORM_INC_DIR}/pthread.h
        )
    endif ()
else ()
    set (PLATFORM_SOURCES "")
endif ()
add_library(${LIBRARY_TARGET} ${CORE_SOURCES} ${PLATFORM_SOURCES})
target_include_directories(
    ${LIBRARY_TARGET} PUBLIC

    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
target_include_directories(
    ${LIBRARY_TARGET} PRIVATE

    fe/src)
target_compile_options(${LIBRARY_TARGET} PUBLIC -march=native PRIVATE -Wno-array-bounds)
set_target_properties(${LIBRARY_TARGET} PROPERTIES LIBRARY_OUTPUT_NAME ${LIBRARY_NAME})
if (LINUX)
    set_target_properties(${LIBRARY_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

target_link_libraries(
    ${LIBRARY_TARGET} PRIVATE

    asmjit
    utf8proc
    blake3
    fmt
    sqlite3
    dyncall_s
    dynload_s
    spdlog::spdlog
    ${CMAKE_DL_LIBS}
)
if (WIN32)
    target_link_libraries(${LIBRARY_TARGET} PRIVATE wsock32 ws2_32)
endif()
target_link_libraries(dyncall_s ${LIBRARY_TARGET})
target_link_libraries(dynload_s ${LIBRARY_TARGET})

# -----------------------------------------------------------------------------
#
# Interface Library
#
# -----------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
    ${PROJECT_NAME} INTERFACE

    fe/src
    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
target_link_libraries(
    ${PROJECT_NAME} INTERFACE

    asmjit
    utf8proc
    blake3
    fmt
    dyncall_s
    dynload_s
    spdlog::spdlog
    ${LIBRARY_TARGET})
if (WIN32)
    target_link_libraries(${PROJECT_NAME} INTERFACE wsock32 ws2_32)
endif()

