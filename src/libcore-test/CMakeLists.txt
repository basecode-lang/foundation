cmake_minimum_required(VERSION 3.15)
project(core-test)

add_executable(
        ${PROJECT_NAME}

        main.cpp
        array_tests.cpp
        stack_tests.cpp
        slice_tests.cpp
        intern_tests.cpp
        hashtable_tests.cpp
)
target_link_libraries(
        ${PROJECT_NAME}

        core
)