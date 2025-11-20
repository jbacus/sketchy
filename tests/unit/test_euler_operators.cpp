#include <gtest/gtest.h>
#include "kernel/winged_edge.h"

using namespace SketchyKernel;

// Test fixture for Euler Operator tests
class EulerOperatorTest : public ::testing::Test {
protected:
    WingedEdgeKernel kernel;

    void SetUp() override {
        kernel = WingedEdgeKernel();
    }
};

// ==================== MVSF Tests ====================

TEST_F(EulerOperatorTest, MVSF_CreatesVertexAndFace) {
    Point3D p(1.0, 2.0, 3.0);
    auto v = kernel.mvsf(p);

    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->coords.x, 1.0);
    EXPECT_EQ(v->coords.y, 2.0);
    EXPECT_EQ(v->coords.z, 3.0);
    EXPECT_EQ(v->id, 1);

    // Should have created 1 vertex and 1 face
    EXPECT_EQ(kernel.getVertexCount(), 1);
    EXPECT_EQ(kernel.getFaceCount(), 1);
    EXPECT_EQ(kernel.getEdgeCount(), 0); // No edges yet
}

TEST_F(EulerOperatorTest, MVSF_MultipleCalls) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto v2 = kernel.mvsf(Point3D(1, 1, 1));

    EXPECT_EQ(kernel.getVertexCount(), 2);
    EXPECT_EQ(kernel.getFaceCount(), 2);
    EXPECT_NE(v1->id, v2->id); // Different IDs
}

// ==================== MEV Tests ====================

TEST_F(EulerOperatorTest, MEV_CreatesEdgeAndVertex) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto edge = kernel.mev(v1, Point3D(1, 0, 0), face);

    ASSERT_NE(edge, nullptr);
    EXPECT_EQ(kernel.getVertexCount(), 2);
    EXPECT_EQ(kernel.getEdgeCount(), 1);
    EXPECT_EQ(kernel.getFaceCount(), 1); // Same face count

    // Check edge connectivity
    EXPECT_EQ(edge->v1, v1);
    EXPECT_NE(edge->v2, nullptr);
    EXPECT_EQ(edge->v2->coords.x, 1.0);
    EXPECT_EQ(edge->v2->coords.y, 0.0);
    EXPECT_EQ(edge->v2->coords.z, 0.0);

    // Both sides of edge should reference the same face initially
    EXPECT_EQ(edge->f1, face);
    EXPECT_EQ(edge->f2, face);
}

TEST_F(EulerOperatorTest, MEV_BuildsLineString) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;
    auto e2 = kernel.mev(v2, Point3D(2, 0, 0), face);
    auto v3 = e2->v2;

    EXPECT_EQ(kernel.getVertexCount(), 3);
    EXPECT_EQ(kernel.getEdgeCount(), 2);

    // Check connectivity
    EXPECT_EQ(e1->v2, e2->v1);
}

TEST_F(EulerOperatorTest, MEV_ThrowsOnNullVertex) {
    auto face = kernel.mvsf(Point3D(0, 0, 0));
    auto f = kernel.getFaces()[0];

    EXPECT_THROW(kernel.mev(nullptr, Point3D(1, 0, 0), f), std::invalid_argument);
}

TEST_F(EulerOperatorTest, MEV_ThrowsOnNullFace) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));

    EXPECT_THROW(kernel.mev(v, Point3D(1, 0, 0), nullptr), std::invalid_argument);
}

// ==================== MEF Tests ====================

TEST_F(EulerOperatorTest, MEF_SplitsFace) {
    // Create a square: v1 -> v2 -> v3 -> v4 -> back to v1
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
    auto v3 = e2->v2;

    auto e3 = kernel.mev(v3, Point3D(0, 1, 0), face);
    auto v4 = e3->v2;

    // Now close the loop with MEF, connecting v4 back to v1
    // This should split the face
    auto closing_edge = kernel.mef(v4, v1, face);

    EXPECT_NE(closing_edge, nullptr);
    EXPECT_EQ(kernel.getFaceCount(), 2); // Original face + new face
    EXPECT_EQ(closing_edge->v1, v4);
    EXPECT_EQ(closing_edge->v2, v1);

    // The edge should separate two different faces
    EXPECT_NE(closing_edge->f1, nullptr);
    EXPECT_NE(closing_edge->f2, nullptr);
    EXPECT_NE(closing_edge->f1, closing_edge->f2);
}

TEST_F(EulerOperatorTest, MEF_ThrowsOnSameVertex) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    EXPECT_THROW(kernel.mef(v, v, face), std::invalid_argument);
}

TEST_F(EulerOperatorTest, MEF_ThrowsOnNullInputs) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    auto e = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e->v2;

    EXPECT_THROW(kernel.mef(nullptr, v2, face), std::invalid_argument);
    EXPECT_THROW(kernel.mef(v1, nullptr, face), std::invalid_argument);
    EXPECT_THROW(kernel.mef(v1, v2, nullptr), std::invalid_argument);
}

// ==================== KEF Tests ====================

