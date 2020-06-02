cmake_minimum_required(VERSION 3.15)
project(basecode-foundation C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
if (MSVC)
    message(FATAL "Microsoft Visual C++ not supported.")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)

# common commands in a shorter form
set(MKDIR COMMAND ${CMAKE_COMMAND} -E make_directory)
set(CPR   COMMAND ${CMAKE_COMMAND} -E copy_directory)
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

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif()
endif()

add_compile_options(-Wall)
add_compile_options(-Wno-string-plus-int)
add_compile_options(-Wno-cast-qual -Wno-auto-var-id -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override -Wno-c99-designator)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    add_compile_options(-Wno-parentheses -Wno-sequence-point)
endif()

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# libfmt
add_subdirectory(ext/fmt-6.2.0 EXCLUDE_FROM_ALL)

# spdlog
option(SPDLOG_FMT_EXTERNAL  ""  ON)
option(SPDLOG_INSTALL       ""  OFF)
option(SPDLOG_BUILD_TESTS   ""  OFF)
option(SPDLOG_BUILD_EXAMPLE ""  OFF)
add_subdirectory(ext/spdlog-1.5.0 EXCLUDE_FROM_ALL)

# utf8proc
add_subdirectory(ext/utf8proc-2.5.0 EXCLUDE_FROM_ALL)

# blake3
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
target_include_directories(blake3 INTERFACE ext/blake3-0.3.1)

# dyncall
add_subdirectory(ext/dyncall-1.1.0 EXCLUDE_FROM_ALL)
target_compile_definitions(dyncall_s PRIVATE malloc=dyn_alloc free=dyn_free)
target_include_directories(dyncall_s INTERFACE ext/dyncall-1.1.0)
target_compile_definitions(dynload_s PRIVATE malloc=dyn_alloc free=dyn_free)
target_include_directories(dynload_s INTERFACE ext/dyncall-1.1.0)

# -----------------------------------------------------------------------------
#
# Project
#
# -----------------------------------------------------------------------------

set(INC_DIR_BASE        "${PROJECT_SOURCE_DIR}/include")
set(PROJECT_DIR_BASE    ${PROJECT_SOURCE_DIR})

add_subdirectory(src/libcore)
add_subdirectory(src/libcore-test)
add_subdirectory(src/scm)
