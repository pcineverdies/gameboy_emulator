cmake_minimum_required(VERSION 3.8)

project(gameboy_emu)

find_package(SDL2 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

file(GLOB_RECURSE sources
        "${CMAKE_SOURCE_DIR}/src/bus/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/memory/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/cpu/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/IO/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/PPU/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/test/test_display.cpp"
        )

add_executable(gameboy ${sources})

target_link_libraries(gameboy PRIVATE SDL2::SDL2)
