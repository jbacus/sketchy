add_library(sketchy_kernel
    geometry.cpp
    winged_edge.cpp
)

target_include_directories(sketchy_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
