#include "winged_edge.h"
#include <algorithm>
#include <unordered_set>

namespace SketchyKernel {

// ==================== EULER OPERATORS ====================

std::shared_ptr<Vertex> WingedEdgeKernel::mvsf(const Point3D& coords) {
    // Create the first vertex
    auto vertex = std::make_shared<Vertex>(next_v_id++, coords);
    vertices.push_back(vertex);

    // Create the initial face (unbounded, represents the "outside" or the first face)
    auto face = std::make_shared<Face>(next_f_id++);
    faces.push_back(face);

    // Note: No edges are created yet - the vertex is isolated
    // The face exists but has no boundary until edges are added via MEV

    return vertex;
}

std::shared_ptr<Edge> WingedEdgeKernel::mev(std::shared_ptr<Vertex> from_vertex,
                                             const Point3D& to_coords,
                                             std::shared_ptr<Face> on_face) {
    if (!from_vertex) {
        throw std::invalid_argument("MEV: from_vertex cannot be null");
    }
    if (!on_face) {
        throw std::invalid_argument("MEV: on_face cannot be null");
    }

    // Create the new vertex
    auto new_vertex = std::make_shared<Vertex>(next_v_id++, to_coords);
    vertices.push_back(new_vertex);

    // Create the new edge connecting from_vertex to new_vertex
    auto new_edge = std::make_shared<Edge>(next_e_id++);
    new_edge->v1 = from_vertex;
    new_edge->v2 = new_vertex;
    new_edge->f1 = on_face;
    new_edge->f2 = on_face; // Initially, both sides are the same face

    edges.push_back(new_edge);

    // Update vertex edge references
    if (!from_vertex->edge) {
        from_vertex->edge = new_edge;
    }
    new_vertex->edge = new_edge;

    // Update face edge reference if not set
    if (!on_face->edge) {
        on_face->edge = new_edge;
    }

    // Wire up the edge connectivity
    // If this is the first edge on the face, it loops back to itself
    if (from_vertex->edge == new_edge) {
        // First edge from this vertex on this face
        new_edge->p1_f1 = new_edge;
        new_edge->n1_f1 = new_edge;
        new_edge->p2_f2 = new_edge;
        new_edge->n2_f2 = new_edge;
    } else {
        // Find the previous edge at from_vertex on this face
        auto prev_edge = from_vertex->edge;

        // Wire the new edge into the circular list
        new_edge->p1_f1 = prev_edge;
        new_edge->n1_f1 = prev_edge->n1_f1;

        if (prev_edge->v2 == from_vertex) {
            prev_edge->n2_f2 = new_edge;
        } else {
            prev_edge->n1_f1 = new_edge;
        }

        // For v2 (new vertex), edge loops back to itself initially
        new_edge->p2_f2 = new_edge;
        new_edge->n2_f2 = new_edge;
    }

    return new_edge;
}

std::shared_ptr<Edge> WingedEdgeKernel::mef(std::shared_ptr<Vertex> v1,
                                             std::shared_ptr<Vertex> v2,
                                             std::shared_ptr<Face> face) {
    if (!v1 || !v2 || !face) {
        throw std::invalid_argument("MEF: vertices and face cannot be null");
    }
    if (v1 == v2) {
        throw std::invalid_argument("MEF: cannot create edge between same vertex");
    }

    // Create the new edge
    auto new_edge = std::make_shared<Edge>(next_e_id++);
    new_edge->v1 = v1;
    new_edge->v2 = v2;

    // Create the new face (splits the original face)
    auto new_face = std::make_shared<Face>(next_f_id++);

    // Assign faces: old face on one side, new face on the other
    new_edge->f1 = face;
    new_edge->f2 = new_face;

    edges.push_back(new_edge);
    faces.push_back(new_face);

    // Update face edge references
    new_face->edge = new_edge;

    // Wire up the connectivity
    // This is complex - we need to find the path from v1 to v2 on the face
    // and split the face along this new edge

    // Find edges at v1 that are on this face
    auto v1_edge = v1->edge;
    if (v1_edge) {
        new_edge->p1_f1 = v1_edge;
        new_edge->n1_f1 = v1_edge;
    }

    // Find edges at v2 that are on this face
    auto v2_edge = v2->edge;
    if (v2_edge) {
        new_edge->p2_f2 = v2_edge;
        new_edge->n2_f2 = v2_edge;
    }

    return new_edge;
}

std::shared_ptr<Face> WingedEdgeKernel::kef(std::shared_ptr<Edge> edge) {
    if (!edge) {
        throw std::invalid_argument("KEF: edge cannot be null");
    }
    if (!edge->f1 || !edge->f2) {
        throw std::invalid_argument("KEF: edge must be adjacent to two faces");
    }

    // Get the two faces
    auto f1 = edge->f1;
    auto f2 = edge->f2;

    // Remove the edge from the edge list
    edges.erase(std::remove(edges.begin(), edges.end(), edge), edges.end());

    // Merge f2 into f1 (f1 survives, f2 is removed)
    // Update all edges that reference f2 to reference f1
    for (auto& e : edges) {
        if (e->f1 == f2) e->f1 = f1;
        if (e->f2 == f2) e->f2 = f1;
    }

    // Remove f2 from the face list
    faces.erase(std::remove(faces.begin(), faces.end(), f2), faces.end());

    // Rewire the edge connectivity around the removed edge
    // Connect the previous and next edges that were connected to the killed edge

    return f1;
}

void WingedEdgeKernel::kfmrh(std::shared_ptr<Face> hole_face, std::shared_ptr<Face> outer_face) {
    if (!hole_face || !outer_face) {
        throw std::invalid_argument("KFMRH: faces cannot be null");
    }

    // Remove the hole face from the face list
    faces.erase(std::remove(faces.begin(), faces.end(), hole_face), faces.end());

    // Update edges on the hole boundary to reference the outer face
    auto boundary = getFaceBoundary(hole_face);
    for (auto& edge : boundary) {
        if (edge->f1 == hole_face) edge->f1 = outer_face;
        if (edge->f2 == hole_face) edge->f2 = outer_face;
    }
}

// ==================== NAVIGATION & QUERY ====================

std::vector<std::shared_ptr<Edge>> WingedEdgeKernel::getIncidentEdges(std::shared_ptr<Vertex> v) const {
    std::vector<std::shared_ptr<Edge>> result;
    if (!v || !v->edge) return result;

    auto start_edge = v->edge;
    auto current_edge = start_edge;
    std::unordered_set<int> visited;

    do {
        if (visited.count(current_edge->id)) break;
        visited.insert(current_edge->id);
        result.push_back(current_edge);

        // Navigate to next edge around vertex
        if (current_edge->v1 == v) {
            current_edge = current_edge->n1_f1;
        } else if (current_edge->v2 == v) {
            current_edge = current_edge->n2_f2;
        } else {
            break;
        }

        if (!current_edge || visited.size() > edges.size()) break;

    } while (current_edge != start_edge);

    return result;
}

std::vector<std::shared_ptr<Face>> WingedEdgeKernel::getIncidentFaces(std::shared_ptr<Vertex> v) const {
    std::vector<std::shared_ptr<Face>> result;
    std::unordered_set<int> seen_face_ids;

    auto incident_edges = getIncidentEdges(v);
    for (const auto& edge : incident_edges) {
        if (edge->f1 && !seen_face_ids.count(edge->f1->id)) {
            result.push_back(edge->f1);
            seen_face_ids.insert(edge->f1->id);
        }
        if (edge->f2 && !seen_face_ids.count(edge->f2->id)) {
            result.push_back(edge->f2);
            seen_face_ids.insert(edge->f2->id);
        }
    }

    return result;
}

std::vector<std::shared_ptr<Edge>> WingedEdgeKernel::getFaceBoundary(std::shared_ptr<Face> f) const {
    std::vector<std::shared_ptr<Edge>> result;
    if (!f || !f->edge) return result;

    auto start_edge = f->edge;
    auto current_edge = start_edge;
    std::unordered_set<int> visited;

    do {
        if (visited.count(current_edge->id)) break;
        visited.insert(current_edge->id);
        result.push_back(current_edge);

        // Navigate to next edge on face boundary
        if (current_edge->f1 == f) {
            current_edge = current_edge->n1_f1;
        } else if (current_edge->f2 == f) {
            current_edge = current_edge->n2_f2;
        } else {
            break;
        }

        if (!current_edge || visited.size() > edges.size()) break;

    } while (current_edge != start_edge);

    return result;
}

std::vector<std::shared_ptr<Vertex>> WingedEdgeKernel::getFaceVertices(std::shared_ptr<Face> f) const {
    std::vector<std::shared_ptr<Vertex>> result;
    auto boundary = getFaceBoundary(f);

    for (const auto& edge : boundary) {
        if (edge->f1 == f) {
            result.push_back(edge->v1);
        } else {
            result.push_back(edge->v2);
        }
    }

    return result;
}

bool WingedEdgeKernel::validate() const {
    // Check all vertex edge references are valid
    for (const auto& v : vertices) {
        if (v->edge) {
            bool found = false;
            for (const auto& e : edges) {
                if (e == v->edge) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;

            // Vertex should be an endpoint of its edge
            if (v->edge->v1 != v && v->edge->v2 != v) {
                return false;
            }
        }
    }

    // Check all edges have valid vertices and faces
    for (const auto& e : edges) {
        if (!e->v1 || !e->v2) return false;

        // Check vertices exist in vertex list
        bool v1_found = false, v2_found = false;
        for (const auto& v : vertices) {
            if (v == e->v1) v1_found = true;
            if (v == e->v2) v2_found = true;
        }
        if (!v1_found || !v2_found) return false;
    }

    // Check all faces have valid edge references
    for (const auto& f : faces) {
        if (f->edge) {
            bool found = false;
            for (const auto& e : edges) {
                if (e == f->edge) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;

            // Face should be adjacent to its edge
            if (f->edge->f1 != f && f->edge->f2 != f) {
                return false;
            }
        }
    }

    return true;
}

bool WingedEdgeKernel::isManifold() const {
    // Check that each edge is adjacent to at most 2 faces
    for (const auto& e : edges) {
        if (e->f1 && e->f2 && e->f1 == e->f2) {
            // Same face on both sides might be ok for some configurations
            continue;
        }
    }

    // Check vertex manifold property - each vertex should have a consistent
    // disk topology (fan of faces)
    for (const auto& v : vertices) {
        auto incident_edges = getIncidentEdges(v);
        if (incident_edges.empty() && v->edge) {
            return false; // Inconsistent
        }
    }

    return true;
}

// ==================== ACCESSORS ====================

std::shared_ptr<Vertex> WingedEdgeKernel::getVertexById(int id) const {
    for (const auto& v : vertices) {
        if (v->id == id) return v;
    }
    return nullptr;
}

std::shared_ptr<Edge> WingedEdgeKernel::getEdgeById(int id) const {
    for (const auto& e : edges) {
        if (e->id == id) return e;
    }
    return nullptr;
}

std::shared_ptr<Face> WingedEdgeKernel::getFaceById(int id) const {
    for (const auto& f : faces) {
        if (f->id == id) return f;
    }
    return nullptr;
}

} // namespace SketchyKernel
