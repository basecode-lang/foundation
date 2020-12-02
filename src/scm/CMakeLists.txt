cmake_minimum_required(VERSION 3.16)
project(scm)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR       0)
set(VERSION_MINOR       1)
set(PRODUCT_NAME        "Basecode Foundation Scheme REPL")
set(SCM_INC_DIR        "${INC_DIR_BASE}/basecode/scm")
if (WIN32)
    set(PLATFORM_INC_DIR    "${INC_DIR_BASE}/platform/win32")
    set(PLATFORM_SRC_DIR    "${PROJECT_DIR_BASE}/src/platform/win32")
endif ()

# -----------------------------------------------------------------------------
#
# Library Configuration
#
# -----------------------------------------------------------------------------

configure_file(
    configure.in.h
    "${CMAKE_BINARY_DIR}/include/basecode/scm/configure.h"
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} basecode-core)
if (NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE -fuse-ld=lld)
endif ()
