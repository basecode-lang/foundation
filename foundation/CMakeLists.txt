cmake_minimum_required(VERSION 3.15)
project(basecode-foundation-core)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (PRODUCT_NAME  "Basecode Foundation Library")

configure_file (
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/foundation/configure.h"
)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/include
)

add_library(
        ${PROJECT_NAME}

        defer.h

        memory/dlmalloc_config.h
        memory/dlmalloc.c memory/dlmalloc.h
)