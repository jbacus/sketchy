#include <gtest/gtest.h>
#include "sketchy/kernel/geometry.h"
#include <cmath>

using namespace sketchy::kernel;

// Test fixture for geometry tests
class GeometryTest : public ::testing::Test {
protected:
    const double EPSILON = 1e-6;

    bool approx_equal(double a, double b) {
        return std::abs(a - b) < EPSILON;
    }

    bool vec_approx_equal(const Vec3& a, const Vec3& b) {
        return approx_equal(a.x, b.x) &&
               approx_equal(a.y, b.y) &&
               approx_equal(a.z, b.z);
    }
};

// Vec3 Tests
TEST_F(GeometryTest, Vec3Construction) {
    Vec3 v1;
    EXPECT_EQ(v1.x, 0.0);
    EXPECT_EQ(v1.y, 0.0);
    EXPECT_EQ(v1.z, 0.0);

    Vec3 v2(1.0, 2.0, 3.0);
    EXPECT_EQ(v2.x, 1.0);
    EXPECT_EQ(v2.y, 2.0);
    EXPECT_EQ(v2.z, 3.0);
}

TEST_F(GeometryTest, Vec3Addition) {
    Vec3 v1(1.0, 2.0, 3.0);
    Vec3 v2(4.0, 5.0, 6.0);
    Vec3 result = v1 + v2;

    EXPECT_EQ(result.x, 5.0);
    EXPECT_EQ(result.y, 7.0);
    EXPECT_EQ(result.z, 9.0);
}

TEST_F(GeometryTest, Vec3Subtraction) {
    Vec3 v1(4.0, 5.0, 6.0);
    Vec3 v2(1.0, 2.0, 3.0);
    Vec3 result = v1 - v2;

    EXPECT_EQ(result.x, 3.0);
    EXPECT_EQ(result.y, 3.0);
    EXPECT_EQ(result.z, 3.0);
}

TEST_F(GeometryTest, Vec3ScalarMultiplication) {
    Vec3 v(1.0, 2.0, 3.0);
    Vec3 result = v * 2.0;

    EXPECT_EQ(result.x, 2.0);
    EXPECT_EQ(result.y, 4.0);
    EXPECT_EQ(result.z, 6.0);
}

TEST_F(GeometryTest, Vec3DotProduct) {
    Vec3 v1(1.0, 0.0, 0.0);
    Vec3 v2(0.0, 1.0, 0.0);

    EXPECT_EQ(v1.dot(v2), 0.0);

    Vec3 v3(1.0, 2.0, 3.0);
    Vec3 v4(2.0, 3.0, 4.0);
    EXPECT_EQ(v3.dot(v4), 20.0);  // 1*2 + 2*3 + 3*4 = 20
}

TEST_F(GeometryTest, Vec3CrossProduct) {
    Vec3 v1(1.0, 0.0, 0.0);
    Vec3 v2(0.0, 1.0, 0.0);
    Vec3 result = v1.cross(v2);

    EXPECT_TRUE(vec_approx_equal(result, Vec3(0.0, 0.0, 1.0)));
}

TEST_F(GeometryTest, Vec3Length) {
    Vec3 v(3.0, 4.0, 0.0);
    EXPECT_DOUBLE_EQ(v.length(), 5.0);

    Vec3 v2(1.0, 1.0, 1.0);
    EXPECT_TRUE(approx_equal(v2.length(), std::sqrt(3.0)));
}

TEST_F(GeometryTest, Vec3Normalize) {
    Vec3 v(3.0, 4.0, 0.0);
    Vec3 normalized = v.normalized();

    EXPECT_DOUBLE_EQ(normalized.length(), 1.0);
    EXPECT_TRUE(vec_approx_equal(normalized, Vec3(0.6, 0.8, 0.0)));
}

// Mat4 Tests
TEST_F(GeometryTest, Mat4Identity) {
    Mat4 identity;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                EXPECT_EQ(identity.m[i][j], 1.0);
            } else {
                EXPECT_EQ(identity.m[i][j], 0.0);
            }
        }
    }
}

TEST_F(GeometryTest, Mat4Translation) {
    Mat4 trans = Mat4::translation(1.0, 2.0, 3.0);
    Vec3 point(0.0, 0.0, 0.0);
    Vec3 transformed = trans.transform(point);

    EXPECT_TRUE(vec_approx_equal(transformed, Vec3(1.0, 2.0, 3.0)));
}

TEST_F(GeometryTest, Mat4Scale) {
    Mat4 scale = Mat4::scale(2.0, 3.0, 4.0);
    Vec3 point(1.0, 1.0, 1.0);
    Vec3 transformed = scale.transform(point);

    EXPECT_TRUE(vec_approx_equal(transformed, Vec3(2.0, 3.0, 4.0)));
}

TEST_F(GeometryTest, Mat4Rotation) {
    // Rotate 90 degrees around Z axis
    Mat4 rot = Mat4::rotation(Vec3(0, 0, 1), M_PI / 2.0);
    Vec3 point(1.0, 0.0, 0.0);
    Vec3 transformed = rot.transform(point);

    EXPECT_TRUE(vec_approx_equal(transformed, Vec3(0.0, 1.0, 0.0)));
}

TEST_F(GeometryTest, Mat4Multiplication) {
    Mat4 trans = Mat4::translation(1.0, 0.0, 0.0);
    Mat4 scale = Mat4::scale(2.0, 2.0, 2.0);
    Mat4 combined = trans * scale;

    Vec3 point(1.0, 1.0, 1.0);
    Vec3 result = combined.transform(point);

    // First scale, then translate
    EXPECT_TRUE(vec_approx_equal(result, Vec3(3.0, 2.0, 2.0)));
}
