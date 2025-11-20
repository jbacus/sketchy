add_library(sketchy_modeling
    tools.cpp
    extrusion.cpp
)

target_include_directories(sketchy_modeling
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(sketchy_modeling
    PUBLIC
        sketchy_kernel
)
