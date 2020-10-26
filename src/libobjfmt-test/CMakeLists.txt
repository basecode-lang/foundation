cmake_minimum_required(VERSION 3.16)
project(basecode-objfmt-test)

add_executable(
    ${PROJECT_NAME}

    test.h
    main.cpp
    pe_tests.cpp
)
target_link_libraries(${PROJECT_NAME} basecode-core basecode-objfmt)
if (NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE -fuse-ld=lld)
endif ()
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE ${PROJECT_SOURCE_DIR}
            ${PROJECT_DIR_BASE}/ext/catch2-2.8.0/single_include
)
#add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/etc
#    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DIR_BASE}/etc/ut.txt ${CMAKE_BINARY_DIR}/etc
#    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_DIR_BASE}/etc/instructions.ig ${CMAKE_BINARY_DIR}/etc
#)
