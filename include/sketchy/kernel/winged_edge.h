#ifndef SKETCHY_KERNEL_WINGED_EDGE_H
#define SKETCHY_KERNEL_WINGED_EDGE_H

#include <memory>
#include <vector>
#include "geometry.h"

namespace sketchy {
namespace kernel {

class Edge;
class Face;

/**
 * Vertex in the Winged-Edge data structure
 *
 * Each vertex stores:
 * - Its 3D position
 * - A reference to one incident edge (for topological navigation)
 */
class Vertex {
public:
    Vec3 position;
    Edge* edge;  // One incident edge

    Vertex(const Vec3& pos) : position(pos), edge(nullptr) {}

    // Navigate to all incident edges
    std::vector<Edge*> get_incident_edges();
    std::vector<Face*> get_incident_faces();
};

/**
 * Edge in the Winged-Edge data structure
 *
 * Each edge stores:
 * - Two endpoint vertices
 * - Two adjacent faces (left and right)
 * - Four "wing" edges for topological navigation
 */
class Edge {
public:
    Vertex* v1;  // Start vertex
    Vertex* v2;  // End vertex

    Face* left_face;   // Face on the left
    Face* right_face;  // Face on the right

    // Wing edges for navigation
    Edge* left_prev;   // Previous edge on left face
    Edge* left_next;   // Next edge on left face
    Edge* right_prev;  // Previous edge on right face
    Edge* right_next;  // Next edge on right face

    Edge(Vertex* v1, Vertex* v2)
        : v1(v1), v2(v2),
          left_face(nullptr), right_face(nullptr),
          left_prev(nullptr), left_next(nullptr),
          right_prev(nullptr), right_next(nullptr) {}

    // Get the other vertex
    Vertex* other_vertex(Vertex* v) const {
        return (v == v1) ? v2 : v1;
    }

    // Get the other face
    Face* other_face(Face* f) const {
        return (f == left_face) ? right_face : left_face;
    }

    double length() const {
        return (v2->position - v1->position).length();
    }
};

/**
 * Face in the Winged-Edge data structure
 *
 * Each face stores:
 * - A reference to one boundary edge
 * - Computed normal vector (cached)
 */
class Face {
public:
    Edge* edge;  // One boundary edge
    Vec3 normal; // Cached normal vector

    Face(Edge* e) : edge(e), normal(0, 0, 0) {
        compute_normal();
    }

    // Get all boundary edges in order
    std::vector<Edge*> get_boundary_edges();

    // Get all vertices in order
    std::vector<Vertex*> get_vertices();

    // Compute and cache the face normal
    void compute_normal();

    // Get the area of the face
    double area() const;
};

/**
 * Mesh - The complete Winged-Edge data structure
 */
class Mesh {
private:
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Edge>> edges;
    std::vector<std::unique_ptr<Face>> faces;

public:
    Mesh() = default;

    // Factory methods
    Vertex* add_vertex(const Vec3& position);
    Edge* add_edge(Vertex* v1, Vertex* v2);
    Face* add_face(const std::vector<Vertex*>& verts);

    // Accessors
    const std::vector<std::unique_ptr<Vertex>>& get_vertices() const { return vertices; }
    const std::vector<std::unique_ptr<Edge>>& get_edges() const { return edges; }
    const std::vector<std::unique_ptr<Face>>& get_faces() const { return faces; }

    // Topology operations
    bool is_manifold() const;
    void validate() const;

    // Create basic primitives
    static std::unique_ptr<Mesh> create_cube(double size = 1.0);
    static std::unique_ptr<Mesh> create_plane(double width, double height);
};

} // namespace kernel
} // namespace sketchy

#endif // SKETCHY_KERNEL_WINGED_EDGE_H
