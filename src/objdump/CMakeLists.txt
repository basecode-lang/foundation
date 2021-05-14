cmake_minimum_required(VERSION 3.16)
project(basecode-objdump C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Object Dump Utility")
set(EXECUTABLE_NAME     "objdump")
set(EXECUTABLE_TARGET   "${EXECUTABLE_NAME}")
set(INC_DIR             "${INC_DIR_BASE}/basecode/objdump")

# -----------------------------------------------------------------------------
#
# Executable Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/objdump/configure.h"
)

add_executable(
    ${EXECUTABLE_TARGET}

    main.cpp
)
target_link_libraries(
    ${EXECUTABLE_TARGET}

    basecode-core
    basecode-binfmt
)
set_target_properties(
    ${EXECUTABLE_TARGET} PROPERTIES

    OUTPUT_NAME ${EXECUTABLE_NAME}
)
if (NOT MSVC)
    target_link_options(${EXECUTABLE_TARGET} PRIVATE -fuse-ld=lld)
endif ()
