cmake_minimum_required(VERSION 3.16)
project(basecode-foundation C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)

# common commands in a shorter form
set(MKDIR COMMAND ${CMAKE_COMMAND} -E make_directory)
set(CP    COMMAND ${CMAKE_COMMAND} -E copy)
set(CPR   COMMAND ${CMAKE_COMMAND} -E copy_directory)
set(CP_IF COMMAND ${CMAKE_COMMAND} -E copy_if_different)
set(LN    COMMAND ${CMAKE_COMMAND} -E create_symlink)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

if (MSVC)
    set(
            CXX_FLAGS
            /W3
            /EHsc
            -Qunused-arguments
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-signed-enum-bitfield
            -Wno-nonportable-system-include-path
            -Wno-newline-eof
            -Wno-deprecated-declarations
            -Wno-documentation-unknown-command
            -Wno-c99-designator
    )
    add_compile_options(
            "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_FLAGS}>"
    )
    set(
            CXX_DEFINES
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
            _CRT_DEPRECATED_NO_WARNINGS
            _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    add_compile_definitions(
            "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_DEFINES}>"
    )
else ()
    set(
        CXX_FLAGS
        -Wall
        -Wno-string-plus-int
        -Wno-signed-enum-bitfield
        -Wno-cast-qual
        -Wno-auto-var-id
        -Wno-unused-private-field
        -Wno-unknown-pragmas
        -Wno-inconsistent-missing-override
        -Wno-c99-designator
        -Wno-deprecated-declarations
    )

    set(CLANG_CXX_FLAGS -fcolor-diagnostics)

    set(GCC_CXX_FLAGS -fdiagnostics-color=always -Wno-parentheses -Wno-sequence-point)

    add_compile_options(
        "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_FLAGS}>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:${GCC_CXX_FLAGS}>"
        "$<$<COMPILE_LANG_AND_ID:CXX,Clang>:${CLANG_CXX_FLAGS}>"
    )
endif()

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# zlib
add_subdirectory(ext/zlib-1.2.11 EXCLUDE_FROM_ALL)
set(ZLIB_FOUND ON)
set(ZLIB_ROOT "${PROJECT_SOURCE_DIR}/ext/zlib-1.2.11")
if (MSVC)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(ZLIB_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstaticd.lib")
    else ()
        set(ZLIB_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlibstatic.lib")
    endif ()
    include_directories("${CMAKE_BINARY_DIR}/ext/zlib-1.2.11")
else ()
    set(ZLIB_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libz.a")
endif()
#set(ZLIB_LIBRARIES "${ZLIB_LIBRARY}")

# bzip2
set(
    BZIP2_SOURCES
    ext/bzip2-1.0.8/blocksort.c
    ext/bzip2-1.0.8/huffman.c
    ext/bzip2-1.0.8/crctable.c
    ext/bzip2-1.0.8/randtable.c
    ext/bzip2-1.0.8/compress.c
    ext/bzip2-1.0.8/decompress.c
    ext/bzip2-1.0.8/bzlib.c
)
add_library(bz2 STATIC ${BZIP2_SOURCES})
target_compile_definitions(bz2 PRIVATE -D_FILE_OFFSET_BITS=64)
set_target_properties(bz2 PROPERTIES LIBRARY_OUTPUT_NAME bz2)
target_include_directories(bz2 INTERFACE "${PROJECT_SOURCE_DIR}/ext/bzip2-1.0.8")
set(BZIP2_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/ext/bzip2-1.0.8")
set(BZIP2_INCLUDE_DIRS "${BZIP2_INCLUDE_DIR}")
if (MSVC)
    set(BZIP2_LIBRARIES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/bz2.lib")
else ()
    set(BZIP2_LIBRARIES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libbz2.a")
endif ()

# libfmt
add_subdirectory(ext/fmt-7.0.3 EXCLUDE_FROM_ALL)

# spdlog
option(SPDLOG_FMT_EXTERNAL  ""  ON)
option(SPDLOG_INSTALL       ""  OFF)
option(SPDLOG_BUILD_TESTS   ""  OFF)
option(SPDLOG_BUILD_EXAMPLE ""  OFF)
add_subdirectory(ext/spdlog-1.8.1 EXCLUDE_FROM_ALL)

# utf8proc
add_subdirectory(ext/utf8proc-2.5.0 EXCLUDE_FROM_ALL)
target_include_directories(utf8proc PUBLIC ext/utf8proc-2.5.0)

# blake3
set(
        BLAKE3_SOURCES
        ext/blake3-0.3.1/blake3.c
        ext/blake3-0.3.1/blake3_dispatch.c
        ext/blake3-0.3.1/blake3_portable.c
)
if (WIN32)
    set(
        BLAKE3_SOURCES
        ${BLAKE3_SOURCES}
        ext/blake3-0.3.1/blake3_sse41_x86-64_windows_gnu.S
        ext/blake3-0.3.1/blake3_avx2_x86-64_windows_gnu.S
        ext/blake3-0.3.1/blake3_avx512_x86-64_windows_gnu.S
    )
else ()
    set(
        BLAKE3_SOURCES
        ${BLAKE3_SOURCES}
        ext/blake3-0.3.1/blake3_sse41_x86-64_unix.S
        ext/blake3-0.3.1/blake3_avx2_x86-64_unix.S
        ext/blake3-0.3.1/blake3_avx512_x86-64_unix.S
    )
endif ()
add_library(blake3 ${BLAKE3_SOURCES})
if (NOT WIN32)
    target_compile_options(blake3 PUBLIC -march=native)
endif()
target_include_directories(blake3 INTERFACE ext/blake3-0.3.1)

# dyncall
add_subdirectory(ext/dyncall-1.1.0 EXCLUDE_FROM_ALL)
target_include_directories(dyncall_s INTERFACE ext/dyncall-1.1.0)
target_include_directories(dynload_s INTERFACE ext/dyncall-1.1.0)
set (DYN_ALLOC_HEADER "${PROJECT_SOURCE_DIR}/include/basecode/core/memory/dyn_alloc.h")
if (MSVC)
    target_compile_options(
        dyncall_s PRIVATE

        "$<$<COMPILE_LANGUAGE:C,CXX>:/FI${DYN_ALLOC_HEADER}>"
    )
    target_compile_options(
        dynload_s PRIVATE

        "$<$<COMPILE_LANGUAGE:C,CXX>:/FI${DYN_ALLOC_HEADER}>"
    )
else ()
    target_compile_options(
        dyncall_s PRIVATE

        "$<$<COMPILE_LANGUAGE:CXX>:--include=${DYN_ALLOC_HEADER}>"
    )
    target_compile_options(
        dynload_s PRIVATE

        "$<$<COMPILE_LANGUAGE:CXX>:--include=${DYN_ALLOC_HEADER}>"
    )
endif ()

# sqlite3
set(
        SQLITE3_SOURCES
        ext/sqlite-3.33.0/sqlite3.c
)
add_library(sqlite3 ${SQLITE3_SOURCES})
target_compile_options(sqlite3 PRIVATE -Wno-unused-variable)
target_include_directories(sqlite3 INTERFACE ext/sqlite-3.33.0)

# -----------------------------------------------------------------------------
#
# Project
#
# -----------------------------------------------------------------------------

set(INC_DIR_BASE        "${PROJECT_SOURCE_DIR}/include")
set(PROJECT_DIR_BASE    ${PROJECT_SOURCE_DIR})

add_subdirectory(src/libcore)
add_subdirectory(src/libcore-test)
add_subdirectory(src/libgfx)
add_subdirectory(src/libbinfmt)
add_subdirectory(src/libbinfmt-test)
add_subdirectory(src/libvm)
add_subdirectory(src/libvm-test)
