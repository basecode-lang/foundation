cmake_minimum_required(VERSION 3.16)
project(basecode-asm-test)

add_executable(
    ${PROJECT_NAME}

    main.cpp
)
target_link_libraries(${PROJECT_NAME} basecode-core basecode-asm)
if (NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE -fuse-ld=lld)
endif ()
target_include_directories(
    ${PROJECT_NAME}

    PRIVATE ${PROJECT_SOURCE_DIR}
            ${PROJECT_DIR_BASE}/ext/catch2-2.8.0/single_include
)
