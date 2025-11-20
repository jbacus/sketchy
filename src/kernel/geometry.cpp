#include "geometry.h"

namespace SketchyKernel {

Mat4 Mat4::translation(double x, double y, double z) {
    Mat4 result;
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    return result;
}

Mat4 Mat4::rotation(const Vec3& axis, double angle) {
    Mat4 result;
    Vec3 a = axis.normalized();
    double c = std::cos(angle);
    double s = std::sin(angle);
    double t = 1.0 - c;

    result.m[0][0] = t * a.x * a.x + c;
    result.m[0][1] = t * a.x * a.y - s * a.z;
    result.m[0][2] = t * a.x * a.z + s * a.y;

    result.m[1][0] = t * a.x * a.y + s * a.z;
    result.m[1][1] = t * a.y * a.y + c;
    result.m[1][2] = t * a.y * a.z - s * a.x;

    result.m[2][0] = t * a.x * a.z - s * a.y;
    result.m[2][1] = t * a.y * a.z + s * a.x;
    result.m[2][2] = t * a.z * a.z + c;

    return result;
}

Mat4 Mat4::scale(double sx, double sy, double sz) {
    Mat4 result;
    result.m[0][0] = sx;
    result.m[1][1] = sy;
    result.m[2][2] = sz;
    return result;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

Vec3 Mat4::transform(const Vec3& v) const {
    double x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
    double y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
    double z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
    double w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];

    if (w != 1.0 && w != 0.0) {
        return Vec3(x / w, y / w, z / w);
    }
    return Vec3(x, y, z);
}

} // namespace SketchyKernel
