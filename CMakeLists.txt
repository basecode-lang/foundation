cmake_minimum_required(VERSION 3.15)
project(basecode-foundation)

set(CMAKE_CXX_STANDARD 20)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(DL_LIBRARY "")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    add_definitions(-D__USE_ISOC99)
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DL_LIBRARY "dl")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-string-plus-int" CACHE STRING "compile flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-cast-qual -Wno-string-plus-int -Wno-auto-var-id -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override -Wno-c99-designator" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Wno-string-plus-int -Wno-unknown-pragmas -Wno-parentheses -Wno-sequence-point" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(FATAL_ERROR "${PROJECT_NAME} unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif()
endif()

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# catch2
include_directories(ext/catch2-2.8.0/single_include)

# libfmt
add_subdirectory(ext/fmt-6.2.0 EXCLUDE_FROM_ALL)

# spdlog
option(SPDLOG_FMT_EXTERNAL "" ON)
option(SPDLOG_INSTALL "" OFF)
option(SPDLOG_BUILD_TESTS "" OFF)
option(SPDLOG_BUILD_EXAMPLE "" OFF)
add_subdirectory(ext/spdlog-1.5.0 EXCLUDE_FROM_ALL)

# utf8proc
add_subdirectory(ext/utf8proc-2.5.0 EXCLUDE_FROM_ALL)
include_directories(ext/utf8proc-2.5.0)

# blake3
include_directories(ext/blake3-0.3.1)
set(
        BLAKE3_SOURCES
        ext/blake3-0.3.1/blake3.c
        ext/blake3-0.3.1/blake3_dispatch.c
        ext/blake3-0.3.1/blake3_portable.c
        ext/blake3-0.3.1/blake3_sse41_x86-64_unix.S
        ext/blake3-0.3.1/blake3_avx2_x86-64_unix.S
        ext/blake3-0.3.1/blake3_avx512_x86-64_unix.S
)
add_library(blake3 ${BLAKE3_SOURCES})

# dyncall
include_directories(ext/dyncall-1.1.0)
add_subdirectory(ext/dyncall-1.1.0 EXCLUDE_FROM_ALL)
target_compile_definitions(dyncall_s PRIVATE malloc=dyn_alloc free=dyn_free)
target_compile_definitions(dynload_s PRIVATE malloc=dyn_alloc free=dyn_free)

# -----------------------------------------------------------------------------
#
# Project
#
# -----------------------------------------------------------------------------

set(INC_DIR_BASE "${PROJECT_SOURCE_DIR}/include")
set(PROJECT_DIR_BASE ${PROJECT_SOURCE_DIR})

add_subdirectory(src/libcore)
add_subdirectory(src/libcore-test)
