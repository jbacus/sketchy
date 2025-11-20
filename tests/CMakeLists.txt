# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Consolidated kernel tests
add_executable(test_kernel
    unit/test_geometry.cpp
    unit/test_euler_operators.cpp
)

target_link_libraries(test_kernel
    PRIVATE
        sketchy_kernel
        GTest::gtest_main
)

target_include_directories(test_kernel
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(test_kernel)
