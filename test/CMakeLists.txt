cmake_minimum_required(VERSION 3.15)
project(basecode-foundation-test)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/include
)

add_executable(
        ${PROJECT_NAME}

        main.cpp
        array_tests.cpp
        stack_tests.cpp
        intern_tests.cpp
        hashtable_tests.cpp
)
target_link_libraries(
        ${PROJECT_NAME}

        basecode-foundation-core
)