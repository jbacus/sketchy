add_library(sketchy_euler_kernel
    winged_edge.cpp
)

target_include_directories(sketchy_euler_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
