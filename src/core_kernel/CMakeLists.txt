add_library(sketchy_kernel
    geometry.cpp
    winged_edge.cpp
    vertex.cpp
    edge.cpp
    face.cpp
)

target_include_directories(sketchy_kernel
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
