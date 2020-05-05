cmake_minimum_required(VERSION 3.15)
project(core-test)

include_directories(
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
)

add_executable(
        ${PROJECT_NAME}

        main.cpp
        cxx_tests.cpp
        array_tests.cpp
        stack_tests.cpp
        slice_tests.cpp
        intern_tests.cpp
        bitset_tests.cpp
        hashtable_tests.cpp
        stable_array_tests.cpp
)
target_link_libraries(
        ${PROJECT_NAME}

        core
)
