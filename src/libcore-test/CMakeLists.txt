cmake_minimum_required(VERSION 3.15)
project(core-test)

add_executable(
        ${PROJECT_NAME}

        test.h
        main.cpp
        ffi_tests.cpp
        cxx_tests.cpp
        set_tests.cpp
        path_tests.cpp
        list_tests.cpp
        array_tests.cpp
        stack_tests.cpp
        slice_tests.cpp
        queue_tests.cpp
        intern_tests.cpp
        bitset_tests.cpp
        symtab_tests.cpp
        bintree_tests.cpp
        hashtab_tests.cpp
        stable_array_tests.cpp
)
target_link_libraries(
        ${PROJECT_NAME}

        core
)
target_include_directories(
        ${PROJECT_NAME} PRIVATE

        ${PROJECT_SOURCE_DIR}
        ${INC_DIR_BASE}/include
        ${CMAKE_BINARY_DIR}/include
)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/etc
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DIR_BASE}/etc/ut.txt ${CMAKE_BINARY_DIR}/etc
)
