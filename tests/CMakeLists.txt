cmake_minimum_required(VERSION 3.10)
project(SketchyKernel CXX)

Set the C++ standard to C++17

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

------------------------------------------------------------------

1. Google Test Dependency

FetchContent is the modern way to include external dependencies like GTest

This downloads and builds the GTest framework automatically.

------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
googletest
URL https://www.google.com/search?q=https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
# SHA256 0c0a905898d9c57d5423f3387d838706d863f8d9134988e0013328518903e1a0
)
FetchContent_MakeAvailable(googletest)

------------------------------------------------------------------

2. Define Executable for Unit Tests

------------------------------------------------------------------

Include directory for the kernel source (so tests can find "winged_edge.h")

target_include_directories(gtest_main PUBLIC
src/kernel
)

Define the test executable

add_executable(kernel_tests tests/unit/test_winged_edge.cpp src/kernel/winged_edge.cpp)

Link the executable against GTest and GTest_main libraries

target_link_libraries(kernel_tests
PRIVATE
GTest::gtest_main
GTest::gtest
)

Enable automatic test discovery for CTest

include(CTest)
enable_testing()
add_test(NAME run_kernel_tests COMMAND kernel_tests)

Set common warning flags for better code quality

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
target_compile_options(kernel_tests PRIVATE
-Wall
-Wextra
-pedantic
)
endif()
