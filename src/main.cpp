#include <iostream>
#include "sketchy/kernel/winged_edge.h"

using namespace sketchy::kernel;

int main(int argc, char** argv) {
    std::cout << "Sketchy - Agentic 3D Modeler" << std::endl;
    std::cout << "=============================" << std::endl << std::endl;

    // Create a simple cube mesh
    auto cube = Mesh::create_cube(2.0);

    std::cout << "Created cube mesh:" << std::endl;
    std::cout << "  Vertices: " << cube->get_vertices().size() << std::endl;
    std::cout << "  Edges: " << cube->get_edges().size() << std::endl;
    std::cout << "  Faces: " << cube->get_faces().size() << std::endl;

    // Validate the mesh
    try {
        cube->validate();
        std::cout << "  Validation: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  Validation: FAILED - " << e.what() << std::endl;
        return 1;
    }

    // Check if manifold
    if (cube->is_manifold()) {
        std::cout << "  Manifold: YES" << std::endl;
    } else {
        std::cout << "  Manifold: NO" << std::endl;
    }

    std::cout << std::endl << "Winged-Edge data structure initialized successfully!" << std::endl;

    return 0;
}
