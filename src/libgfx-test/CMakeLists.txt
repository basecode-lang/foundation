cmake_minimum_required(VERSION 3.16)
project(basecode-gfx-test)

add_executable(
    ${PROJECT_NAME}

    main.cpp
    test_app.cpp test_app.h
)
target_link_libraries(${PROJECT_NAME} basecode-core basecode-gfx)
if (NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE -fuse-ld=lld)
endif ()
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE ${PROJECT_SOURCE_DIR}
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_DIR_BASE}/share
                                               ${CMAKE_BINARY_DIR}/share
)
