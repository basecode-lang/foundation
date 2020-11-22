cmake_minimum_required(VERSION 3.16)
project(basecode-vm C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Virtual Machine Library")
set(LIBRARY_NAME        "basecode-vm")
set(LIBRARY_TARGET      "${LIBRARY_NAME}-lib")
set(INC_DIR             "${INC_DIR_BASE}/basecode/vm")

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/vm/configure.h"
)
configure_file(
    ${PROJECT_DIR_BASE}/etc/vm.fe
    ${CMAKE_BINARY_DIR}/etc/vm.fe COPYONLY
)

# -----------------------------------------------------------------------------
#
# libbasecode-vm static library
#
# -----------------------------------------------------------------------------

set(
    VM_SOURCES

    bytecode.cpp ${INC_DIR}/bytecode.h

    vm.cpp ${INC_DIR}/vm.h
)
add_library(${LIBRARY_TARGET} ${VM_SOURCES})
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
