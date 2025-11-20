#ifndef SKETCHY_KERNEL_WINGED_EDGE_H
#define SKETCHY_KERNEL_WINGED_EDGE_H

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>

namespace SketchyKernel {

struct Vertex;
struct Edge;
struct Face;

// A simple structure for 3D coordinates
struct Point3D {
    double x, y, z;

    Point3D() : x(0), y(0), z(0) {}
    Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
};

// Represents a Vertex in the Winged-Edge structure
struct Vertex {
    int id;
    Point3D coords;
    std::shared_ptr<Edge> edge; // Pointer to one incident edge

    Vertex(int id, const Point3D& coords) : id(id), coords(coords), edge(nullptr) {}
};

// Represents an Edge in the Winged-Edge structure
struct Edge {
    int id;
    // Pointers to the two vertices this edge connects
    std::shared_ptr<Vertex> v1, v2;
    // Pointers to the two faces this edge separates
    std::shared_ptr<Face> f1, f2;
    // Pointers to the four adjacent edges
    // Naming convention: p = previous, n = next, relative to v1 and v2
    // p1_f1: previous edge at v1 on face f1
    // n1_f1: next edge at v1 on face f1
    // p2_f2: previous edge at v2 on face f2
    // n2_f2: next edge at v2 on face f2
    std::shared_ptr<Edge> p1_f1, n1_f1, p2_f2, n2_f2;

    Edge(int id) : id(id), v1(nullptr), v2(nullptr), f1(nullptr), f2(nullptr),
                   p1_f1(nullptr), n1_f1(nullptr), p2_f2(nullptr), n2_f2(nullptr) {}
};

// Represents a Face in the Winged-Edge structure
struct Face {
    int id;
    std::shared_ptr<Edge> edge; // Pointer to one edge on its boundary

    Face(int id) : id(id), edge(nullptr) {}
};

/**
 * The main class for topological operations using Euler Operators
 *
 * Euler Operators are atomic topological modifications that preserve
 * the topological validity of the model. They form the foundation for
 * all modeling operations (extrusion, splitting, etc.).
 */
class WingedEdgeKernel {
private:
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<std::shared_ptr<Edge>> edges;
    std::vector<std::shared_ptr<Face>> faces;
    int next_v_id = 1;
    int next_e_id = 1;
    int next_f_id = 1;

public:
    WingedEdgeKernel() = default;

    // ==================== EULER OPERATORS ====================

    /**
     * MVSF: Make Vertex, Solid, Face
     * Creates the initial object: one solid (implicitly), one face, and one vertex.
     * This is typically the first operation when building a solid model.
     *
     * @param coords The 3D coordinates of the vertex
     * @return Pointer to the created vertex
     */
    std::shared_ptr<Vertex> mvsf(const Point3D& coords);

    /**
     * MEV: Make Edge, Vertex
     * Creates a new edge and vertex, extending from an existing vertex.
     * This is used to grow the boundary of a face by adding a new vertex.
     *
     * @param from_vertex The existing vertex to extend from
     * @param to_coords The coordinates of the new vertex
     * @param on_face The face that will be modified
     * @return Pointer to the newly created edge
     */
    std::shared_ptr<Edge> mev(std::shared_ptr<Vertex> from_vertex,
                               const Point3D& to_coords,
                               std::shared_ptr<Face> on_face);

    /**
     * MEF: Make Edge, Face
     * Creates a new edge connecting two existing vertices on the same face,
     * splitting that face into two faces.
     * This is the fundamental operation for creating internal structure.
     *
     * @param v1 First vertex (must be on the face boundary)
     * @param v2 Second vertex (must be on the face boundary)
     * @param face The face to split
     * @return Pointer to the newly created edge
     */
    std::shared_ptr<Edge> mef(std::shared_ptr<Vertex> v1,
                               std::shared_ptr<Vertex> v2,
                               std::shared_ptr<Face> face);

    /**
     * KEF: Kill Edge, Face
     * Removes an edge and merges two adjacent faces into one.
     * This is the inverse of MEF.
     *
     * @param edge The edge to remove
     * @return Pointer to the remaining face
     */
    std::shared_ptr<Face> kef(std::shared_ptr<Edge> edge);

    /**
     * KFMRH: Kill Face, Make Ring Hole
     * Creates a hole in a face by removing an interior face.
     * Used for creating genus (handles/holes) in the model.
     *
     * @param hole_face The face to remove (creates a hole)
     * @param outer_face The face that will contain the hole
     */
    void kfmrh(std::shared_ptr<Face> hole_face, std::shared_ptr<Face> outer_face);

    // ==================== NAVIGATION & QUERY ====================

    /**
     * Get all edges incident to a vertex
     */
    std::vector<std::shared_ptr<Edge>> getIncidentEdges(std::shared_ptr<Vertex> v) const;

    /**
     * Get all faces incident to a vertex
     */
    std::vector<std::shared_ptr<Face>> getIncidentFaces(std::shared_ptr<Vertex> v) const;

    /**
     * Get all edges on the boundary of a face (in order)
     */
    std::vector<std::shared_ptr<Edge>> getFaceBoundary(std::shared_ptr<Face> f) const;

    /**
     * Get all vertices on the boundary of a face (in order)
     */
    std::vector<std::shared_ptr<Vertex>> getFaceVertices(std::shared_ptr<Face> f) const;

    /**
     * Validate the topological consistency of the entire data structure
     * Checks all Euler-Poincare relationships and connectivity invariants
     */
    bool validate() const;

    /**
     * Check if the model is a valid 2-manifold
     * Every edge should be adjacent to exactly 2 faces (or 1 for boundary edges)
     */
    bool isManifold() const;

    // ==================== ACCESSORS ====================

    size_t getVertexCount() const { return vertices.size(); }
    size_t getEdgeCount() const { return edges.size(); }
    size_t getFaceCount() const { return faces.size(); }

    const std::vector<std::shared_ptr<Vertex>>& getVertices() const { return vertices; }
    const std::vector<std::shared_ptr<Edge>>& getEdges() const { return edges; }
    const std::vector<std::shared_ptr<Face>>& getFaces() const { return faces; }

    /**
     * Get vertex by ID (for debugging)
     */
    std::shared_ptr<Vertex> getVertexById(int id) const;

    /**
     * Get edge by ID (for debugging)
     */
    std::shared_ptr<Edge> getEdgeById(int id) const;

    /**
     * Get face by ID (for debugging)
     */
    std::shared_ptr<Face> getFaceById(int id) const;
};

} // namespace SketchyKernel

#endif // SKETCHY_KERNEL_WINGED_EDGE_H
