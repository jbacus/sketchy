#include <gtest/gtest.h>
#include "sketchy/kernel/winged_edge.h"

using namespace sketchy::kernel;

// Test fixture for Winged-Edge tests
class WingedEdgeTest : public ::testing::Test {
protected:
    std::unique_ptr<Mesh> mesh;

    void SetUp() override {
        mesh = std::make_unique<Mesh>();
    }
};

// Vertex Tests
TEST_F(WingedEdgeTest, AddVertex) {
    Vertex* v = mesh->add_vertex(Vec3(1.0, 2.0, 3.0));

    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->position.x, 1.0);
    EXPECT_EQ(v->position.y, 2.0);
    EXPECT_EQ(v->position.z, 3.0);
    EXPECT_EQ(mesh->get_vertices().size(), 1);
}

TEST_F(WingedEdgeTest, AddMultipleVertices) {
    mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    mesh->add_vertex(Vec3(1.0, 1.0, 0.0));

    EXPECT_EQ(mesh->get_vertices().size(), 3);
}

// Edge Tests
TEST_F(WingedEdgeTest, AddEdge) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));

    Edge* e = mesh->add_edge(v1, v2);

    ASSERT_NE(e, nullptr);
    EXPECT_EQ(e->v1, v1);
    EXPECT_EQ(e->v2, v2);
    EXPECT_EQ(mesh->get_edges().size(), 1);
}

TEST_F(WingedEdgeTest, EdgeLength) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(3.0, 4.0, 0.0));

    Edge* e = mesh->add_edge(v1, v2);

    EXPECT_DOUBLE_EQ(e->length(), 5.0);
}

TEST_F(WingedEdgeTest, EdgeInvalidVertices) {
    Vertex* v = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));

    // Same vertex for both endpoints
    EXPECT_THROW(mesh->add_edge(v, v), std::invalid_argument);

    // Null vertices
    EXPECT_THROW(mesh->add_edge(nullptr, v), std::invalid_argument);
    EXPECT_THROW(mesh->add_edge(v, nullptr), std::invalid_argument);
}

// Face Tests
TEST_F(WingedEdgeTest, AddTriangleFace) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(0.5, 1.0, 0.0));

    Face* f = mesh->add_face({v1, v2, v3});

    ASSERT_NE(f, nullptr);
    EXPECT_EQ(mesh->get_faces().size(), 1);
    EXPECT_EQ(mesh->get_edges().size(), 3);

    // Check that face has 3 boundary edges
    auto edges = f->get_boundary_edges();
    EXPECT_EQ(edges.size(), 3);
}

TEST_F(WingedEdgeTest, AddQuadFace) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(1.0, 1.0, 0.0));
    Vertex* v4 = mesh->add_vertex(Vec3(0.0, 1.0, 0.0));

    Face* f = mesh->add_face({v1, v2, v3, v4});

    ASSERT_NE(f, nullptr);
    EXPECT_EQ(mesh->get_faces().size(), 1);

    auto edges = f->get_boundary_edges();
    EXPECT_EQ(edges.size(), 4);

    auto vertices = f->get_vertices();
    EXPECT_EQ(vertices.size(), 4);
}

TEST_F(WingedEdgeTest, FaceNormal) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(0.0, 1.0, 0.0));

    Face* f = mesh->add_face({v1, v2, v3});

    // Normal should point in +Z direction for CCW winding
    EXPECT_GT(f->normal.z, 0.5);  // Should be close to (0, 0, 1)
    EXPECT_DOUBLE_EQ(f->normal.length(), 1.0);  // Should be normalized
}

TEST_F(WingedEdgeTest, FaceArea) {
    // Create a 1x1 square
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(1.0, 1.0, 0.0));
    Vertex* v4 = mesh->add_vertex(Vec3(0.0, 1.0, 0.0));

    Face* f = mesh->add_face({v1, v2, v3, v4});

    EXPECT_DOUBLE_EQ(f->area(), 1.0);
}

TEST_F(WingedEdgeTest, InvalidFace) {
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));

    // Face with less than 3 vertices
    EXPECT_THROW(mesh->add_face({v1, v2}), std::invalid_argument);
}

// Mesh Primitive Tests
TEST_F(WingedEdgeTest, CreateCube) {
    auto cube = Mesh::create_cube(2.0);

    EXPECT_EQ(cube->get_vertices().size(), 8);
    EXPECT_EQ(cube->get_faces().size(), 6);

    // A cube should have 12 edges
    EXPECT_EQ(cube->get_edges().size(), 12);
}

TEST_F(WingedEdgeTest, CreatePlane) {
    auto plane = Mesh::create_plane(2.0, 3.0);

    EXPECT_EQ(plane->get_vertices().size(), 4);
    EXPECT_EQ(plane->get_edges().size(), 4);
    EXPECT_EQ(plane->get_faces().size(), 1);
}

// Topology Tests
TEST_F(WingedEdgeTest, CubeIsManifold) {
    auto cube = Mesh::create_cube(1.0);

    EXPECT_TRUE(cube->is_manifold());
}

TEST_F(WingedEdgeTest, CubeValidation) {
    auto cube = Mesh::create_cube(1.0);

    EXPECT_NO_THROW(cube->validate());
}

TEST_F(WingedEdgeTest, VertexIncidentEdges) {
    // Create a simple pyramid
    Vertex* apex = mesh->add_vertex(Vec3(0.0, 0.0, 1.0));
    Vertex* v1 = mesh->add_vertex(Vec3(-1.0, -1.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, -1.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(1.0, 1.0, 0.0));
    Vertex* v4 = mesh->add_vertex(Vec3(-1.0, 1.0, 0.0));

    // Base
    mesh->add_face({v1, v2, v3, v4});

    // Sides
    mesh->add_face({v1, v2, apex});
    mesh->add_face({v2, v3, apex});
    mesh->add_face({v3, v4, apex});
    mesh->add_face({v4, v1, apex});

    // Apex should have 4 incident edges
    auto incident_edges = apex->get_incident_edges();
    EXPECT_GE(incident_edges.size(), 4);
}

// Edge Sharing Test
TEST_F(WingedEdgeTest, SharedEdge) {
    // Create two adjacent triangles sharing an edge
    Vertex* v1 = mesh->add_vertex(Vec3(0.0, 0.0, 0.0));
    Vertex* v2 = mesh->add_vertex(Vec3(1.0, 0.0, 0.0));
    Vertex* v3 = mesh->add_vertex(Vec3(0.5, 1.0, 0.0));
    Vertex* v4 = mesh->add_vertex(Vec3(0.5, -1.0, 0.0));

    Face* f1 = mesh->add_face({v1, v2, v3});
    Face* f2 = mesh->add_face({v1, v4, v2});

    // Should only have 5 edges, not 6 (one is shared)
    EXPECT_EQ(mesh->get_edges().size(), 5);

    // Find the shared edge
    Edge* shared_edge = nullptr;
    for (const auto& e : mesh->get_edges()) {
        if ((e->v1 == v1 && e->v2 == v2) || (e->v1 == v2 && e->v2 == v1)) {
            shared_edge = e.get();
            break;
        }
    }

    ASSERT_NE(shared_edge, nullptr);
    EXPECT_NE(shared_edge->left_face, nullptr);
    EXPECT_NE(shared_edge->right_face, nullptr);
}
