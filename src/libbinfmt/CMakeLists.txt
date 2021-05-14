cmake_minimum_required(VERSION 3.16)
project(basecode-binfmt C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Binary Format Library")
set(LIBRARY_NAME        "basecode-binfmt")
set(LIBRARY_TARGET      "${LIBRARY_NAME}-lib")
set(INC_DIR             "${INC_DIR_BASE}/basecode/binfmt")

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/binfmt/configure.h"
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/binfmt.scm
    ${CMAKE_BINARY_DIR}/etc/binfmt.scm COPYONLY
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/binfmt-tests.scm
    ${CMAKE_BINARY_DIR}/etc/binfmt-tests.scm COPYONLY
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/binfmt-localization.scm
    ${CMAKE_BINARY_DIR}/etc/binfmt-localization.scm COPYONLY
)

# -----------------------------------------------------------------------------
#
# libbasecode-binfmt static library
#
# -----------------------------------------------------------------------------

set(
    BINFMT_SOURCES

    backends/backends.cpp
    binfmt.cpp ${INC_DIR}/binfmt.h

    cv.cpp ${INC_DIR}/cv.h
    ${INC_DIR}/pdb.h
    ${INC_DIR}/types.h
)
add_library(${LIBRARY_TARGET} ${BINFMT_SOURCES})
target_include_directories(
    ${LIBRARY_TARGET} PUBLIC

    ${INC_DIR_BASE}
    ${CMAKE_BINARY_DIR}/include)
set_target_properties(${LIBRARY_TARGET} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})
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
