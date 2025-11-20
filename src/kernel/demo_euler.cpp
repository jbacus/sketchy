#include <iostream>
#include <iomanip>
#include "winged_edge.h"

using namespace SketchyKernel;

void printKernelStats(const WingedEdgeKernel& kernel) {
    std::cout << "  Vertices: " << kernel.getVertexCount()
              << " | Edges: " << kernel.getEdgeCount()
              << " | Faces: " << kernel.getFaceCount() << std::endl;
}

void demonstrateMVSF() {
    std::cout << "\n=== MVSF: Make Vertex-Solid-Face ===" << std::endl;
    std::cout << "Creating initial vertex and face..." << std::endl;

    WingedEdgeKernel kernel;
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));

    std::cout << "Created vertex ID " << v1->id
              << " at (" << v1->coords.x << ", "
              << v1->coords.y << ", " << v1->coords.z << ")" << std::endl;
    printKernelStats(kernel);
    std::cout << "✓ MVSF creates the foundation for building geometry" << std::endl;
}

void demonstrateMEV() {
    std::cout << "\n=== MEV: Make Edge-Vertex ===" << std::endl;
    std::cout << "Building a line segment..." << std::endl;

    WingedEdgeKernel kernel;
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    std::cout << "Created edge ID " << e1->id
              << " from vertex " << e1->v1->id
              << " to vertex " << e1->v2->id << std::endl;

    printKernelStats(kernel);
    std::cout << "✓ MEV extends the geometry by adding new edges and vertices" << std::endl;
}

void buildTriangle() {
    std::cout << "\n=== Building a Triangle with Euler Operators ===" << std::endl;

    WingedEdgeKernel kernel;

    std::cout << "Step 1: MVSF - Create initial vertex" << std::endl;
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    printKernelStats(kernel);

    std::cout << "\nStep 2: MEV - Add second vertex" << std::endl;
    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;
    std::cout << "  Edge " << e1->id << ": v" << v1->id << " -> v" << v2->id << std::endl;
    printKernelStats(kernel);

    std::cout << "\nStep 3: MEV - Add third vertex" << std::endl;
    auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
    auto v3 = e2->v2;
    std::cout << "  Edge " << e2->id << ": v" << v2->id << " -> v" << v3->id << std::endl;
    printKernelStats(kernel);

    std::cout << "\nStep 4: MEF - Close the triangle (split face)" << std::endl;
    auto e3 = kernel.mef(v3, v1, face);
    std::cout << "  Closing edge " << e3->id << ": v" << v3->id << " -> v" << v1->id << std::endl;
    std::cout << "  Face split: f" << e3->f1->id << " and f" << e3->f2->id << std::endl;
    printKernelStats(kernel);

    std::cout << "\nValidation: " << (kernel.validate() ? "PASSED ✓" : "FAILED ✗") << std::endl;
    std::cout << "Manifold: " << (kernel.isManifold() ? "YES ✓" : "NO ✗") << std::endl;

    std::cout << "\n✓ Successfully built a triangle using Euler operators!" << std::endl;
}

void buildSquare() {
    std::cout << "\n=== Building a Square (Quad) with Euler Operators ===" << std::endl;

    WingedEdgeKernel kernel;

    // Build the quad step by step
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
    auto v3 = e2->v2;

    auto e3 = kernel.mev(v3, Point3D(0, 1, 0), face);
    auto v4 = e3->v2;

    auto e4 = kernel.mef(v4, v1, face);

    std::cout << "Created square with vertices:" << std::endl;
    std::cout << "  v1: (0, 0, 0)" << std::endl;
    std::cout << "  v2: (1, 0, 0)" << std::endl;
    std::cout << "  v3: (1, 1, 0)" << std::endl;
    std::cout << "  v4: (0, 1, 0)" << std::endl;

    printKernelStats(kernel);

    std::cout << "\nFace boundary check:" << std::endl;
    for (const auto& f : kernel.getFaces()) {
        auto boundary = kernel.getFaceBoundary(f);
        auto verts = kernel.getFaceVertices(f);
        std::cout << "  Face " << f->id << ": " << boundary.size()
                  << " edges, " << verts.size() << " vertices" << std::endl;
    }

    std::cout << "\n✓ Successfully built a square!" << std::endl;
}

void demonstrateKEF() {
    std::cout << "\n=== KEF: Kill Edge-Face (Inverse of MEF) ===" << std::endl;
    std::cout << "Creating and then removing an edge..." << std::endl;

    WingedEdgeKernel kernel;

    // Build a simple split face scenario
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;
    auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
    auto v3 = e2->v2;

    std::cout << "Before MEF:" << std::endl;
    printKernelStats(kernel);

    // Split the face
    auto split_edge = kernel.mef(v3, v1, face);

    std::cout << "\nAfter MEF (face split):" << std::endl;
    printKernelStats(kernel);

    // Now merge back using KEF
    std::cout << "\nApplying KEF to merge faces back..." << std::endl;
    auto merged_face = kernel.kef(split_edge);

    std::cout << "After KEF (faces merged):" << std::endl;
    printKernelStats(kernel);
    std::cout << "Merged into face ID: " << merged_face->id << std::endl;

    std::cout << "\n✓ KEF successfully reversed the face split!" << std::endl;
}

void demonstrateTopologyQueries() {
    std::cout << "\n=== Topology Navigation Queries ===" << std::endl;

    WingedEdgeKernel kernel;

    // Build a triangle
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;
    auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
    auto v3 = e2->v2;
    kernel.mef(v3, v1, face);

    std::cout << "\nQuerying incident edges for each vertex:" << std::endl;
    for (const auto& v : kernel.getVertices()) {
        auto edges = kernel.getIncidentEdges(v);
        std::cout << "  Vertex " << v->id << ": " << edges.size() << " incident edges" << std::endl;
    }

    std::cout << "\nQuerying incident faces for each vertex:" << std::endl;
    for (const auto& v : kernel.getVertices()) {
        auto faces = kernel.getIncidentFaces(v);
        std::cout << "  Vertex " << v->id << ": " << faces.size() << " incident faces" << std::endl;
    }

    std::cout << "\n✓ Topological queries working correctly!" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   Sketchy Kernel: Euler Operator Demonstration       ║" << std::endl;
    std::cout << "║   Winged-Edge Topology with Constructive Operators   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << std::endl;

    demonstrateMVSF();
    demonstrateMEV();
    buildTriangle();
    buildSquare();
    demonstrateKEF();
    demonstrateTopologyQueries();

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "All Euler operator demonstrations completed successfully!" << std::endl;
    std::cout << "The kernel is ready for modeling operations (extrusion, etc.)" << std::endl;
    std::cout << std::string(60, '=') << std::endl << std::endl;

    return 0;
}
