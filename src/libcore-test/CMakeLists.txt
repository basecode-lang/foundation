cmake_minimum_required(VERSION 3.16)
project(basecode-core-test)

add_library(ffi-test-kernel SHARED ffi_kernel.cpp)
target_compile_options(ffi-test-kernel PRIVATE -march=native)
get_target_property(BASECODE_CORE_INCLUDE_DIRS basecode-core INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(ffi-test-kernel PRIVATE ${BASECODE_CORE_INCLUDE_DIRS})

add_executable(
    ${PROJECT_NAME}

    test.h
    main.cpp
    job_tests.cpp
    ffi_tests.cpp
    cxx_tests.cpp
    set_tests.cpp
    buf_tests.cpp
    eav_tests.cpp
    path_tests.cpp
    list_tests.cpp
    uuid_tests.cpp
    error_tests.cpp
    array_tests.cpp
    stack_tests.cpp
    slice_tests.cpp
    queue_tests.cpp
    buddy_tests.cpp
    config_tests.cpp
    intern_tests.cpp
    bitset_tests.cpp
    string_tests.cpp
    symtab_tests.cpp
    filesys_tests.cpp
    gap_buf_tests.cpp
    bintree_tests.cpp
    hashtab_tests.cpp
    str_array_tests.cpp
    assoc_array_tests.cpp
    stable_array_tests.cpp
)
target_link_libraries(${PROJECT_NAME} basecode-core)
if (NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE -fuse-ld=lld)
endif ()
add_dependencies(${PROJECT_NAME} ffi-test-kernel)
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE ${PROJECT_SOURCE_DIR}
            ${PROJECT_DIR_BASE}/ext/catch2-2.8.0/single_include
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/etc
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DIR_BASE}/etc/ut.txt ${CMAKE_BINARY_DIR}/etc
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DIR_BASE}/etc/instructions.ig ${CMAKE_BINARY_DIR}/etc
)
