#pragma once
#include <vector>

struct Quaternion {
	float x;
	float y;
    float z;
	float w;
    Quaternion(float yaw, float pitch, float roll);
    Quaternion(float x, float y, float z, float w);
    Quaternion() { Quaternion(0, 0, 0); }

    // Basic operations
    double Length() const;
    Quaternion Normalize();

    // Complex operations
    Quaternion Mul(const Quaternion& other) const;
    void RotateVector(std::vector<float>& vector);
};
