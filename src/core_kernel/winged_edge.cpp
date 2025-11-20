#include "sketchy/kernel/winged_edge.h"
#include <stdexcept>
#include <unordered_set>

namespace sketchy {
namespace kernel {

Vertex* Mesh::add_vertex(const Vec3& position) {
    vertices.push_back(std::make_unique<Vertex>(position));
    return vertices.back().get();
}

Edge* Mesh::add_edge(Vertex* v1, Vertex* v2) {
    if (!v1 || !v2) {
        throw std::invalid_argument("Cannot create edge with null vertices");
    }

    if (v1 == v2) {
        throw std::invalid_argument("Cannot create edge with same start and end vertex");
    }

    edges.push_back(std::make_unique<Edge>(v1, v2));
    Edge* new_edge = edges.back().get();

    // Set vertex edge reference if not already set
    if (!v1->edge) v1->edge = new_edge;
    if (!v2->edge) v2->edge = new_edge;

    return new_edge;
}

Face* Mesh::add_face(const std::vector<Vertex*>& verts) {
    if (verts.size() < 3) {
        throw std::invalid_argument("Face must have at least 3 vertices");
    }

    // Create or find edges for this face
    std::vector<Edge*> face_edges;

    for (size_t i = 0; i < verts.size(); i++) {
        Vertex* v1 = verts[i];
        Vertex* v2 = verts[(i + 1) % verts.size()];

        // Look for existing edge
        Edge* edge = nullptr;
        for (auto& e : edges) {
            if ((e->v1 == v1 && e->v2 == v2) || (e->v1 == v2 && e->v2 == v1)) {
                edge = e.get();
                break;
            }
        }

        // Create new edge if not found
        if (!edge) {
            edge = add_edge(v1, v2);
        }

        face_edges.push_back(edge);
    }

    // Create the face
    faces.push_back(std::make_unique<Face>(face_edges[0]));
    Face* new_face = faces.back().get();

    // Set up winged-edge connections
    for (size_t i = 0; i < face_edges.size(); i++) {
        Edge* current = face_edges[i];
        Edge* next = face_edges[(i + 1) % face_edges.size()];
        Edge* prev = face_edges[(i + face_edges.size() - 1) % face_edges.size()];

        // Determine if we're on the left or right side of the edge
        bool is_left_face = (current->left_face == nullptr);

        if (is_left_face) {
            current->left_face = new_face;
            current->left_next = next;
            current->left_prev = prev;
        } else {
            current->right_face = new_face;
            current->right_next = next;
            current->right_prev = prev;
        }
    }

    new_face->compute_normal();
    return new_face;
}

bool Mesh::is_manifold() const {
    // Check that every edge has at most 2 incident faces
    for (const auto& edge : edges) {
        int face_count = 0;
        if (edge->left_face) face_count++;
        if (edge->right_face) face_count++;

        if (face_count > 2) {
            return false; // Non-manifold edge
        }
    }

    // Check that every vertex has a consistent fan of faces
    for (const auto& vertex : vertices) {
        // This is a simplified check; full manifold verification is complex
        if (!vertex->edge) continue;

        auto incident_edges = const_cast<Vertex*>(vertex.get())->get_incident_edges();
        if (incident_edges.empty()) {
            return false;
        }
    }

    return true;
}

void Mesh::validate() const {
    // Check vertex integrity
    for (const auto& v : vertices) {
        if (v->edge) {
            if (v->edge->v1 != v.get() && v->edge->v2 != v.get()) {
                throw std::runtime_error("Vertex edge reference is invalid");
            }
        }
    }

    // Check edge integrity
    for (const auto& e : edges) {
        if (!e->v1 || !e->v2) {
            throw std::runtime_error("Edge has null vertex");
        }

        // Check face references
        if (e->left_face) {
            bool found = false;
            for (const auto& f : faces) {
                if (f.get() == e->left_face) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Edge left_face reference is invalid");
            }
        }
    }

    // Check face integrity
    for (const auto& f : faces) {
        if (!f->edge) {
            throw std::runtime_error("Face has null edge reference");
        }

        auto boundary_edges = const_cast<Face*>(f.get())->get_boundary_edges();
        if (boundary_edges.size() < 3) {
            throw std::runtime_error("Face has fewer than 3 boundary edges");
        }
    }
}

std::unique_ptr<Mesh> Mesh::create_cube(double size) {
    auto mesh = std::make_unique<Mesh>();

    double half = size / 2.0;

    // Create 8 vertices of a cube
    auto v0 = mesh->add_vertex(Vec3(-half, -half, -half));
    auto v1 = mesh->add_vertex(Vec3( half, -half, -half));
    auto v2 = mesh->add_vertex(Vec3( half,  half, -half));
    auto v3 = mesh->add_vertex(Vec3(-half,  half, -half));
    auto v4 = mesh->add_vertex(Vec3(-half, -half,  half));
    auto v5 = mesh->add_vertex(Vec3( half, -half,  half));
    auto v6 = mesh->add_vertex(Vec3( half,  half,  half));
    auto v7 = mesh->add_vertex(Vec3(-half,  half,  half));

    // Create 6 faces
    mesh->add_face({v0, v1, v2, v3}); // Front
    mesh->add_face({v5, v4, v7, v6}); // Back
    mesh->add_face({v4, v0, v3, v7}); // Left
    mesh->add_face({v1, v5, v6, v2}); // Right
    mesh->add_face({v3, v2, v6, v7}); // Top
    mesh->add_face({v4, v5, v1, v0}); // Bottom

    return mesh;
}

std::unique_ptr<Mesh> Mesh::create_plane(double width, double height) {
    auto mesh = std::make_unique<Mesh>();

    double hw = width / 2.0;
    double hh = height / 2.0;

    auto v0 = mesh->add_vertex(Vec3(-hw, -hh, 0));
    auto v1 = mesh->add_vertex(Vec3( hw, -hh, 0));
    auto v2 = mesh->add_vertex(Vec3( hw,  hh, 0));
    auto v3 = mesh->add_vertex(Vec3(-hw,  hh, 0));

    mesh->add_face({v0, v1, v2, v3});

    return mesh;
}

} // namespace kernel
} // namespace sketchy