TEST_F(EulerOperatorTest, KEF_RemovesEdgeAndMergesFaces) {
    // Create a simple configuration with 2 faces separated by an edge
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
    auto v3 = e2->v2;

    // Create edge that splits face
    auto split_edge = kernel.mef(v3, v1, face);

    ASSERT_EQ(kernel.getFaceCount(), 2);
    size_t initial_edges = kernel.getEdgeCount();

    // Now use KEF to merge faces back
    auto merged_face = kernel.kef(split_edge);

    EXPECT_NE(merged_face, nullptr);
    EXPECT_EQ(kernel.getFaceCount(), 1);
    EXPECT_EQ(kernel.getEdgeCount(), initial_edges - 1);
}

TEST_F(EulerOperatorTest, KEF_ThrowsOnNullEdge) {
    EXPECT_THROW(kernel.kef(nullptr), std::invalid_argument);
}

// ==================== Navigation Tests ====================

TEST_F(EulerOperatorTest, GetIncidentEdges_SingleVertex) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v, Point3D(0, 1, 0), face);

    auto incident = kernel.getIncidentEdges(v);
    EXPECT_GE(incident.size(), 2);
}

TEST_F(EulerOperatorTest, GetFaceBoundary_Square) {
    // Build a square face
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
    auto v3 = e2->v2;

    auto e3 = kernel.mev(v3, Point3D(0, 1, 0), face);
    auto v4 = e3->v2;

    auto e4 = kernel.mef(v4, v1, face);

    // Get one of the resulting faces and check its boundary
    auto faces = kernel.getFaces();
    for (const auto& f : faces) {
        auto boundary = kernel.getFaceBoundary(f);
        // Should have edges forming a closed loop
        EXPECT_GE(boundary.size(), 1);
    }
}

TEST_F(EulerOperatorTest, GetFaceVertices) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    kernel.mev(v1, Point3D(1, 0, 0), face);

    auto verts = kernel.getFaceVertices(face);
    EXPECT_GE(verts.size(), 0);
}

// ==================== Validation Tests ====================

TEST_F(EulerOperatorTest, Validate_EmptyKernel) {
    WingedEdgeKernel empty;
    EXPECT_TRUE(empty.validate());
}

TEST_F(EulerOperatorTest, Validate_AfterMVSF) {
    kernel.mvsf(Point3D(0, 0, 0));
    EXPECT_TRUE(kernel.validate());
}

TEST_F(EulerOperatorTest, Validate_AfterMEV) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    kernel.mev(v, Point3D(1, 0, 0), face);

    EXPECT_TRUE(kernel.validate());
}

TEST_F(EulerOperatorTest, Validate_AfterComplexOperations) {
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
    auto v3 = e2->v2;

    kernel.mef(v3, v1, face);

    EXPECT_TRUE(kernel.validate());
}

// ==================== Manifold Tests ====================

TEST_F(EulerOperatorTest, IsManifold_SimpleConfiguration) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));
    EXPECT_TRUE(kernel.isManifold());

    auto face = kernel.getFaces()[0];
    kernel.mev(v, Point3D(1, 0, 0), face);

    EXPECT_TRUE(kernel.isManifold());
}

// ==================== ID Lookup Tests ====================

TEST_F(EulerOperatorTest, GetVertexById) {
    auto v = kernel.mvsf(Point3D(1, 2, 3));
    int id = v->id;

    auto found = kernel.getVertexById(id);
    EXPECT_EQ(found, v);
    EXPECT_EQ(found->coords.x, 1.0);

    auto not_found = kernel.getVertexById(9999);
    EXPECT_EQ(not_found, nullptr);
}

TEST_F(EulerOperatorTest, GetEdgeById) {
    auto v = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    auto e = kernel.mev(v, Point3D(1, 0, 0), face);
    int id = e->id;

    auto found = kernel.getEdgeById(id);
    EXPECT_EQ(found, e);

    auto not_found = kernel.getEdgeById(9999);
    EXPECT_EQ(not_found, nullptr);
}

TEST_F(EulerOperatorTest, GetFaceById) {
    kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];
    int id = face->id;

    auto found = kernel.getFaceById(id);
    EXPECT_EQ(found, face);

    auto not_found = kernel.getFaceById(9999);
    EXPECT_EQ(not_found, nullptr);
}

// ==================== Edge Case Tests ====================

TEST_F(EulerOperatorTest, BuildTriangle) {
    // Build a complete triangle using Euler operators
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    // Add second vertex
    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    // Add third vertex
    auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
    auto v3 = e2->v2;

    // Close the triangle
    auto e3 = kernel.mef(v3, v1, face);

    EXPECT_EQ(kernel.getVertexCount(), 3);
    EXPECT_EQ(kernel.getEdgeCount(), 3);
    EXPECT_EQ(kernel.getFaceCount(), 2); // Original + interior face
    EXPECT_TRUE(kernel.validate());
}

TEST_F(EulerOperatorTest, BuildQuad) {
    // Build a quadrilateral
    auto v1 = kernel.mvsf(Point3D(0, 0, 0));
    auto face = kernel.getFaces()[0];

    auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
    auto v2 = e1->v2;

    auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
    auto v3 = e2->v2;

    auto e3 = kernel.mev(v3, Point3D(0, 1, 0), face);
    auto v4 = e3->v2;

    auto e4 = kernel.mef(v4, v1, face);

    EXPECT_EQ(kernel.getVertexCount(), 4);
    EXPECT_EQ(kernel.getEdgeCount(), 4);
    EXPECT_TRUE(kernel.validate());
}
