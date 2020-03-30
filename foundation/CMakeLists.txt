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
        types.h

        adt/types.h
        adt/string/utf8.cpp adt/string/utf8.h
        adt/string/ascii.cpp adt/string/ascii.h

        context/context.cpp context/context.h

        graphics/types.h

        memory/dlmalloc_config.h
        memory/system.cpp memory/system.h
        memory/dlmalloc.c memory/dlmalloc.h

        # XXX: just for explanatory reasons
        reasons.cpp reasons.h
)