#include <iostream>
#include "kernel/winged_edge.h"
#include "kernel/geometry.h"

using namespace SketchyKernel;

int main(int argc, char** argv) {
    std::cout << "Sketchy - Agentic 3D Modeler" << std::endl;
    std::cout << "=============================" << std::endl << std::endl;

    // Create a kernel instance
    WingedEdgeKernel kernel;

    // Build a simple square using Euler operators
    std::cout << "Building a square with Euler operators..." << std::endl;

    // Start with MVSF
    auto v1 = kernel.mvsf(Vec3(0, 0, 0));
    auto face = kernel.getFaces()[0];

    // Build square face
    auto e1 = kernel.mev(v1, Vec3(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Vec3(1, 1, 0), face);
    auto v3 = e2->v2;

    auto e3 = kernel.mev(v3, Vec3(0, 1, 0), face);
    auto v4 = e3->v2;

    auto e4 = kernel.mef(v4, v1, face);

    std::cout << "\nGeometry statistics:" << std::endl;
    std::cout << "  Vertices: " << kernel.getVertexCount() << std::endl;
    std::cout << "  Edges: " << kernel.getEdgeCount() << std::endl;
    std::cout << "  Faces: " << kernel.getFaceCount() << std::endl;

    // Validate the mesh
    try {
        kernel.validate();
        std::cout << "  Validation: PASSED ✓" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  Validation: FAILED - " << e.what() << std::endl;
        return 1;
    }

    // Check if manifold
    if (kernel.isManifold()) {
        std::cout << "  Manifold: YES ✓" << std::endl;
    } else {
        std::cout << "  Manifold: NO" << std::endl;
    }

    std::cout << "\nWinged-Edge kernel initialized successfully!" << std::endl;
    std::cout << "Run './euler_demo' for interactive demonstrations." << std::endl;

    return 0;
}
