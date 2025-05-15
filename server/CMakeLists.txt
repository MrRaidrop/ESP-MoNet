cmake_minimum_required(VERSION 3.13)
project(lightweight_https_server C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

file(GLOB_RECURSE SOURCES
    src/*.c
)

add_executable(https_server ${SOURCES})
target_link_libraries(https_server ssl crypto pthread)
