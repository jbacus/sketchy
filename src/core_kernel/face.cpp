#include "sketchy/kernel/winged_edge.h"
#include <cmath>

namespace sketchy {
namespace kernel {

std::vector<Edge*> Face::get_boundary_edges() {
    std::vector<Edge*> result;
    if (!edge) return result;

    Edge* current = edge;
    Edge* start = edge;

    do {
        result.push_back(current);

        // Navigate to the next edge on this face
        if (current->left_face == this) {
            current = current->left_next;
        } else if (current->right_face == this) {
            current = current->right_next;
        } else {
            // Error: edge doesn't belong to this face
            break;
        }

        // Safety check to prevent infinite loops
        if (result.size() > 1000) break;

    } while (current && current != start);

    return result;
}

std::vector<Vertex*> Face::get_vertices() {
    std::vector<Vertex*> result;
    auto edges = get_boundary_edges();

    for (Edge* e : edges) {
        // Determine which vertex to add based on winding order
        if (e->left_face == this) {
            result.push_back(e->v1);
        } else {
            result.push_back(e->v2);
        }
    }

    return result;
}

void Face::compute_normal() {
    auto vertices = get_vertices();

    if (vertices.size() < 3) {
        normal = Vec3(0, 0, 1); // Default normal
        return;
    }

    // Use Newell's method for robustness with non-planar polygons
    Vec3 n(0, 0, 0);

    for (size_t i = 0; i < vertices.size(); i++) {
        const Vec3& v1 = vertices[i]->position;
        const Vec3& v2 = vertices[(i + 1) % vertices.size()]->position;

        n.x += (v1.y - v2.y) * (v1.z + v2.z);
        n.y += (v1.z - v2.z) * (v1.x + v2.x);
        n.z += (v1.x - v2.x) * (v1.y + v2.y);
    }

    normal = n.normalized();
}

double Face::area() const {
    auto vertices_list = const_cast<Face*>(this)->get_vertices();

    if (vertices_list.size() < 3) {
        return 0.0;
    }

    // Triangulate and sum areas
    double total_area = 0.0;
    const Vec3& v0 = vertices_list[0]->position;

    for (size_t i = 1; i < vertices_list.size() - 1; i++) {
        const Vec3& v1 = vertices_list[i]->position;
        const Vec3& v2 = vertices_list[i + 1]->position;

        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 cross = edge1.cross(edge2);

        total_area += cross.length() * 0.5;
    }

    return total_area;
}

} // namespace kernel
} // namespace sketchy
