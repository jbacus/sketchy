#include "sketchy/kernel/winged_edge.h"
#include <algorithm>

namespace sketchy {
namespace kernel {

std::vector<Edge*> Vertex::get_incident_edges() {
    std::vector<Edge*> result;
    if (!edge) return result;

    // Walk around the vertex collecting all incident edges
    Edge* current = edge;
    Edge* start = edge;

    do {
        result.push_back(current);

        // Find the next edge around this vertex
        if (current->v1 == this) {
            // We're at v1, so navigate via right face
            current = current->right_next;
            while (current && current->v2 != this && current->v1 != this) {
                if (current->v1 == this) {
                    current = current->right_next;
                } else {
                    current = current->left_next;
                }
            }
        } else {
            // We're at v2, so navigate via left face
            current = current->left_next;
            while (current && current->v1 != this && current->v2 != this) {
                if (current->v2 == this) {
                    current = current->left_next;
                } else {
                    current = current->right_next;
                }
            }
        }

        // Prevent infinite loop
        if (result.size() > 1000) break;

    } while (current && current != start);

    return result;
}

std::vector<Face*> Vertex::get_incident_faces() {
    std::vector<Face*> result;
    auto edges = get_incident_edges();

    for (Edge* e : edges) {
        if (e->left_face && std::find(result.begin(), result.end(), e->left_face) == result.end()) {
            result.push_back(e->left_face);
        }
        if (e->right_face && std::find(result.begin(), result.end(), e->right_face) == result.end()) {
            result.push_back(e->right_face);
        }
    }

    return result;
}

} // namespace kernel
} // namespace sketchy
