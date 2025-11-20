#ifndef SKETCHY_KERNEL_GEOMETRY_H
#define SKETCHY_KERNEL_GEOMETRY_H

#include <array>
#include <cmath>

namespace sketchy {
namespace kernel {

/**
 * 3D Vector representation
 */
class Vec3 {
public:
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    // Vector operations
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(double scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    double dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized() const {
        double len = length();
        if (len > 0) {
            return *this / len;
        }
        return Vec3(0, 0, 0);
    }
};

/**
 * 4x4 Transformation Matrix
 */
class Mat4 {
public:
    std::array<std::array<double, 4>, 4> m;

    Mat4() {
        // Identity matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0 : 0.0;
            }
        }
    }

    static Mat4 translation(double x, double y, double z);
    static Mat4 rotation(const Vec3& axis, double angle);
    static Mat4 scale(double sx, double sy, double sz);

    Mat4 operator*(const Mat4& other) const;
    Vec3 transform(const Vec3& v) const;
};

} // namespace kernel
} // namespace sketchy

#endif // SKETCHY_KERNEL_GEOMETRY_H
