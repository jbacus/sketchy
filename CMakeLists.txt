cmake_minimum_required(VERSION 3.15)
project(Sketchy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_GRAPHICS "Build graphics module with OpenGL" OFF)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

# Core library subdirectories
add_subdirectory(src/kernel)

if(BUILD_GRAPHICS)
    add_subdirectory(src/graphics)
endif()

# Main executable
add_executable(sketchy
    src/main.cpp
)

target_link_libraries(sketchy
    PRIVATE
        sketchy_kernel
)

# Euler operator demonstration
add_executable(euler_demo
    src/kernel/demo_euler.cpp
)

target_link_libraries(euler_demo
    PRIVATE
        sketchy_kernel
)

if(BUILD_GRAPHICS)
    target_link_libraries(sketchy PRIVATE sketchy_graphics)
endif()

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS sketchy DESTINATION bin)
