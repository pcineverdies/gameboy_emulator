cmake_minimum_required(VERSION 3.8)
project(my_project)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE sources
        "${CMAKE_SOURCE_DIR}/src/bus/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/memory/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/cpu/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/IO/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/test/test_output.cpp"
        )

add_executable(gameboy ${sources})

