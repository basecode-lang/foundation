cmake_minimum_required(VERSION 3.16)
project(basecode-objfmt C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Object Format Library")
set(LIBRARY_NAME        "basecode-objfmt")
set(LIBRARY_TARGET      "${PROJECT_NAME}-lib")
set(INC_DIR             "${INC_DIR_BASE}/basecode/objfmt")

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/objfmt/configure.h"
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/objfmt.fe
    ${CMAKE_BINARY_DIR}/etc/objfmt.fe COPYONLY
)

# -----------------------------------------------------------------------------
#
# libbasecode-objfmt static library
#
# -----------------------------------------------------------------------------

set(
    OBJFMT_SOURCES

    coff.cpp ${INC_DIR}/coff.h

    container.cpp ${INC_DIR}/container.h

    elf.cpp ${INC_DIR}/elf.h

    macho.cpp ${INC_DIR}/macho.h

    pe.cpp ${INC_DIR}/pe.h

    objfmt.cpp ${INC_DIR}/objfmt.h

    ${INC_DIR}/types.h
)
add_library(${LIBRARY_TARGET} ${OBJFMT_SOURCES})
target_include_directories(
    ${LIBRARY_TARGET} PUBLIC

    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
set_target_properties(${LIBRARY_TARGET} PROPERTIES LIBRARY_OUTPUT_NAME ${LIBRARY_NAME})
if (LINUX)
    set_target_properties(${LIBRARY_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE True)
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
