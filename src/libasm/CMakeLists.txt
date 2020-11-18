cmake_minimum_required(VERSION 3.16)
project(basecode-asm C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Assembler Library")
set(LIBRARY_NAME        "basecode-asm")
set(LIBRARY_TARGET      "${LIBRARY_NAME}-lib")
set(INC_DIR             "${INC_DIR_BASE}/basecode/asm")

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/asm/configure.h"
)
#configure_file(
#    ${PROJECT_DIR_BASE}/etc/binfmt.fe
#    ${CMAKE_BINARY_DIR}/etc/binfmt.fe COPYONLY
#)

# -----------------------------------------------------------------------------
#
# libbasecode-asm static library
#
# -----------------------------------------------------------------------------

set(
    ASM_SOURCES

    asm.cpp ${INC_DIR}/asm.h
)
add_library(${LIBRARY_TARGET} ${ASM_SOURCES})
target_include_directories(
    ${LIBRARY_TARGET} PUBLIC

    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
set_target_properties(${LIBRARY_TARGET} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})
if (LINUX)
    set_target_properties(${LIBRARY_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()

target_link_libraries(
    ${LIBRARY_TARGET} PRIVATE

    basecode-core
)

# -----------------------------------------------------------------------------
#
# Interface Library
#
# -----------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
    ${PROJECT_NAME} INTERFACE

    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
target_link_libraries(
    ${PROJECT_NAME} INTERFACE

    basecode-core
    ${LIBRARY_TARGET})
