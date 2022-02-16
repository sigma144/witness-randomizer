#include "Quaternion.h"
#define _USE_MATH_DEFINES // For M_PI_4
#include <cmath>

double Quaternion::Length() const {
    return std::sqrt(w * w + x * x + y * y + z * z);
}

Quaternion Quaternion::Normalize() {
    float length = (float)Length();
    w /= length;
    x /= length;
    y /= length;
    z /= length;
    return *this;
}

Quaternion Quaternion::Mul(const Quaternion& other) const {
    return Quaternion{
        w * other.w - x * other.x - y * other.y - z * other.z,
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w
    };
}

Quaternion Quaternion::Rotate90() {
    float angle = (float)M_PI_4;
    auto q = Quaternion{std::cos(angle), std::sin(angle), 0, 0};
    return this->Mul(q).Normalize();
}
