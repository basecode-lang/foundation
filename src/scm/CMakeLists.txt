cmake_minimum_required(VERSION 3.15)
project(scm)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} basecode-core)
target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-array-bounds")
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-ftls-model=global-dynamic")

