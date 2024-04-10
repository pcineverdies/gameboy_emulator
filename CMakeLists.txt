cmake_minimum_required(VERSION 3.8)

project(gameboy_emu)

find_package(SDL2 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if( PROFILING )
  set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -pg")
else()
  set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
endif()

file(GLOB_RECURSE sources
        "${CMAKE_SOURCE_DIR}/src/bus/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/memory/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/cpu/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/IO/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/PPU/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/APU/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/gameboy.cpp"
        "${CMAKE_SOURCE_DIR}/src/utils/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/main.cpp"
        )

add_executable(gameboy ${sources})

target_link_libraries(gameboy PRIVATE SDL2::SDL2)
target_compile_features(gameboy PRIVATE cxx_std_17)


if(DEBUG)
  add_compile_definitions(__DEBUG)
endif()
